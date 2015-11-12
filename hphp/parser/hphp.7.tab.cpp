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
#line 873 "hphp.7.tab.cpp"

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
#define YYLAST   16788

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  197
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  261
/* YYNRULES -- Number of rules.  */
#define YYNRULES  972
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1775

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
    2661,  2665,  2667,  2669,  2671,  2673,  2675,  2679,  2683,  2688,
    2693,  2697,  2699,  2701,  2709,  2719,  2727,  2734,  2743,  2745,
    2750,  2755,  2757,  2759,  2764,  2767,  2769,  2770,  2772,  2774,
    2776,  2780,  2784,  2788,  2789,  2791,  2793,  2797,  2801,  2804,
    2808,  2815,  2816,  2818,  2823,  2826,  2827,  2833,  2837,  2841,
    2843,  2850,  2855,  2860,  2863,  2866,  2867,  2873,  2877,  2881,
    2883,  2886,  2887,  2893,  2897,  2901,  2903,  2906,  2909,  2911,
    2914,  2916,  2921,  2925,  2929,  2936,  2940,  2942,  2944,  2946,
    2951,  2956,  2961,  2966,  2971,  2976,  2979,  2982,  2987,  2990,
    2993,  2995,  2999,  3003,  3007,  3008,  3011,  3017,  3024,  3031,
    3039,  3041,  3044,  3046,  3049,  3051,  3056,  3058,  3063,  3067,
    3068,  3070,  3074,  3077,  3081,  3083,  3085,  3086,  3087,  3090,
    3093,  3096,  3101,  3104,  3110,  3114,  3116,  3118,  3119,  3123,
    3128,  3134,  3138,  3140,  3143,  3144,  3149,  3151,  3155,  3158,
    3161,  3164,  3166,  3168,  3170,  3172,  3176,  3181,  3188,  3190,
    3199,  3206,  3208
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     198,     0,    -1,    -1,   199,   200,    -1,   200,   201,    -1,
      -1,   216,    -1,   233,    -1,   240,    -1,   237,    -1,   247,
      -1,   437,    -1,   126,   187,   188,   189,    -1,   153,   209,
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
      -1,   210,   442,    -1,   210,   442,    -1,   213,     9,   438,
      14,   377,    -1,   109,   438,    14,   377,    -1,   214,   215,
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
     189,    -1,   123,   187,   434,   188,   189,    -1,   189,    -1,
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
      -1,   108,    -1,    -1,   232,   231,   441,   234,   187,   275,
     188,   446,   309,    -1,    -1,   313,   232,   231,   441,   235,
     187,   275,   188,   446,   309,    -1,    -1,   398,   312,   232,
     231,   441,   236,   187,   275,   188,   446,   309,    -1,    -1,
     163,   204,   238,    30,   456,   436,   190,   282,   191,    -1,
      -1,   398,   163,   204,   239,    30,   456,   436,   190,   282,
     191,    -1,    -1,   253,   250,   241,   254,   255,   190,   285,
     191,    -1,    -1,   398,   253,   250,   242,   254,   255,   190,
     285,   191,    -1,    -1,   128,   251,   243,   256,   190,   285,
     191,    -1,    -1,   398,   128,   251,   244,   256,   190,   285,
     191,    -1,    -1,   127,   246,   375,   254,   255,   190,   285,
     191,    -1,    -1,   165,   252,   248,   255,   190,   285,   191,
      -1,    -1,   398,   165,   252,   249,   255,   190,   285,   191,
      -1,   441,    -1,   157,    -1,   441,    -1,   441,    -1,   127,
      -1,   120,   127,    -1,   120,   119,   127,    -1,   119,   120,
     127,    -1,   119,   127,    -1,   129,   368,    -1,    -1,   130,
     257,    -1,    -1,   129,   257,    -1,    -1,   368,    -1,   257,
       9,   368,    -1,   368,    -1,   258,     9,   368,    -1,   133,
     260,    -1,    -1,   410,    -1,    36,   410,    -1,   134,   187,
     423,   188,    -1,   216,    -1,    30,   214,    94,   189,    -1,
     216,    -1,    30,   214,    96,   189,    -1,   216,    -1,    30,
     214,    92,   189,    -1,   216,    -1,    30,   214,    98,   189,
      -1,   204,    14,   377,    -1,   265,     9,   204,    14,   377,
      -1,   190,   267,   191,    -1,   190,   189,   267,   191,    -1,
      30,   267,   102,   189,    -1,    30,   189,   267,   102,   189,
      -1,   267,   103,   332,   268,   214,    -1,   267,   104,   268,
     214,    -1,    -1,    30,    -1,   189,    -1,   269,    73,   323,
     216,    -1,    -1,   270,    73,   323,    30,   214,    -1,    -1,
      74,   216,    -1,    -1,    74,    30,   214,    -1,    -1,   274,
       9,   399,   315,   457,   166,    81,    -1,   274,     9,   399,
     315,   457,    36,   166,    81,    -1,   274,     9,   399,   315,
     457,   166,    -1,   274,   382,    -1,   399,   315,   457,   166,
      81,    -1,   399,   315,   457,    36,   166,    81,    -1,   399,
     315,   457,   166,    -1,    -1,   399,   315,   457,    81,    -1,
     399,   315,   457,    36,    81,    -1,   399,   315,   457,    36,
      81,    14,   332,    -1,   399,   315,   457,    81,    14,   332,
      -1,   274,     9,   399,   315,   457,    81,    -1,   274,     9,
     399,   315,   457,    36,    81,    -1,   274,     9,   399,   315,
     457,    36,    81,    14,   332,    -1,   274,     9,   399,   315,
     457,    81,    14,   332,    -1,   276,     9,   399,   457,   166,
      81,    -1,   276,     9,   399,   457,    36,   166,    81,    -1,
     276,     9,   399,   457,   166,    -1,   276,   382,    -1,   399,
     457,   166,    81,    -1,   399,   457,    36,   166,    81,    -1,
     399,   457,   166,    -1,    -1,   399,   457,    81,    -1,   399,
     457,    36,    81,    -1,   399,   457,    36,    81,    14,   332,
      -1,   399,   457,    81,    14,   332,    -1,   276,     9,   399,
     457,    81,    -1,   276,     9,   399,   457,    36,    81,    -1,
     276,     9,   399,   457,    36,    81,    14,   332,    -1,   276,
       9,   399,   457,    81,    14,   332,    -1,   278,   382,    -1,
      -1,   332,    -1,    36,   410,    -1,   166,   332,    -1,   278,
       9,   332,    -1,   278,     9,   166,   332,    -1,   278,     9,
      36,   410,    -1,   279,     9,   280,    -1,   280,    -1,    81,
      -1,   192,   410,    -1,   192,   190,   332,   191,    -1,   281,
       9,    81,    -1,   281,     9,    81,    14,   377,    -1,    81,
      -1,    81,    14,   377,    -1,   282,   283,    -1,    -1,   284,
     189,    -1,   439,    14,   377,    -1,   285,   286,    -1,    -1,
      -1,   311,   287,   317,   189,    -1,    -1,   313,   456,   288,
     317,   189,    -1,   318,   189,    -1,   319,   189,    -1,   320,
     189,    -1,    -1,   312,   232,   231,   440,   187,   289,   273,
     188,   446,   310,    -1,    -1,   398,   312,   232,   231,   441,
     187,   290,   273,   188,   446,   310,    -1,   159,   295,   189,
      -1,   160,   303,   189,    -1,   162,   305,   189,    -1,     4,
     129,   368,   189,    -1,     4,   130,   368,   189,    -1,   114,
     258,   189,    -1,   114,   258,   190,   291,   191,    -1,   291,
     292,    -1,   291,   293,    -1,    -1,   212,   152,   204,   167,
     258,   189,    -1,   294,    99,   312,   204,   189,    -1,   294,
      99,   313,   189,    -1,   212,   152,   204,    -1,   204,    -1,
     296,    -1,   295,     9,   296,    -1,   297,   365,   301,   302,
      -1,   157,    -1,    29,   298,    -1,   298,    -1,   135,    -1,
     135,   173,   456,   174,    -1,   135,   173,   456,     9,   456,
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
      14,   377,    -1,   318,     9,   439,    14,   377,    -1,   109,
     439,    14,   377,    -1,   319,     9,   439,    -1,   120,   109,
     439,    -1,   120,   321,   436,    -1,   321,   436,    14,   456,
      -1,   109,   178,   441,    -1,   187,   322,   188,    -1,    70,
     372,   375,    -1,    70,   245,    -1,    69,   332,    -1,   357,
      -1,   352,    -1,   187,   332,   188,    -1,   324,     9,   332,
      -1,   332,    -1,   324,    -1,    -1,    27,    -1,    27,   332,
      -1,    27,   332,   133,   332,    -1,   187,   326,   188,    -1,
     410,    14,   326,    -1,   134,   187,   423,   188,    14,   326,
      -1,    28,   332,    -1,   410,    14,   329,    -1,   134,   187,
     423,   188,    14,   329,    -1,   333,    -1,   410,    -1,   322,
      -1,   414,    -1,   413,    -1,   134,   187,   423,   188,    14,
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
      30,   332,    -1,   332,    31,   332,    -1,   433,    -1,    64,
     332,    -1,    63,   332,    -1,    62,   332,    -1,    61,   332,
      -1,    60,   332,    -1,    59,   332,    -1,    58,   332,    -1,
      71,   373,    -1,    57,   332,    -1,   379,    -1,   351,    -1,
     350,    -1,   193,   374,   193,    -1,    13,   332,    -1,   354,
      -1,   114,   187,   356,   382,   188,    -1,    -1,    -1,   232,
     231,   187,   336,   275,   188,   446,   334,   190,   214,   191,
      -1,    -1,   313,   232,   231,   187,   337,   275,   188,   446,
     334,   190,   214,   191,    -1,    -1,   183,    81,   339,   344,
      -1,    -1,   183,   184,   340,   275,   185,   446,   344,    -1,
      -1,   183,   190,   341,   214,   191,    -1,    -1,    81,   342,
     344,    -1,    -1,   184,   343,   275,   185,   446,   344,    -1,
       8,   332,    -1,     8,   329,    -1,     8,   190,   214,   191,
      -1,    88,    -1,   435,    -1,   346,     9,   345,   133,   332,
      -1,   345,   133,   332,    -1,   347,     9,   345,   133,   377,
      -1,   345,   133,   377,    -1,   346,   381,    -1,    -1,   347,
     381,    -1,    -1,   177,   187,   348,   188,    -1,   135,   187,
     424,   188,    -1,    68,   424,   194,    -1,   368,   190,   426,
     191,    -1,   368,   190,   428,   191,    -1,   354,    68,   420,
     194,    -1,   355,    68,   420,   194,    -1,   351,    -1,   435,
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
     370,   152,   419,    -1,   369,    -1,   416,    -1,   371,   152,
     419,    -1,   368,    -1,   121,    -1,   421,    -1,   187,   188,
      -1,   323,    -1,    -1,    -1,    87,    -1,   430,    -1,   187,
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
     127,    -1,   210,    -1,    80,    -1,   435,    -1,   376,    -1,
     195,   430,   195,    -1,   196,   430,   196,    -1,   148,   430,
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
     191,    -1,   401,    -1,   419,    -1,   204,    -1,   190,   332,
     191,    -1,   403,    -1,   419,    -1,    68,   420,   194,    -1,
     190,   332,   191,    -1,   411,   405,    -1,   187,   322,   188,
     405,    -1,   422,   405,    -1,   187,   322,   188,   405,    -1,
     187,   322,   188,   400,   402,    -1,   187,   333,   188,   400,
     402,    -1,   187,   322,   188,   400,   401,    -1,   187,   333,
     188,   400,   401,    -1,   417,    -1,   367,    -1,   415,    -1,
     416,    -1,   406,    -1,   408,    -1,   410,   400,   402,    -1,
     371,   152,   419,    -1,   412,   187,   277,   188,    -1,   413,
     187,   277,   188,    -1,   187,   410,   188,    -1,   367,    -1,
     415,    -1,   416,    -1,   406,    -1,   410,   400,   402,    -1,
     409,    -1,   412,   187,   277,   188,    -1,   187,   410,   188,
      -1,   371,   152,   419,    -1,   417,    -1,   406,    -1,   367,
      -1,   351,    -1,   376,    -1,   187,   410,   188,    -1,   187,
     333,   188,    -1,   413,   187,   277,   188,    -1,   412,   187,
     277,   188,    -1,   187,   414,   188,    -1,   335,    -1,   338,
      -1,   410,   400,   404,   442,   187,   277,   188,    -1,   187,
     322,   188,   400,   404,   442,   187,   277,   188,    -1,   371,
     152,   206,   442,   187,   277,   188,    -1,   371,   152,   419,
     187,   277,   188,    -1,   371,   152,   190,   332,   191,   187,
     277,   188,    -1,   418,    -1,   418,    68,   420,   194,    -1,
     418,   190,   332,   191,    -1,   419,    -1,    81,    -1,   192,
     190,   332,   191,    -1,   192,   419,    -1,   332,    -1,    -1,
     417,    -1,   407,    -1,   408,    -1,   421,   400,   402,    -1,
     370,   152,   417,    -1,   187,   410,   188,    -1,    -1,   407,
      -1,   409,    -1,   421,   400,   401,    -1,   187,   410,   188,
      -1,   423,     9,    -1,   423,     9,   410,    -1,   423,     9,
     134,   187,   423,   188,    -1,    -1,   410,    -1,   134,   187,
     423,   188,    -1,   425,   381,    -1,    -1,   425,     9,   332,
     133,   332,    -1,   425,     9,   332,    -1,   332,   133,   332,
      -1,   332,    -1,   425,     9,   332,   133,    36,   410,    -1,
     425,     9,    36,   410,    -1,   332,   133,    36,   410,    -1,
      36,   410,    -1,   427,   381,    -1,    -1,   427,     9,   332,
     133,   332,    -1,   427,     9,   332,    -1,   332,   133,   332,
      -1,   332,    -1,   429,   381,    -1,    -1,   429,     9,   377,
     133,   377,    -1,   429,     9,   377,    -1,   377,   133,   377,
      -1,   377,    -1,   430,   431,    -1,   430,    87,    -1,   431,
      -1,    87,   431,    -1,    81,    -1,    81,    68,   432,   194,
      -1,    81,   400,   204,    -1,   150,   332,   191,    -1,   150,
      80,    68,   332,   194,   191,    -1,   151,   410,   191,    -1,
     204,    -1,    82,    -1,    81,    -1,   124,   187,   324,   188,
      -1,   125,   187,   410,   188,    -1,   125,   187,   333,   188,
      -1,   125,   187,   414,   188,    -1,   125,   187,   413,   188,
      -1,   125,   187,   322,   188,    -1,     7,   332,    -1,     6,
     332,    -1,     5,   187,   332,   188,    -1,     4,   332,    -1,
       3,   332,    -1,   410,    -1,   434,     9,   410,    -1,   371,
     152,   205,    -1,   371,   152,   127,    -1,    -1,    99,   456,
      -1,   178,   441,    14,   456,   189,    -1,   398,   178,   441,
      14,   456,   189,    -1,   180,   441,   436,    14,   456,   189,
      -1,   398,   180,   441,   436,    14,   456,   189,    -1,   206,
      -1,   456,   206,    -1,   205,    -1,   456,   205,    -1,   206,
      -1,   206,   173,   448,   174,    -1,   204,    -1,   204,   173,
     448,   174,    -1,   173,   444,   174,    -1,    -1,   456,    -1,
     443,     9,   456,    -1,   443,   381,    -1,   443,     9,   166,
      -1,   444,    -1,   166,    -1,    -1,    -1,    30,   456,    -1,
      99,   456,    -1,   100,   456,    -1,   448,     9,   449,   204,
      -1,   449,   204,    -1,   448,     9,   449,   204,   447,    -1,
     449,   204,   447,    -1,    48,    -1,    49,    -1,    -1,    88,
     133,   456,    -1,    29,    88,   133,   456,    -1,   212,   152,
     204,   133,   456,    -1,   451,     9,   450,    -1,   450,    -1,
     451,   381,    -1,    -1,   177,   187,   452,   188,    -1,   212,
      -1,   204,   152,   455,    -1,   204,   442,    -1,    29,   456,
      -1,    57,   456,    -1,   212,    -1,   135,    -1,   136,    -1,
     453,    -1,   454,   152,   455,    -1,   135,   173,   456,   174,
      -1,   135,   173,   456,     9,   456,   174,    -1,   157,    -1,
     187,   108,   187,   445,   188,    30,   456,   188,    -1,   187,
     456,     9,   443,   381,   188,    -1,   456,    -1,    -1
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
    2602,  2603,  2607,  2608,  2609,  2613,  2618,  2623,  2624,  2628,
    2633,  2638,  2639,  2643,  2644,  2649,  2651,  2656,  2667,  2681,
    2693,  2708,  2709,  2710,  2711,  2712,  2713,  2714,  2724,  2733,
    2735,  2737,  2741,  2742,  2743,  2744,  2745,  2761,  2762,  2764,
    2766,  2773,  2774,  2775,  2776,  2777,  2778,  2779,  2780,  2782,
    2787,  2791,  2792,  2796,  2799,  2806,  2810,  2819,  2826,  2834,
    2836,  2837,  2841,  2842,  2844,  2849,  2850,  2861,  2862,  2863,
    2864,  2875,  2878,  2881,  2882,  2883,  2884,  2895,  2899,  2900,
    2901,  2903,  2904,  2905,  2909,  2911,  2914,  2916,  2917,  2918,
    2919,  2922,  2924,  2925,  2929,  2931,  2934,  2936,  2937,  2938,
    2942,  2944,  2947,  2950,  2952,  2954,  2958,  2959,  2961,  2962,
    2968,  2969,  2971,  2981,  2983,  2985,  2988,  2989,  2990,  2994,
    2995,  2996,  2997,  2998,  2999,  3000,  3001,  3002,  3003,  3004,
    3008,  3009,  3013,  3015,  3023,  3025,  3029,  3033,  3038,  3042,
    3050,  3051,  3055,  3056,  3062,  3063,  3072,  3073,  3081,  3084,
    3088,  3091,  3096,  3101,  3103,  3104,  3105,  3109,  3110,  3114,
    3115,  3118,  3121,  3123,  3127,  3133,  3134,  3135,  3139,  3143,
    3153,  3161,  3163,  3167,  3169,  3174,  3180,  3183,  3188,  3196,
    3199,  3202,  3203,  3206,  3209,  3210,  3215,  3218,  3222,  3226,
    3232,  3242,  3243
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
  "variable_no_calls", "dimmable_variable_no_calls", "assignment_list",
  "array_pair_list", "non_empty_array_pair_list", "collection_init",
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
     411,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     413,   414,   414,   415,   415,   416,   416,   416,   417,   418,
     418,   418,   419,   419,   419,   420,   420,   421,   421,   421,
     421,   421,   421,   422,   422,   422,   422,   422,   423,   423,
     423,   423,   423,   423,   424,   424,   425,   425,   425,   425,
     425,   425,   425,   425,   426,   426,   427,   427,   427,   427,
     428,   428,   429,   429,   429,   429,   430,   430,   430,   430,
     431,   431,   431,   431,   431,   431,   432,   432,   432,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   433,   433,
     434,   434,   435,   435,   436,   436,   437,   437,   437,   437,
     438,   438,   439,   439,   440,   440,   441,   441,   442,   442,
     443,   443,   444,   445,   445,   445,   445,   446,   446,   447,
     447,   448,   448,   448,   448,   449,   449,   449,   450,   450,
     450,   451,   451,   452,   452,   453,   454,   455,   455,   456,
     456,   456,   456,   456,   456,   456,   456,   456,   456,   456,
     456,   457,   457
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
       3,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     1,     1,     7,     9,     7,     6,     8,     1,     4,
       4,     1,     1,     4,     2,     1,     0,     1,     1,     1,
       3,     3,     3,     0,     1,     1,     3,     3,     2,     3,
       6,     0,     1,     4,     2,     0,     5,     3,     3,     1,
       6,     4,     4,     2,     2,     0,     5,     3,     3,     1,
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     4,     4,     4,     2,     2,     4,     2,     2,
       1,     3,     3,     3,     0,     2,     5,     6,     6,     7,
       1,     2,     1,     2,     1,     4,     1,     4,     3,     0,
       1,     3,     2,     3,     1,     1,     0,     0,     2,     2,
       2,     4,     2,     5,     3,     1,     1,     0,     3,     4,
       5,     3,     1,     2,     0,     4,     1,     3,     2,     2,
       2,     1,     1,     1,     1,     3,     4,     6,     1,     8,
       6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   411,     0,   775,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   865,     0,
     853,   657,     0,   663,   664,   665,    22,   722,   842,   138,
     139,   666,     0,   119,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   170,     0,     0,     0,     0,     0,     0,
     377,   378,   379,   382,   381,   380,     0,     0,     0,     0,
     199,     0,     0,     0,   670,   672,   673,   667,   668,     0,
       0,     0,   674,   669,     0,   641,    24,    25,    26,    28,
      27,     0,   671,     0,     0,     0,     0,   675,   383,   510,
       0,   137,   109,     0,   658,     0,     0,     4,    99,   101,
     721,     0,   640,     0,     6,   169,     7,     9,     8,    10,
       0,     0,   375,   422,     0,     0,     0,     0,     0,     0,
       0,   420,   831,   832,   492,   491,   405,   495,     0,   404,
     802,   642,   649,     0,   724,   490,   374,   805,   806,   817,
     421,     0,     0,   424,   423,   803,   804,   801,   838,   841,
     480,   723,    11,   382,   381,   380,     0,     0,    28,     0,
      99,   169,     0,   909,   421,   908,     0,   906,   905,   494,
       0,   412,   417,     0,     0,   462,   463,   464,   465,   489,
     487,   486,   485,   484,   483,   482,   481,   842,   666,   644,
       0,     0,   929,   824,   642,     0,   643,   444,     0,   442,
       0,   869,     0,   731,   403,   653,   189,     0,   929,   402,
     652,   647,     0,   662,   643,   848,   849,   855,   847,   654,
       0,     0,   656,   488,     0,     0,     0,     0,   408,     0,
     117,   410,     0,     0,   123,   125,     0,     0,   127,     0,
      69,    68,    63,    62,    54,    55,    46,    66,    77,     0,
      49,     0,    61,    53,    59,    79,    72,    71,    44,    67,
      86,    87,    45,    82,    42,    83,    43,    84,    41,    88,
      76,    80,    85,    73,    74,    48,    75,    78,    40,    70,
      56,    89,    64,    57,    47,    39,    38,    37,    36,    35,
      34,    58,    90,    92,    51,    32,    33,    60,   962,   963,
      52,   968,    31,    50,    81,     0,     0,    99,    91,   920,
     961,     0,   964,     0,     0,   129,     0,     0,   160,     0,
       0,     0,     0,     0,     0,    94,    95,   288,     0,     0,
     287,     0,   203,     0,   200,   293,     0,     0,     0,     0,
       0,   926,   185,   197,   861,   865,     0,   890,     0,   677,
       0,     0,     0,   888,     0,    16,     0,   103,   177,   191,
     198,   547,   522,     0,   914,   502,   504,   506,   779,   411,
     422,     0,     0,   420,   421,   423,     0,     0,   844,   659,
       0,   660,     0,     0,     0,   159,     0,     0,   105,   279,
       0,    21,   168,     0,   196,   181,   195,   380,   383,   169,
     376,   152,   153,   154,   155,   156,   158,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   853,     0,   151,   846,   846,   875,
       0,     0,     0,     0,     0,     0,     0,     0,   373,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   443,   441,   780,   781,     0,   846,     0,   793,
     279,   279,   846,     0,   861,     0,   169,     0,     0,   131,
       0,   777,   772,   731,     0,   422,   420,     0,   873,     0,
     527,   730,   864,   662,   422,   420,   421,   105,     0,   279,
     401,     0,   795,   655,     0,   109,   239,     0,   509,     0,
     134,     0,     0,   409,     0,     0,     0,     0,     0,   126,
     150,   128,   962,   963,   959,   960,     0,   954,     0,     0,
       0,     0,    65,    30,    52,    29,   921,   157,   130,   109,
       0,   147,   149,     0,     0,    96,     0,    18,     0,     0,
     289,     0,   132,   202,   201,     0,     0,   133,   910,     0,
       0,   422,   420,   421,   424,   423,     0,   947,   209,     0,
     862,     0,     0,   135,     0,     0,   676,   889,   722,     0,
       0,   887,   727,   886,   102,     5,    13,    14,     0,   207,
       0,     0,   515,     0,     0,   731,     0,     0,   650,   645,
     516,     0,     0,     0,     0,   779,   109,     0,   733,   778,
     972,   400,   414,   476,   811,   830,   114,   108,   110,   111,
     112,   113,   374,     0,   493,   725,   726,   100,   731,     0,
     930,     0,     0,     0,   733,   280,     0,   498,   171,   205,
       0,   447,   449,   448,     0,     0,   479,   445,   446,   450,
     452,   451,   467,   466,   469,   468,   470,   472,   474,   473,
     471,   461,   460,   454,   455,   453,   456,   457,   459,   475,
     458,   845,     0,     0,   879,     0,   731,   913,     0,   912,
     929,   808,   187,   179,   193,     0,   914,   183,   169,     0,
     415,   418,   426,   440,   439,   438,   437,   436,   435,   434,
     433,   432,   431,   430,   429,   783,     0,   782,   785,   807,
     789,   929,   786,     0,     0,     0,     0,     0,     0,     0,
       0,   907,   413,   770,   774,   730,   776,     0,   646,     0,
     868,     0,   867,   205,     0,   646,   852,   851,   838,   841,
       0,     0,   782,   785,   850,   786,   406,   241,   243,   109,
     513,   512,   407,     0,   109,   223,   118,   410,     0,     0,
       0,     0,     0,   235,   235,   124,     0,     0,     0,     0,
     952,   731,     0,   936,     0,     0,     0,     0,     0,   729,
       0,   641,     0,     0,   679,   640,   684,     0,   678,   107,
     683,   929,   965,     0,     0,     0,    19,    20,     0,    93,
      97,     0,   286,   294,   291,     0,     0,   899,   904,   901,
     900,   903,   902,    12,   945,   946,     0,     0,     0,     0,
     861,   858,     0,   526,   898,   897,   896,     0,   892,     0,
     893,   895,     0,     5,     0,     0,     0,   541,   542,   550,
     549,     0,   420,     0,   730,   521,   525,     0,     0,   915,
       0,   503,     0,     0,   937,   779,   265,   971,     0,     0,
     794,     0,   843,   730,   932,   928,   281,   282,   639,   732,
     278,     0,   779,     0,     0,   207,   500,   173,   478,     0,
     530,   531,     0,   528,   730,   874,     0,     0,   279,   209,
       0,   207,     0,     0,   205,     0,   853,   427,     0,     0,
     791,   792,   809,   810,   839,   840,     0,     0,     0,   758,
     738,   739,   740,   747,     0,     0,     0,   751,   749,   750,
     764,   731,     0,   772,   872,   871,     0,   207,     0,   796,
     661,     0,   245,     0,     0,   115,     0,     0,     0,     0,
       0,     0,     0,   215,   216,   227,     0,   109,   225,   144,
     235,     0,   235,     0,     0,   966,     0,     0,     0,   730,
     953,   955,   935,   731,   934,     0,   731,   705,   706,   703,
     704,   737,     0,   731,   729,     0,   524,     0,     0,   881,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   958,   161,
       0,   164,   148,    98,   290,     0,   911,   136,   947,   927,
     942,   208,   210,   300,     0,     0,   859,     0,   891,     0,
      17,     0,   914,   206,   300,     0,     0,   646,   518,     0,
     651,   916,     0,   937,   507,     0,     0,   972,     0,   270,
     268,   785,   797,   929,   785,   798,   931,     0,     0,   283,
     106,     0,   779,   204,     0,   779,     0,   477,   878,   877,
       0,   279,     0,     0,     0,     0,     0,     0,   207,   175,
     662,   784,   279,     0,   743,   744,   745,   746,   752,   753,
     762,     0,   731,     0,   758,     0,   742,   766,   730,   769,
     771,   773,     0,   866,     0,   784,     0,     0,     0,     0,
     242,   514,   120,     0,   410,   215,   217,   861,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   229,     0,     0,
     948,     0,   951,   730,     0,     0,     0,   681,   730,   728,
       0,   719,     0,   731,     0,   685,   720,   718,   885,     0,
     731,   688,   690,   689,     0,     0,   686,   687,   691,   693,
     692,   708,   707,   710,   709,   711,   713,   715,   714,   712,
     701,   700,   695,   696,   694,   697,   698,   699,   702,   957,
       0,   109,   292,     0,     0,     0,   944,     0,   374,   863,
     861,   416,   419,   425,     0,    15,     0,   374,   553,     0,
       0,   555,   548,   551,     0,   546,     0,   918,     0,   938,
     511,     0,   271,     0,     0,   266,     0,   285,   284,   937,
       0,   300,     0,   779,     0,   279,     0,   836,   300,   914,
     300,   917,     0,     0,     0,   428,     0,     0,   755,   730,
     757,   748,     0,   741,     0,     0,   731,   763,   870,   300,
       0,   109,     0,   238,   224,     0,     0,     0,   214,   140,
     228,     0,     0,   231,     0,   236,   237,   109,   230,   967,
     949,     0,   933,     0,   970,   736,   735,   680,     0,   730,
     523,   682,     0,   529,   730,   880,   717,     0,     0,     0,
     941,   939,   940,   211,     0,     0,     0,   381,   372,     0,
       0,     0,   186,   299,   301,     0,   371,     0,     0,     0,
     914,   374,     0,   894,   296,   192,   544,     0,     0,   517,
     505,     0,   274,   264,     0,   267,   273,   279,   497,   937,
     374,   937,     0,   876,     0,   835,   374,     0,   374,   919,
     300,   779,   833,   761,   760,   754,     0,   756,   730,   765,
     374,   109,   244,   116,   121,   142,   218,     0,   226,   232,
     109,   234,   950,     0,     0,   520,     0,   884,   883,   716,
     109,   165,   943,     0,     0,     0,   922,     0,     0,     0,
     212,     0,   914,     0,   337,   333,   339,   641,    28,     0,
     327,     0,   332,   336,   349,     0,   347,   352,     0,   351,
       0,   350,     0,   169,   303,     0,   305,     0,   306,   307,
       0,     0,   860,     0,   545,   543,   554,   552,   275,     0,
       0,   262,   272,     0,     0,     0,     0,   182,   497,   937,
     837,   188,   296,   194,   374,     0,     0,   768,     0,   190,
     240,     0,     0,   109,   221,   141,   233,   969,   734,     0,
       0,     0,     0,     0,   399,     0,   923,     0,   317,   321,
     396,   397,   331,     0,     0,     0,   312,   605,   604,   601,
     603,   602,   622,   624,   623,   593,   564,   565,   583,   599,
     598,   560,   570,   571,   573,   572,   592,   576,   574,   575,
     577,   578,   579,   580,   581,   582,   584,   585,   586,   587,
     588,   589,   591,   590,   561,   562,   563,   566,   567,   569,
     607,   608,   617,   616,   615,   614,   613,   612,   600,   619,
     609,   610,   611,   594,   595,   596,   597,   620,   621,   625,
     627,   626,   628,   629,   606,   631,   630,   633,   635,   634,
     568,   638,   636,   637,   632,   618,   559,   344,   556,     0,
     313,   365,   366,   364,   357,     0,   358,   314,   391,     0,
       0,     0,     0,   395,     0,   169,   178,   295,     0,     0,
       0,   263,   277,   834,     0,   109,   367,   109,   172,     0,
       0,     0,   184,   937,   759,     0,   109,   219,   122,   143,
       0,   519,   882,   163,   315,   316,   394,   213,     0,     0,
     731,     0,   340,   328,     0,     0,     0,   346,   348,     0,
       0,   353,   360,   361,   359,     0,     0,   302,   924,     0,
       0,     0,   398,     0,   297,     0,   276,     0,   539,   733,
       0,     0,   109,   174,   180,     0,   767,     0,     0,   145,
     318,    99,     0,   319,   320,     0,     0,   334,   730,   342,
     338,   343,   557,   558,     0,   329,   362,   363,   355,   356,
     354,   392,   389,   947,   308,   304,   393,     0,   298,   540,
     732,     0,   499,   368,     0,   176,     0,   222,     0,   167,
       0,   374,     0,   341,   345,     0,     0,   779,   310,     0,
     537,   496,   501,   220,     0,     0,   146,   325,     0,   373,
     335,   390,   925,     0,   733,   385,   779,   538,     0,   166,
       0,     0,   324,   937,   779,   249,   386,   387,   388,   972,
     384,     0,     0,     0,   323,     0,   385,     0,   937,     0,
     322,   369,   109,   309,   972,     0,   254,   252,     0,   109,
       0,     0,   255,     0,     0,   250,   311,     0,   370,     0,
     258,   248,     0,   251,   257,   162,   259,     0,     0,   246,
     256,     0,   247,   261,   260
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   843,   595,   170,  1386,   690,
     334,   335,   109,   110,   111,   112,   113,   386,   627,   628,
     522,   239,  1451,   528,  1367,  1452,  1689,   805,   329,   549,
    1649,  1022,  1191,  1706,   403,   171,   629,   883,  1076,  1244,
     117,   598,   900,   630,   649,   904,   578,   899,   219,   503,
     631,   599,   901,   405,   352,   369,   120,   885,   846,   829,
    1031,  1389,  1129,   953,  1598,  1455,   766,   959,   527,   775,
     961,  1277,   758,   942,   945,  1118,  1713,  1714,   617,   618,
     643,   644,   339,   340,   346,  1423,  1577,  1578,  1198,  1313,
    1412,  1571,  1697,  1716,  1608,  1653,  1654,  1655,  1399,  1400,
    1401,  1402,  1610,  1611,  1617,  1665,  1405,  1406,  1410,  1564,
    1565,  1566,  1588,  1743,  1314,  1315,   172,   122,  1729,  1730,
    1569,  1317,  1318,  1319,  1320,   123,   232,   523,   524,   124,
     125,   126,   127,   128,   129,   130,   131,  1435,   132,   882,
    1075,   133,   614,   615,   616,   236,   378,   518,   604,   605,
    1153,   606,  1154,   134,   135,   136,   796,   137,   138,  1639,
     139,   600,  1425,   601,  1045,   851,  1215,  1212,  1557,  1558,
     140,   141,   142,   222,   143,   223,   233,   390,   510,   144,
     981,   800,   145,   982,   874,   866,   983,   928,  1098,   929,
    1100,  1101,  1102,   931,  1255,  1256,   932,   734,   493,   183,
     184,   632,   620,   476,  1061,  1062,   720,   721,   870,   147,
     225,   148,   149,   174,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   682,   229,   230,   581,   212,   213,   685,
     686,  1159,  1160,   362,   363,   837,   160,   569,   161,   613,
     162,   321,  1579,  1629,   353,   398,   638,   639,   975,  1056,
    1196,   826,   827,   780,   781,   782,   322,   323,   802,  1388,
     868
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1443
static const yytype_int16 yypact[] =
{
   -1443,   159, -1443, -1443,  5639, 13205, 13205,    -7, 13205, 13205,
   13205, 11071, 13205, -1443, 13205, 13205, 13205, 13205, 13205, 13205,
   13205, 13205, 13205, 13205, 13205, 13205, 15578, 15578, 11265, 13205,
   15975,    41,   198, -1443, -1443, -1443, -1443, -1443,   186, -1443,
   -1443,   185, 13205, -1443,   198,   208,   224,   258, -1443,   198,
   11459,   572, 11653, -1443, 13955, 10101,   166, 13205,  1517,    87,
   -1443, -1443, -1443,   399,   261,    65,   297,   314,   330,   359,
   -1443,   572,   381,   384, -1443, -1443, -1443, -1443, -1443, 13205,
     524,   639, -1443, -1443,   572, -1443, -1443, -1443, -1443,   572,
   -1443,   572, -1443,   291,   387,   572,   572, -1443,   486, -1443,
   11847, -1443, -1443,   215,   566,   660,   660, -1443,   506,   421,
       8,   394, -1443,    88, -1443,   550, -1443, -1443, -1443, -1443,
     982,   762, -1443, -1443,   408,   415,   423,   435,   439,   468,
    3032, -1443, -1443, -1443, -1443,    83, -1443,   515,   587, -1443,
     147,   450, -1443,   525,     5, -1443,  3051,   155, -1443, -1443,
    2651,   142,   498,   165, -1443,   143,    64,   500,   179, -1443,
   -1443,   626, -1443, -1443, -1443,   552,   523,   556, -1443, 13205,
   -1443,   550,   762, 16440,  2880, 16440, 13205, 16440, 16440,  4284,
     533,  4618,  4284,   668,   572,   670,   670,   132,   670,   670,
     670,   670,   670,   670,   670,   670,   670, -1443, -1443, -1443,
      55, 13205,   567, -1443, -1443,   590,   558,   398,   568,   398,
   15578, 15277,   564,   751, -1443,   552, -1443, 13205,   567, -1443,
     614, -1443,   620,   603, -1443,   151, -1443, -1443, -1443,   398,
     142, 12041, -1443, -1443, 13205,  8937,   766,    94, 16440,  9907,
   -1443, 13205, 13205,   572, -1443, -1443,  4025,   588, -1443, 13190,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,  2628,
   -1443,  2628, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,    82,    73,
     556, -1443, -1443, -1443, -1443,   610,  1950,    92, -1443, -1443,
     648,   790, -1443,   657, 14716, -1443,   635, 13384, -1443,    44,
   14562,   817,   817,   572,    96, -1443,    62, -1443,  3165,    98,
   -1443,   699, -1443,   700, -1443,   830,   108, 15578, 13205, 13205,
     659,   677, -1443, -1443, 15295, 11265,   109,   459,   451, -1443,
   13399, 15578,   540, -1443,   572, -1443,   424,   421, -1443, -1443,
   -1443, -1443, 16068,   837,   754, -1443, -1443, -1443,    89, 13205,
     666,   671, 16440,   672,  1025,   673,  5833, 13205, -1443,   528,
     663,   662,   528,   508,   466, -1443,   572,  2628,   675, 10295,
   13955, -1443, -1443,   849, -1443, -1443, -1443, -1443, -1443,   550,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, 13205, 13205, 13205,
   12235, 13205, 13205, 13205, 13205, 13205, 13205, 13205, 13205, 13205,
   13205, 13205, 13205, 13205, 13205, 13205, 13205, 13205, 13205, 13205,
   13205, 13205, 13205, 13205, 16161, 13205, -1443, 13205, 13205, 13205,
    4717,   572,   572,   572,   572,   572,   982,   756,  1400,  4944,
   13205, 13205, 13205, 13205, 13205, 13205, 13205, 13205, 13205, 13205,
   13205, 13205, -1443, -1443, -1443, -1443,  1026, 13205, 13205, -1443,
   10295, 10295, 13205, 13205, 15295,   678,   550, 12429, 14608, -1443,
   13205, -1443,   679,   862,   723,   685,   689, 13538,   398, 12623,
   -1443, 12817, -1443,   603,   697,   698,  1716, -1443,   277, 10295,
   -1443,  1503, -1443, -1443, 14654, -1443, -1443, 10489, -1443, 13205,
   -1443,   797,  9131,   881,   703, 16323,   883,    72,    85, -1443,
   -1443, -1443,   722, -1443, -1443, -1443,  2628,   482,   711,   897,
   15202,   572, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
     720, -1443, -1443,   110,   111,    66,   817, -1443,   572, 13205,
     398,    87, -1443, -1443, -1443, 15202,   827, -1443,   398,    76,
      80,   725,   727,  1911,   173,   728,   731,   493,   782,   734,
     398,   100,   736, -1443,  1609,   572, -1443, -1443,   844,  2825,
      22, -1443, -1443, -1443,   421, -1443, -1443, -1443,   892,   799,
     758,   356,   783, 13205,   805,   930,   764,   792, -1443,   156,
   -1443,  2628,  2628,   945,   766,    89, -1443,   776,   954, -1443,
    2628,   163, -1443,   447,   167, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443,  1822,  2932, -1443, -1443, -1443, -1443,   955,   801,
   -1443, 15578, 13205,   778,   973, 16440,   969, -1443, -1443,   855,
    1185, 11638, 16615,  4284, 13205, 16394, 14399, 16686, 10275,  4423,
    5171, 12406, 12600, 12600, 12600, 12600,  3485,  3485,  3485,  3485,
    3485,   788,   788,   712,   712,   712,   132,   132,   132, -1443,
     670, 16440,   791,   793, 15740,   800,   979,     4, 13205,   199,
     567,   172, -1443, -1443, -1443,   980,   754, -1443,   550, 15392,
   -1443, -1443,  4284,  4284,  4284,  4284,  4284,  4284,  4284,  4284,
    4284,  4284,  4284,  4284,  4284, -1443, 13205,   350, -1443,   157,
   -1443,   567,   352,   803,  3108,   807,   811,   806,  3405,   128,
     814, -1443, 16440,  2226, -1443,   572, -1443,   163,    43, 15578,
   16440, 15578, 15786,   855,   163,   398,   176, -1443,   156,   841,
     815, 13205, -1443,   280, -1443, -1443, -1443,  8743,   492, -1443,
   -1443, 16440, 16440,   198, -1443, -1443, -1443, 13205,   903, 15085,
   15202,   572,  9325,   821,   825, -1443,    74,   927,   885,   864,
   -1443,  1010,   835,  2124,  2628, 15202, 15202, 15202, 15202, 15202,
     840,   872,   842, 15202,   382,   879, -1443,   845, -1443, 16530,
   -1443,    20, -1443,  6027,  1451,   848, -1443, -1443,   572, -1443,
   -1443,  3683, -1443, 16530,  1039, 15578,   866, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443,   119,   572,  1451,   868,
   15295, 15485,  1042, -1443, -1443, -1443, -1443,   870, -1443, 13205,
   -1443, -1443,  5251, -1443,  2628,  1451,   875, -1443, -1443, -1443,
   -1443,  1056,   884, 13205, 16068, -1443, -1443,  4717,   880, -1443,
    2628, -1443,   894,  6221,  1044,   141, -1443, -1443,   116,  1026,
   -1443,  1503, -1443,  2628, -1443, -1443,   398, 16440, -1443, 10683,
   -1443, 15202,    61,   890,  1451,   799, -1443, -1443, 14399, 13205,
   -1443, -1443, 13205, -1443, 13205, -1443,  4196,   893, 10295,   782,
    1051,   799,  2628,  1069,   855,   572, 16161,   398,  4244,   899,
   -1443, -1443,   189,   902, -1443, -1443,  1070,  1285,  1285,  2226,
   -1443, -1443, -1443,  1043,   905,    58,   907, -1443, -1443, -1443,
   -1443,  1088,   910,   679,   398,   398, 13011,   799,  1503, -1443,
   -1443,  5112,   607,   198,  9907, -1443,  6415,   911,  6609,   914,
   15085, 15578,   922,   981,   398, 16530,  1101, -1443, -1443, -1443,
   -1443,   516, -1443,   295,  2628, -1443,   984,  2628,   572,   482,
   -1443, -1443, -1443,  1104, -1443,   932,   955,   701,   701,  1054,
    1054,  4120,   928,  1116, 15202, 14984, 16068,  3570, 14850, 15202,
   15202, 15202, 15202,  3815, 15202, 15202, 15202, 15202, 15202, 15202,
   15202, 15202, 15202, 15202, 15202, 15202, 15202, 15202, 15202, 15202,
   15202, 15202, 15202, 15202, 15202, 15202, 15202,   572, -1443, -1443,
    1046, -1443, -1443, -1443, -1443, 15202,   398, -1443,   493, -1443,
     602,  1119, -1443, -1443,   130,   937,   398, 10877, -1443,  2507,
   -1443,  5445,   754,  1119, -1443,   402,     0, -1443, 16440,   997,
     961, -1443,   963,  1044, -1443,  2628,   766,  2628,    86,  1136,
    1073,   303, -1443,   567,   315, -1443, -1443, 15578, 13205, 16440,
   16530,   970,    61, -1443,   971,    61,   972, 14399, 16440, 15845,
     976, 10295,   978,   989,  2628,   990,   992,  2628,   799, -1443,
     603,   389, 10295, 13205, -1443, -1443, -1443, -1443, -1443, -1443,
    1027,   993,  1161,  1103,  2226,  1045, -1443, 16068,  2226, -1443,
   -1443, -1443, 15578, 16440,  1002, -1443,   198,  1163,  1125,  9907,
   -1443, -1443, -1443,  1012, 13205,   981,   398, 15295, 15085,  1014,
   15202,  6803,   617,  1015, 13205,    97,   300, -1443,  1033,  2628,
   -1443,  1077, -1443,  2182,  1182,  1034, 15202, -1443, 15202, -1443,
    1037, -1443,  1093,  1218,  1040, -1443, -1443, -1443, 15891,  1048,
    1231, 16573,  3447,  4465, 15202, 16486, 16721, 10663,  4875, 11243,
    3790, 12794, 12794, 12794, 12794,  3583,  3583,  3583,  3583,  3583,
    1084,  1084,   701,   701,   701,  1054,  1054,  1054,  1054, -1443,
    1057, -1443, 16530,   572,  2628,  2628, -1443,  1451,   547, -1443,
   15295, -1443, -1443,  4284,  1052, -1443,  1058,  1055, -1443,   220,
   13205, -1443, -1443, -1443, 13205, -1443, 13205, -1443,   766, -1443,
   -1443,   164,  1228,  1168, 13205, -1443,  1066,   398, 16440,  1044,
    1067, -1443,  1086,    61, 13205, 10295,  1087, -1443, -1443,   754,
   -1443, -1443,  1095,  1097,  1090, -1443,  1102,  2226, -1443,  2226,
   -1443, -1443,  1106, -1443,  1111,  1108,  1283, -1443,   398, -1443,
    1269, -1443,  1112, -1443, -1443,  1117,  1120,   131, -1443, -1443,
   16530,  1114,  1118, -1443, 12026, -1443, -1443, -1443, -1443, -1443,
   -1443,  2628, -1443,  2628, -1443, 16530, 15950, -1443, 15202, 16068,
   -1443, -1443, 15202, -1443, 15202, -1443, 16651, 15202,  1110,  6997,
     602, -1443, -1443, -1443,   651, 14094,  1451,  1200, -1443,  1917,
    1153,  1099, -1443, -1443, -1443,   756,  2069,   112,   113,  1126,
     754,  1400,   134, -1443, -1443, -1443,  1159, 11056, 11444, 16440,
   -1443,   286,  1303,  1238, 13205, -1443, 16440, 10295,  1207,  1044,
    1151,  1044,  1137, 16440,  1138, -1443,  1219,  1134,  1353, -1443,
   -1443,    61, -1443, -1443,  1194, -1443,  2226, -1443, 16068, -1443,
    1527, -1443,  8743, -1443, -1443, -1443, -1443,  9519, -1443, -1443,
   -1443,  8743, -1443,  1142, 15202, 16530,  1198, 16530, 15994, 16651,
   -1443, -1443, -1443,  1451,  1451,   572, -1443,  1329, 14984,    77,
   -1443, 14094,   754,  2330, -1443,  1173, -1443,   115,  1160,   117,
   -1443, 14401, -1443, -1443, -1443,   122, -1443, -1443,   536, -1443,
    1162, -1443,  1271,   550, -1443, 14233, -1443, 14233, -1443, -1443,
    1339,   756, -1443, 13677, -1443, -1443, -1443, -1443,  1341,  1275,
   13205, -1443, 16440,  1170,  1172,  1180,   597, -1443,  1207,  1044,
   -1443, -1443, -1443, -1443,  1605,  1183,  2226, -1443,  1240, -1443,
    8743,  9713,  9519, -1443, -1443, -1443,  8743, -1443, 16530, 15202,
   15202,  7191,  1186,  1187, -1443, 15202, -1443,  1451, -1443, -1443,
   -1443, -1443, -1443,  2628,  2853,  1917, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443,   171, -1443,  1153,
   -1443, -1443, -1443, -1443, -1443,    70,   349, -1443,  1360,   124,
   14716,  1271,  1363, -1443,  2628,   550, -1443, -1443,  1191,  1368,
   13205, -1443, 16440, -1443,   127, -1443, -1443, -1443, -1443,  1193,
     597, 13816, -1443,  1044, -1443,  2226, -1443, -1443, -1443, -1443,
    7385, 16530, 16530, -1443, -1443, -1443, 16530, -1443,   629,   129,
    1375,  1195, -1443, -1443, 15202, 14401, 14401,  1331, -1443,   536,
     536,   358, -1443, -1443, -1443, 15202,  1309, -1443,  1221,  1204,
     125, 15202, -1443,   572, -1443, 15202, 16440,  1311, -1443,  1387,
    7579,  7773, -1443, -1443, -1443,   597, -1443,  7967,  1210,  1291,
   -1443,  1305,  1254, -1443, -1443,  1308,  2628, -1443,  2853, -1443,
   -1443, 16530, -1443, -1443,  1244, -1443,  1377, -1443, -1443, -1443,
   -1443, 16530,  1395,   493, -1443, -1443, 16530,  1226, 16530, -1443,
     140,  1227, -1443, -1443,  8161, -1443,  1225, -1443,  1229,  1248,
     572,  1400,  1245, -1443, -1443, 15202,   146,   102, -1443,  1343,
   -1443, -1443, -1443, -1443,  1451,   848, -1443,  1253,   572,  1115,
   -1443, 16530, -1443,  1234,  1417,   609,   102, -1443,  1346, -1443,
    1451,  1239, -1443,  1044,   103, -1443, -1443, -1443, -1443,  2628,
   -1443,  1241,  1243,   126, -1443,   618,   609,   169,  1044,  1242,
   -1443, -1443, -1443, -1443,  2628,   321,  1419,  1354,   618, -1443,
    8355,   170,  1425,  1359, 13205, -1443, -1443,  8549, -1443,   329,
    1428,  1362, 13205, -1443, 16440, -1443,  1437,  1371, 13205, -1443,
   16440, 13205, -1443, 16440, 16440
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1443, -1443, -1443,  -515, -1443, -1443, -1443,   449,     1,   -32,
     484,   898,    32,  1631, -1443,  2801, -1443,  -447, -1443,    34,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443,  -252, -1443, -1443,  -163,   106,    25, -1443, -1443, -1443,
   -1443, -1443, -1443,    26, -1443, -1443, -1443, -1443, -1443, -1443,
      27, -1443, -1443,  1001,  1007,  1006,   -82,  -599,  -808,   561,
     619,  -244,   353,  -852, -1443,    28, -1443, -1443, -1443, -1443,
    -701,   205, -1443, -1443, -1443, -1443,  -235, -1443,  -573, -1443,
    -410, -1443, -1443,   921, -1443,    42, -1443, -1443,  -975, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,    14,
   -1443,   101, -1443, -1443, -1443, -1443, -1443,   -68, -1443,   181,
    -801, -1443, -1442,  -253, -1443,  -125,   162,  -119,  -237, -1443,
     -74, -1443, -1443, -1443,   193,   -28,     7,    33,  -683,   -75,
   -1443, -1443,   -20, -1443, -1443,    -5,   -39,    63, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443,  -566,  -800, -1443,
   -1443, -1443, -1443, -1443,  1613, -1443, -1443, -1443, -1443, -1443,
     458, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,  -793,
   -1443,  2395,    30, -1443,  1461,  -382, -1443, -1443,  -459,  3571,
    3611, -1443, -1443,   520,  -155,  -618, -1443, -1443,   589,   403,
    -641,   401, -1443, -1443, -1443, -1443, -1443,   576, -1443, -1443,
   -1443,    90,  -806,  -115,  -400,  -388, -1443,   642,  -105, -1443,
   -1443,    36,    37,   757, -1443, -1443,   724,   -22, -1443,  -336,
      35,  -338,    99,  -239, -1443, -1443,  -441,  1177, -1443, -1443,
   -1443, -1443, -1443,    78,   889, -1443, -1443, -1443,  -332,  -596,
   -1443,  1123,  -858, -1443,   -63,  -169,    48,   742, -1443,  -990,
     228,  -138,   509,   571, -1443, -1443, -1443, -1443,   526,   -27,
   -1016
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -957
static const yytype_int16 yytable[] =
{
     173,   175,   410,   177,   178,   179,   181,   182,   485,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   457,   319,   211,   214,   381,   880,   324,   370,   116,
     118,   119,   373,   374,   609,   326,   608,   238,   114,   235,
     610,  1221,   862,   729,   743,   246,   479,   249,   861,   507,
     327,   240,   330,   410,  1049,   318,   244,   406,   502,  1057,
     221,   383,   679,  1218,   456,   228,   226,   227,   757,  1207,
     725,   726,   380,   963,   238,   237,   718,  1074,   385,  1619,
     842,   771,   -30,   964,   949,   815,  1467,   -30,   719,   519,
     336,   -65,   930,  1085,   146,   382,   -65,   400,  1125,   750,
     903,   -29,   803,   519,  1620,   556,   -29,   561,    13,   831,
     115,   753,   356,   366,   511,   773,   367,   566,   519,   556,
     556,  1415,  1417,   754,  -330,   512,  1475,  1275,  1028,  1114,
     383,  1559,  -814,  1626,  1626,  1467,    13,   831,  1656,   831,
     831,   380,   494,   831,   937,  1105,   345,   385,  1643,    13,
      13,  -532,  1058,   474,   475,  1028,   550,  -643,  1213,     3,
    -929,   558,   496,  1637,   382,   808,   121,  1222,   337,   863,
     748,   488,  1017,   495,   474,   475,  1699,   -92,   505,   385,
     176,   397,   391,   393,   394,  1614,  1152,   444,    13,   504,
    1214,   -92,  -825,   397,  -508,  -929,   382,  1059,  -929,   445,
    1331,  1615,   388,  1685,   359,  1745,  1759,  1106,  1638,   683,
     477,  -813,   382,   841,   551,  -812,  -650,  -644,   396,  -854,
    1616,  1700,   396,  -815,   482,  -816,   514,   409,   231,   514,
    -827,   477,   534,  -534,   535,  -819,   238,   525,   723,  1338,
    -820,  -534,   585,   727,  -857,  1332,   650,   482,   965,  -269,
    1746,  1760,  1223,  -535,  -814,   536,  1340,  -818,  1621,  1132,
     772,  1136,   -30,  1346,   816,  1348,  1468,  1469,   817,   516,
    -824,   -65,  1326,   521,  -269,   774,  1268,   401,   486,   338,
    1243,   -29,  1060,   520,  1360,   557,  1276,   562,   832,   539,
    -253,  -732,   546,  1029,   474,   475,   197,   567,   583,   806,
     807,  1416,  1418,  1657,  -330,  1088,  1476,  1254,   458,  1071,
     572,  1560,   946,  1627,  1675,  1740,   916,   948,  1199,  1366,
    1712,   571,  1422,   730,  -651,   318,  -732,   575,  1041,  -732,
    1333,  -645,   478,  -813,  -823,  1747,  1761,  -812,   736,   410,
     648,  -854,  -822,   238,   382,  -815,   483,  -816,  -856,  1436,
     211,  1438,   481,   478,  -826,   589,   328,  -819,   197,   898,
     481,   821,  -820,   336,   336,   555,  -857,  1428,   319,   483,
     640,  -799,   -91,   324,   181,  1444,  -829,   371,  1622,  -818,
     343,   570,   633,  -800,   700,   234,   -91,  1668,   344,  1034,
     370,   695,   696,   406,   645,   241,   594,  1623,  1134,  1135,
    1624,   318,  1752,  1134,  1135,   387,  1669,   103,   848,  1670,
    1766,   242,   651,   652,   653,   655,   656,   657,   658,   659,
     660,   661,   662,   663,   664,   665,   666,   667,   668,   669,
     670,   671,   672,   673,   674,   675,   676,   677,   678,   701,
     680,  1265,   681,   681,   684,   243,  1206,  1387,   371,  1590,
     855,   689,  1429,   108,   702,   703,   704,   705,   706,   707,
     708,   709,   710,   711,   712,   713,   714,  1257,   619,   103,
    -856,  1064,   681,   724,   221,   645,   645,   681,   728,   228,
     226,   227,   702,  1065,   347,   732,  1137,  1753,  1082,  1376,
    1220,  1278,   115,  -799,   740,  1767,   742,   760,   318,  1230,
     247,   348,  1232,   317,   645,  -800,   869,   457,   871,   776,
    1131,   777,   761,   849,   762,  -536,   609,   349,   608,   341,
     351,   897,   610,  -787,  1090,  -790,   342,   584,   850,   474,
     475,   895,   357,  1470,  -929,   905,   395,  -787,   368,  -790,
     351,   824,   825,   747,   351,   351,   350,   357,   121,   691,
     456,  1304,   909,   591,   811,   397,   765,  1572,  1448,  1573,
    1208,    36,  -788,   698,   852,   943,   944,   375,   354,   351,
     778,   355,  -929,  1209,   372,   722,  -788,   396,   474,   475,
     396,   399,    48,   447,   858,   859,   402,   887,   336,   357,
     474,   475,  1210,   867,    13,   591,   691,   411,   382,  -646,
     586,   360,   361,  1645,   412,   357,  1353,   749,  1354,   357,
     755,   358,   413,   596,   597,    36,   360,   361,  1133,  1134,
    1135,   357,   869,   871,   414,   507,   970,   591,   415,   938,
     871,  1245,  1018,   492,  -827,   167,    48,   877,    84,   939,
     449,    86,    87,  1347,    88,   168,    90,   357,   609,   888,
     608,    36,  1330,   389,   610,   448,  1305,   416,   360,   361,
    1342,  1306,   636,    60,    61,    62,   163,  1307,   407,  1308,
     376,  1236,    48,   359,   360,   361,   377,   450,   360,   361,
    1116,  1117,  1246,   896,   108,   480,  1267,  -821,   108,   592,
     360,   361,   526,  1561,  -533,    86,    87,  1562,    88,   168,
      90,  1194,  1195,   635,  -644,   619,  1309,  1310,    36,  1311,
     484,   908,   364,  1737,   491,  1447,   360,   361,    36,  1272,
    1134,  1135,   489,  1408,  1420,  1726,  1727,  1728,  1751,    48,
     408,    86,    87,  1735,    88,   168,    90,   445,  1312,    48,
     397,   357,   497,   357,  1299,  -825,   941,   392,  1748,   591,
     208,   208,  1013,  1014,  1015,   481,   640,   640,   500,  1322,
     501,   150,   238,   441,   442,   443,  -642,   444,  1016,   609,
     947,   608,   508,   545,   517,   610,  1109,   530,  1445,   445,
    1383,  1384,   167,   207,   209,    84,  1586,  1587,    86,    87,
     509,    88,   168,    90,   458,   364,  1471,   537,    86,    87,
    -956,    88,   168,    90,   540,  1594,   958,  1741,  1742,   541,
     360,   361,   360,   361,  1362,   553,   554,  1042,  1666,  1667,
    1650,  1145,  1662,  1663,   547,  1344,   563,   564,  1149,   365,
    1371,   973,   976,  1052,  1039,   108,   438,   439,   440,   441,
     442,   443,  1089,   444,   565,   637,  1066,   576,  1048,   317,
     577,   611,   351,   612,   621,   445,   634,   384,   689,   622,
     623,   625,  -104,   115,    53,   647,   733,   116,   118,   119,
      53,   735,   586,   737,  1069,  1086,   114,   738,    60,    61,
      62,   163,   164,   407,  1077,   744,   745,  1078,   763,  1079,
     519,  1715,   767,   645,  1226,   536,    36,   770,   783,   545,
     351,   693,   351,   351,   351,   351,   784,   804,   814,   115,
    1715,   828,   839,   818,  1450,   819,   822,    48,  1736,   121,
     823,   830,   844,  1456,   833,   717,   384,  1433,    36,   845,
     847,  1113,   146,  1461,   208,  -666,   221,  1138,   853,   854,
    1140,   228,   226,   227,   857,   408,   545,  1250,   115,    48,
    1119,   609,   856,   608,  1646,   619,  1050,   610,   384,   860,
     752,   864,  1201,   865,   873,   121,   878,   498,   722,   115,
     755,   108,   619,   333,   506,   875,    86,    87,  1120,    88,
     168,    90,   879,   881,   884,   890,  1151,   891,   894,  1157,
     801,   893,   150,  -648,   902,   912,   150,   910,  1290,   913,
     914,   886,   950,   940,   121,  1295,  1600,   810,    86,    87,
     960,    88,   168,    90,   962,   966,   968,  1202,   967,   969,
     609,  1681,   608,   971,   985,   121,   610,   984,  1219,   986,
     867,   988,  1203,   836,   838,   989,   647,   755,  1021,   487,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   115,  1025,   115,  1027,  1037,  1239,  1033,  1304,
    1242,    36,   208,  1228,  1038,  1044,   116,   118,   119,  1051,
    1046,   208,  1047,   574,  1055,   114,   645,  1072,   208,  1053,
    1081,  1084,    48,  1087,  1093,   208,  1092,   645,  1203,  -828,
     472,   473,  1104,  1103,  1107,   560,  1725,  1108,  1110,   351,
    1122,  1359,    13,  1124,   568,    36,   573,   197,   121,  1127,
     121,   580,  1280,  1143,  1128,  1130,  1066,  1139,   590,   238,
    1144,  1016,  1147,  1260,  1200,  1148,    48,  1190,  1197,  1274,
    1216,   146,  1010,  1011,  1012,  1013,  1014,  1015,  1640,   404,
    1641,    86,    87,   150,    88,   168,    90,   115,   898,  1647,
    1224,  1016,  1217,  1263,  1225,  1304,   474,   475,  1229,  1233,
    1247,  1231,   619,  1235,  1305,   619,  1237,  1301,  1302,  1306,
    1249,    60,    61,    62,   163,  1307,   407,  1308,    36,  1238,
    1240,  1241,   927,   715,   933,    86,    87,  1248,    88,   168,
      90,   923,  1259,  1261,  1253,  1684,  1421,   410,    13,    48,
    1262,  1264,  1269,   121,  1273,  1327,   108,  1279,   208,  1328,
    1281,  1329,  1283,   624,  1309,  1310,   716,  1311,   103,  1336,
     956,   108,  1284,  1304,  1407,  1287,  1288,  1289,  1291,  1343,
     645,    60,    61,    62,   163,   164,   407,   115,   408,  1293,
    1294,   580,  1334,  1323,  1356,  1298,  1325,   587,  1324,  1335,
    1570,   593,   108,  1337,  1372,  1339,  1373,  1023,    86,    87,
    1305,    88,   168,    90,    36,  1306,    13,    60,    61,    62,
     163,  1307,   407,  1308,  1341,  1345,  1030,  1351,   587,   150,
     593,   587,   593,   593,  1349,    48,  1408,  1350,  1321,  1414,
    1352,   108,  1358,   121,  1355,  1750,  1357,  1321,   408,  1361,
    1380,  1363,  1757,  1368,  1722,  1364,   545,  1369,  1365,  1391,
    1309,  1310,   108,  1311,  1404,  1419,  1424,  1430,   717,  1431,
     752,  1434,  1464,   619,  1442,  1439,  1440,  1446,  1305,  1432,
    1457,  1459,   645,  1306,   408,    60,    61,    62,   163,  1307,
     407,  1308,  1437,  1465,    86,    87,  1473,    88,   168,    90,
    1474,  1567,  1568,  1574,   351,  1580,  1581,  1304,  1583,  1584,
    1316,  1094,  1095,  1096,    36,   208,  1097,  1097,   927,  1316,
    1585,  1593,   886,  1595,  1625,  1604,  1605,  1631,  1309,  1310,
    1634,  1311,  1635,  1642,  1658,    48,  1660,   752,  1664,  1466,
    1672,  1674,  1679,   108,  1673,   108,  1680,   108,   876,  1687,
      13,  1454,   408,  1688,  -326,   115,  1690,  1691,  1694,  1695,
    1441,  1620,  1633,  1698,  1703,  1701,  1704,  1141,  1705,  1710,
    1720,  1413,  1723,   208,  1717,  1582,  1724,  1732,  1734,  1738,
    1321,  1739,  1749,  1754,   545,  1755,  1321,   545,  1321,  1762,
    1763,   619,  1768,  1769,    86,    87,  1609,    88,   168,    90,
    1321,  1771,  1772,  1719,   809,  1659,   907,   697,   692,   694,
    1083,   121,  1305,   208,  1043,   208,   801,  1306,   115,    60,
      61,    62,   163,  1307,   407,  1308,  1733,   115,  1266,  1370,
    1599,  1731,   812,   458,  1591,  1597,  1454,   205,   205,  1613,
     108,  1618,  1411,   208,  1472,  1756,   934,  1630,   935,  1744,
    1392,  1589,  1316,  1211,  1150,  1252,  1251,  1099,  1316,  1111,
    1316,  1063,  1309,  1310,   150,  1311,    60,    61,    62,   163,
     164,   407,  1316,   646,   121,   974,   954,  1575,  1382,   150,
      36,  1304,   582,   121,  1321,  1696,   408,  1193,  1628,   208,
    1142,     0,     0,  1189,  1443,     0,     0,  1632,     0,     0,
       0,    48,     0,   927,   208,   208,   115,   927,     0,     0,
     150,     0,   115,     0,     0,     0,  1708,   115,   108,     0,
    1677,   318,  1026,     0,    13,  1636,     0,     0,     0,     0,
     108,     0,    36,   408,   197,     0,     0,   580,  1036,     0,
     410,     0,     0,     0,     0,     0,    36,     0,     0,   150,
       0,     0,     0,    48,   167,     0,  1316,    84,    85,  1304,
      86,    87,   121,    88,   168,    90,     0,    48,   121,     0,
     150,     0,     0,   121,     0,   331,   332,     0,     0,  1692,
       0,     0,     0,     0,     0,     0,  1305,     0,     0,   203,
     203,  1306,  1300,    60,    61,    62,   163,  1307,   407,  1308,
       0,     0,    13,     0,     0,     0,     0,   202,   202,     0,
     715,   218,    86,    87,     0,    88,   168,    90,     0,     0,
       0,   205,     0,   333,   208,   208,    86,    87,     0,    88,
     168,    90,     0,     0,     0,   218,  1309,  1310,    36,  1311,
     834,   835,     0,   751,     0,   103,   927,     0,   927,     0,
       0,   150,   867,   150,     0,   150,   115,   954,  1126,    48,
     408,     0,     0,     0,  1305,     0,     0,   867,  1449,  1306,
       0,    60,    61,    62,   163,  1307,   407,  1308,     0,     0,
     487,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,     0,     0,     0,   115,   115,   108,  1764,
       0,     0,     0,   115,   317,     0,     0,  1770,     0,     0,
    1409,     0,   121,  1773,  1309,  1310,  1774,  1311,    86,    87,
       0,    88,   168,    90,     0,     0,     0,     0,     0,     0,
       0,   472,   473,     0,     0,     0,     0,   619,   408,     0,
     115,   208,     0,     0,     0,     0,  1592,     0,   150,   205,
       0,     0,   121,   121,     0,   927,   619,     0,   205,   121,
       0,   108,     0,     0,   619,   205,   108,     0,     0,     0,
     108,     0,   205,   203,  1227,     0,     0,     0,     0,     0,
       0,     0,     0,   607,   351,     0,   208,   545,     0,     0,
     317,   202,     0,     0,     0,     0,   121,   474,   475,     0,
    1556,   208,   208,  1709,     0,     0,   115,  1563,     0,     0,
       0,     0,     0,   115,   317,     0,   317,     0,     0,  1258,
       0,     0,   317,     0,     0,     0,   150,     0,     0,     0,
       0,     0,     0,     0,   580,   954,     0,     0,   150,     0,
     218,     0,   218,     0,     0,   927,     0,     0,     0,   108,
     108,   108,     0,     0,   746,   108,     0,     0,     0,     0,
     108,     0,   121,     0,     0,     0,     0,     0,     0,   121,
       0,     0,     0,     0,   208,   487,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,    60,    61,
      62,    63,    64,   407,     0,   205,  1393,   218,     0,    70,
     451,   203,     0,     0,     0,     0,     0,   580,     0,     0,
     203,     0,     0,     0,     0,     0,     0,   203,     0,   202,
       0,     0,     0,     0,   203,     0,   472,   473,   202,   259,
       0,     0,     0,     0,     0,   202,     0,   453,     0,     0,
       0,     0,   202,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,   218,     0,   408,     0,   261,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,   545,
       0,     0,     0,     0,     0,     0,     0,     0,   218,    36,
       0,   218,     0,     0,     0,     0,     0,     0,     0,  1394,
     317,     0,   474,   475,   927,     0,     0,     0,     0,   108,
      48,     0,  1395,  1396,     0,     0,   150,  1651,   538,     0,
       0,     0,     0,     0,  1556,  1556,     0,     0,  1563,  1563,
     167,     0,     0,    84,  1397,   218,    86,    87,     0,    88,
    1398,    90,   351,     0,     0,   532,   533,     0,     0,   108,
     108,     0,     0,     0,     0,     0,   108,   203,   259,   820,
       0,     0,   205,   167,     0,     0,    84,   311,     0,    86,
      87,     0,    88,   168,    90,   202,     0,     0,     0,   150,
       0,     0,     0,     0,   150,     0,   261,   315,   150,     0,
       0,     0,     0,   108,     0,     0,     0,   316,     0,  1707,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,   259,     0,     0,     0,  1721,     0,     0,
     205,     0,     0,     0,     0,     0,     0,   218,   218,    48,
       0,   794,     0,     0,     0,     0,     0,  -373,     0,     0,
       0,   261,     0,     0,     0,    60,    61,    62,   163,   164,
     407,     0,     0,     0,     0,     0,   794,     0,     0,   108,
     205,     0,   205,    36,   532,   533,   108,   150,   150,   150,
       0,   259,     0,   150,     0,     0,     0,     0,   150,     0,
       0,     0,   167,     0,    48,    84,   311,     0,    86,    87,
     205,    88,   168,    90,     0,     0,     0,     0,     0,   261,
       0,     0,   218,   218,     0,     0,   315,     0,     0,     0,
       0,   218,   408,     0,   203,     0,   316,     0,     0,   532,
     533,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   202,     0,   917,   918,   205,   167,     0,     0,
      84,   311,    48,    86,    87,     0,    88,   168,    90,     0,
     972,   205,   205,     0,   919,     0,     0,     0,     0,     0,
       0,   315,   920,   921,   922,    36,     0,     0,     0,     0,
       0,   316,   203,     0,   923,   607,     0,   532,   533,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
     202,     0,     0,     0,     0,   167,     0,     0,    84,   311,
       0,    86,    87,     0,    88,   168,    90,     0,  1282,     0,
       0,     0,   203,     0,   203,     0,     0,   150,     0,   315,
       0,   924,     0,     0,     0,     0,     0,     0,     0,   316,
     202,     0,   202,     0,   925,     0,     0,     0,     0,     0,
       0,     0,   203,     0,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,     0,     0,     0,   150,   150,     0,
     202,   794,     0,   926,   150,     0,     0,     0,     0,    36,
       0,   205,   205,     0,   218,   218,   794,   794,   794,   794,
     794,   204,   204,     0,   794,   220,     0,     0,   203,     0,
      48,     0,     0,     0,     0,   218,     0,     0,     0,     0,
       0,   150,     0,   203,   203,     0,   202,   607,     0,     0,
       0,     0,  1394,     0,     0,     0,     0,     0,     0,   218,
       0,   202,   202,     0,     0,  1395,  1396,     0,     0,     0,
       0,     0,     0,     0,     0,   218,   218,     0,     0,     0,
       0,     0,     0,   167,     0,   218,    84,    85,     0,    86,
      87,   218,    88,  1398,    90,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   218,     0,     0,   150,     0,     0,
       0,     0,   794,     0,   150,   218,     0,   417,   418,   419,
       0,     0,     0,     0,     0,     0,     0,     0,   205,     0,
       0,     0,     0,   218,     0,     0,   420,   218,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,     0,   444,   203,   203,     0,     0,     0,   607,     0,
       0,     0,     0,   205,   445,     0,     0,     0,     0,     0,
       0,   202,   202,     0,     0,     0,     0,     0,   205,   205,
       0,     0,     0,     0,     0,   218,     0,     0,   218,     0,
     218,     0,     0,     0,     0,   204,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   794,     0,   218,     0,     0,
     794,   794,   794,   794,   794,   794,   794,   794,   794,   794,
     794,   794,   794,   794,   794,   794,   794,   794,   794,   794,
     794,   794,   794,   794,   794,   794,   794,   794,     0,     0,
       0,     0,     0,     0,     0,     0,   794,   259,     0,     0,
       0,   205,     0,     0,     0,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,     0,     0,
     203,     0,     0,     0,     0,   261,   218,     0,   218,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   202,     0,
       0,  1204,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,   218,   472,   473,   218,     0,
       0,     0,     0,     0,     0,   203,     0,     0,    48,     0,
       0,     0,     0,   204,     0,     0,     0,     0,   218,     0,
     203,   203,   204,   202,     0,     0,     0,     0,     0,   204,
     607,     0,     0,     0,     0,     0,   204,     0,   202,   202,
       0,   794,     0,   532,   533,     0,     0,   204,     0,     0,
     218,     0,     0,     0,   218,     0,     0,   794,     0,   794,
       0,   167,   474,   475,    84,   311,     0,    86,    87,     0,
      88,   168,    90,     0,     0,   794,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   315,     0,     0,     0,     0,
       0,     0,     0,   203,     0,   316,     0,     0,     0,   607,
       0,     0,     0,     0,     0,   218,   218,     0,   218,     0,
       0,   202,     0,     0,     0,   417,   418,   419,     0,   220,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   420,   320,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   204,
     444,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   445,     0,   487,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,     0,     0,     0,
       0,     0,   218,     0,   218,     0,     0,     0,     0,   794,
     218,     0,     0,   794,     0,   794,     0,     0,   794,    33,
      34,    35,     0,     0,     0,   797,   218,   218,     0,     0,
     218,   198,   417,   418,   419,   472,   473,   218,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     797,   420,     0,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,     0,   444,     0,   218,
      74,    75,    76,    77,    78,     0,     0,     0,     0,   445,
       0,   200,     0,     0,     0,   794,     0,    82,    83,     0,
       0,   474,   475,     0,   218,   218,   840,     0,     0,     0,
       0,    92,   218,     0,   218,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,   204,     0,     0,     0,
       0,     0,   417,   418,   419,     0,   218,     0,   218,     0,
       0,     0,     0,     0,   218,     0,     0,     0,     0,     0,
     320,   420,   320,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,     0,   444,     0,     0,
     794,   794,     0,     0,   204,     0,   794,     0,   218,   445,
       0,     0,     0,     0,   218,     0,   218,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   320,   417,   418,
     419,     0,     0,   872,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   204,     0,   204,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,   204,   797,     0,    60,    61,    62,
      63,    64,   407,     0,     0,   445,     0,     0,    70,   451,
     797,   797,   797,   797,   797,     0,     0,     0,   797,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   320,  1020,
       0,   320,     0,     0,     0,   218,     0,     0,     0,     0,
     204,     0,     0,     0,   452,     0,   453,     0,     0,     0,
       0,   446,   218,  1032,     0,   204,   204,     0,     0,   454,
       0,   455,     0,    28,   408,     0,     0,     0,     0,   218,
    1032,    33,    34,    35,    36,   794,   197,     0,     0,   204,
       0,     0,     0,   198,     0,     0,   794,     0,     0,     0,
       0,     0,   794,     0,     0,    48,   794,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   797,     0,     0,  1073,
       0,     0,     0,     0,     0,     0,   199,   218,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   911,
      73,   220,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,   794,    88,   168,    90,
       0,     0,     0,    92,     0,   218,     0,   320,   779,     0,
       0,   795,     0,     0,     0,   204,   204,    97,     0,     0,
       0,   218,   201,     0,     0,   559,     0,   103,     0,     0,
     218,     0,     0,     0,     0,     0,   795,     0,     0,     0,
       0,     0,     0,     0,     0,   218,     0,     0,     0,   797,
       0,   204,     0,     0,   797,   797,   797,   797,   797,   797,
     797,   797,   797,   797,   797,   797,   797,   797,   797,   797,
     797,   797,   797,   797,   797,   797,   797,   797,   797,   797,
     797,   797,   320,   320,     0,   417,   418,   419,     0,     0,
     797,   320,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   992,
     444,     0,   204,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   445,     0,     0,     0,   993,     0,     0,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,     0,   204,     0,     0,     0,     0,   204,     0,     0,
       0,     0,     0,     0,  1016,     0,     0,     0,     0,     0,
       0,     0,   204,   204,     0,   797,  -957,  -957,  -957,  -957,
    -957,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,   797,     0,   797,     0,     0,     0,     0,     0,     0,
       0,     0,   445,     0,     0,     0,     0,     0,     0,   797,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   795,     0,     0,     0,     0,     0,     0,     0,     0,
     990,   991,   992,     0,   320,   320,   795,   795,   795,   795,
     795,     0,  1303,     0,   795,   204,   915,   206,   206,   993,
       0,   224,   994,   995,   996,   997,   998,   999,  1000,  1001,
    1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  -957,  -957,  -957,  -957,  -957,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,     0,     0,
       0,     0,     0,     0,     0,   320,     0,     0,     0,     0,
    1016,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   320,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   320,     0,     0,     0,     0,     0,
       0,     0,   795,   797,   204,     0,     0,   797,     0,   797,
       0,     0,   797,   417,   418,   419,     0,     0,     0,     0,
       0,  1390,     0,   320,  1403,     0,     0,     0,     0,     0,
       0,     0,   420,     0,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     445,     0,     0,   204,     0,     0,     0,     0,  1155,     0,
       0,     0,     0,     0,     0,   320,     0,     0,   320,   797,
     779,     0,     0,     0,     0,     0,     0,     0,  1462,  1463,
       0,   206,     0,     0,     0,   795,     0,     0,  1403,     0,
     795,   795,   795,   795,   795,   795,   795,   795,   795,   795,
     795,   795,   795,   795,   795,   795,   795,   795,   795,   795,
     795,   795,   795,   795,   795,   795,   795,   795,     0,     0,
       0,     0,     0,     0,     0,     0,   795,   999,  1000,  1001,
    1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,     0,  1164,     0,     0,     0,     0,
       0,     0,     0,     0,   797,   797,   320,  1016,   320,     0,
     797,     0,  1607,   785,   786,     0,     0,     0,     0,   787,
    1403,   788,     0,     0,  1024,     0,     0,     0,     0,     0,
       0,     0,     0,   789,     0,   320,     0,     0,   320,     0,
       0,    33,    34,    35,    36,     0,     0,     0,     0,     0,
       0,     0,     0,   198,     0,     0,     0,     0,     0,   206,
       0,     0,     0,     0,     0,    48,     0,     0,   206,     0,
       0,     0,     0,     0,     0,   206,     0,     0,     0,     0,
       0,   795,   206,     0,     0,     0,     0,     0,     0,     0,
     320,     0,     0,   224,   320,     0,     0,   795,     0,   795,
     790,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   200,     0,   795,     0,     0,   167,    82,
      83,    84,   791,     0,    86,    87,     0,    88,   168,    90,
       0,     0,     0,    92,     0,     0,     0,     0,     0,     0,
       0,     0,   792,     0,     0,   320,   320,    97,     0,     0,
       0,     0,   793,     0,     0,     0,     0,     0,     0,   797,
       0,     0,     0,     0,     0,   224,     0,     0,     0,     0,
     797,     0,     0,     0,     0,     0,   797,     0,     0,     0,
     797,     0,     0,     0,     0,   417,   418,   419,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   420,   206,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,     0,   320,     0,   320,     0,     0,     0,     0,   795,
     797,     0,   445,   795,     0,   795,     0,     0,   795,  1718,
       0,     0,     0,     0,     0,     0,   320,     0,     0,     0,
       0,   798,     0,     0,     0,  1390,     0,   320,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     990,   991,   992,     0,     0,     0,   798,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   993,
       0,   799,   994,   995,   996,   997,   998,   999,  1000,  1001,
    1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,     0,   795,   813,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1016,     0,     0,
       0,     0,   320,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   417,   418,   419,     0,
       0,     0,   206,     0,   529,     0,   320,     0,   320,     0,
       0,     0,     0,     0,   320,   420,     0,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,  1146,   417,   418,   419,     0,     0,     0,
     795,   795,     0,   445,     0,     0,   795,     0,     0,     0,
     206,     0,     0,   420,   320,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     206,   445,   206,   420,     0,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
     206,   798,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   445,     0,     0,     0,     0,   798,   798,   798,   798,
     798,     0,     0,     0,   798,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   320,     0,     0,     0,     0,
       0,   955,     0,     0,     0,     0,   206,  1080,     0,     0,
       0,     0,   320,     0,     0,     0,   977,   978,   979,   980,
       0,   206,   206,     0,   987,     0,     0,     0,     0,  1652,
       0,     0,     0,     0,     0,   795,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   224,   795,     0,     0,     0,
       0,     0,   795,     0,     0,  1091,   795,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   798,     0,     0,     0,     0,   320,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   224,   444,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     445,     0,  1070,     0,   993,     0,   795,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,     0,
       0,   206,   206,     0,     0,     0,     0,     0,     0,     0,
     320,     0,  1016,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   320,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   798,     0,   224,     0,     0,
     798,   798,   798,   798,   798,   798,   798,   798,   798,   798,
     798,   798,   798,   798,   798,   798,   798,   798,   798,   798,
     798,   798,   798,   798,   798,   798,   798,   798,     0,     0,
       0,     0,     0,     0,     0,     0,   798,     0,     0,     0,
    1158,  1161,  1162,  1163,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,   417,   418,
     419,     0,     0,     0,     0,     0,  1192,     0,   206,     0,
       0,     0,     0,     0,     0,     0,     0,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   206,     0,   445,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   206,   206,
       0,   798,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   798,     0,   798,
     250,   251,     0,   252,   253,     0,     0,   254,   255,   256,
     257,     0,     0,     0,     0,   798,     0,     0,     0,     0,
       0,  1270,     0,     0,   258,     0,     0,     0,     0,     0,
       0,   490,     0,     0,     0,     0,     0,  1285,     0,  1286,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   206,   260,     0,     0,  1296,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   262,   263,   264,   265,
     266,   267,   268,     0,     0,     0,    36,     0,   197,     0,
       0,     0,     0,     0,     0,     0,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,    48,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,     0,     0,     0,   687,   304,   305,   306,     0,     0,
       0,   307,   542,   543,     0,     0,     0,     0,     0,   798,
     224,     0,     0,   798,     0,   798,     0,     0,   798,     0,
     544,     0,     0,     0,     0,     0,    86,    87,     0,    88,
     168,    90,   312,     0,   313,     0,     0,   314,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1375,
       0,     0,     0,  1377,     0,  1378,     0,   688,  1379,   103,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,   224,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1016,     0,     0,   798,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   379,    12,     0,     0,     0,     0,     0,     0,     0,
     699,     0,     0,     0,     0,  1458,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
     798,   798,    41,     0,     0,     0,   798,     0,     0,     0,
       0,     0,     0,     0,    48,  1612,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   163,   164,   165,     0,     0,    67,    68,
    1601,  1602,     0,     0,     0,     0,  1606,     0,   166,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,   417,   418,   419,     0,    97,    98,    99,     0,
       0,   100,     0,     0,     0,     0,   103,   104,     0,   105,
     106,   420,     0,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,     0,   444,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   445,
       0,     0,     0,     0,     0,   798,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   798,     0,     0,     0,
       0,     0,   798,     0,     0,     0,   798,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,  1661,   444,     0,     0,  1693,
       0,     0,     0,     0,     0,     0,  1671,     0,   445,     0,
       0,     0,  1676,     0,     0,     0,  1678,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,   798,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,  1115,     0,    16,  1711,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
      54,    55,    56,     0,    57,    58,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,    69,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,    81,    82,    83,    84,    85,     0,
      86,    87,     0,    88,    89,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,    95,
       0,    96,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,  1040,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,    54,    55,    56,     0,    57,    58,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,    69,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,    81,    82,
      83,    84,    85,     0,    86,    87,     0,    88,    89,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,    95,     0,    96,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,  1205,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,    81,    82,    83,    84,    85,     0,    86,    87,
       0,    88,    89,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,    95,     0,    96,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,     0,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,   626,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   101,   102,  1019,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,     0,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,  1054,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,     0,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,  1121,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,  1123,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,     0,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,  1271,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,     0,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   101,   102,  1381,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,     0,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,  1603,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,  1648,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,     0,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,     0,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
    1682,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,     0,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,  1683,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,  1686,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   101,   102,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,     0,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,  1702,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,     0,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,  1758,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,     0,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
    1765,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,     0,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   515,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,   163,   164,    65,     0,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   101,   102,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,   764,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
     163,   164,    65,     0,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,   957,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,   163,   164,    65,     0,    66,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,  1453,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,   163,   164,
      65,     0,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,  1596,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,   163,   164,    65,     0,    66,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,   163,   164,    65,     0,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   101,   102,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     163,   164,   165,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   166,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   169,     0,
     325,     0,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,   641,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   445,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   163,   164,   165,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   166,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
       0,   642,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   169,     0,     0,     0,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   163,   164,
     165,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   166,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   169,     0,     0,   759,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,     0,     0,  1067,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1016,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   163,   164,   165,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   166,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,     0,  1068,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     169,     0,     0,     0,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   163,   164,   165,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   166,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   417,   418,   419,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   420,     0,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   445,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,   180,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     163,   164,   165,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   166,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,  1426,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   169,     0,
       0,     0,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,     0,     0,     0,
       0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
    1016,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   163,   164,   165,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   166,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   169,     0,   417,   418,   419,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   420,     0,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   445,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   163,   164,
     165,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   166,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,  1427,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   169,     0,   245,   418,
     419,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,   445,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   163,   164,   165,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   166,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     169,     0,   248,     0,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   163,   164,   165,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   166,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   417,   418,   419,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   420,  1275,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   445,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     163,   164,   165,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   166,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,  1276,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   169,   513,
       0,     0,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   654,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   163,   164,   165,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   166,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   169,     0,     0,     0,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,     0,     0,   699,     0,     0,     0,     0,
       0,     0,     0,   445,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   163,   164,
     165,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   166,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   169,     0,     0,     0,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  -957,  -957,  -957,
    -957,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,     0,   739,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   163,   164,   165,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   166,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     169,     0,     0,     0,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  -957,  -957,  -957,  -957,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,     0,     0,
       0,     0,     0,   741,     0,     0,     0,     0,     0,     0,
       0,  1016,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   163,   164,   165,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   166,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   169,     0,     0,     0,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1112,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     163,   164,   165,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   166,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   169,     0,
     417,   418,   419,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   420,
       0,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   445,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   163,   164,   165,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   166,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,   531,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   169,     0,   417,   418,   419,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   420,     0,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   445,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,   588,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   163,   164,
     165,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   166,    73,     0,    74,    75,    76,    77,
      78,   250,   251,     0,   252,   253,     0,    80,   254,   255,
     256,   257,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,     0,   258,     0,    92,     0,     0,
      93,     0,     0,   548,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   169,     0,     0,     0,
       0,   103,   104,   260,   105,   106,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   262,   263,   264,
     265,   266,   267,   268,     0,     0,     0,    36,     0,   197,
       0,     0,     0,     0,     0,     0,     0,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,    48,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,     0,     0,     0,   303,   304,   305,   306,     0,
       0,     0,   307,   542,   543,     0,     0,     0,     0,     0,
     250,   251,     0,   252,   253,     0,     0,   254,   255,   256,
     257,   544,     0,     0,     0,     0,     0,    86,    87,     0,
      88,   168,    90,   312,   258,   313,   259,     0,   314,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   688,     0,
     103,     0,   260,     0,   261,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   262,   263,   264,   265,
     266,   267,   268,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,    48,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,     0,     0,     0,     0,   304,   305,   306,     0,     0,
       0,   307,   308,   309,     0,     0,     0,     0,     0,   250,
     251,     0,   252,   253,     0,     0,   254,   255,   256,   257,
     310,     0,     0,    84,   311,     0,    86,    87,     0,    88,
     168,    90,   312,   258,   313,   259,     0,   314,     0,     0,
       0,     0,     0,     0,   315,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   316,     0,     0,     0,  1576,     0,
       0,   260,     0,   261,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   262,   263,   264,   265,   266,
     267,   268,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,    48,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
       0,     0,     0,     0,   304,   305,   306,     0,     0,     0,
     307,   308,   309,     0,     0,     0,     0,     0,   250,   251,
       0,   252,   253,     0,     0,   254,   255,   256,   257,   310,
       0,     0,    84,   311,     0,    86,    87,     0,    88,   168,
      90,   312,   258,   313,   259,     0,   314,     0,     0,     0,
       0,     0,     0,   315,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   316,     0,     0,     0,  1644,     0,     0,
     260,     0,   261,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   262,   263,   264,   265,   266,   267,
     268,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    48,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,     0,
       0,     0,   303,   304,   305,   306,     0,     0,     0,   307,
     308,   309,     0,     0,     0,     0,     0,   250,   251,     0,
     252,   253,     0,     0,   254,   255,   256,   257,   310,     0,
       0,    84,   311,     0,    86,    87,     0,    88,   168,    90,
     312,   258,   313,   259,     0,   314,     0,     0,     0,     0,
       0,     0,   315,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   316,     0,     0,     0,     0,     0,     0,   260,
       0,   261,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   262,   263,   264,   265,   266,   267,   268,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,    48,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,     0,     0,
       0,     0,   304,   305,   306,     0,     0,     0,   307,   308,
     309,     0,     0,     0,     0,     0,   250,   251,     0,   252,
     253,     0,     0,   254,   255,   256,   257,   310,     0,     0,
      84,   311,     0,    86,    87,     0,    88,   168,    90,   312,
     258,   313,   259,     0,   314,     0,     0,     0,     0,     0,
       0,   315,  1385,     0,     0,     0,     0,     0,     0,     0,
       0,   316,     0,     0,     0,     0,     0,     0,   260,     0,
     261,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   262,   263,   264,   265,   266,   267,   268,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,    48,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,     0,     0,     0,
       0,   304,   305,   306,     0,     0,     0,   307,   308,   309,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   310,     0,     0,    84,
     311,     0,    86,    87,     0,    88,   168,    90,   312,     0,
     313,     0,     0,   314,  1477,  1478,  1479,  1480,  1481,     0,
     315,  1482,  1483,  1484,  1485,     0,     0,     0,     0,     0,
     316,     0,     0,     0,     0,     0,     0,     0,  1486,  1487,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,  1488,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   445,     0,     0,     0,
    1489,  1490,  1491,  1492,  1493,  1494,  1495,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1496,  1497,  1498,  1499,  1500,  1501,  1502,  1503,  1504,  1505,
    1506,    48,  1507,  1508,  1509,  1510,  1511,  1512,  1513,  1514,
    1515,  1516,  1517,  1518,  1519,  1520,  1521,  1522,  1523,  1524,
    1525,  1526,  1527,  1528,  1529,  1530,  1531,  1532,  1533,  1534,
    1535,  1536,     0,     0,     0,  1537,  1538,     0,  1539,  1540,
    1541,  1542,  1543,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1544,  1545,  1546,     0,     0,     0,
      86,    87,     0,    88,   168,    90,  1547,     0,  1548,  1549,
       0,  1550,   417,   418,   419,     0,     0,     0,  1551,  1552,
       0,  1553,     0,  1554,  1555,     0,     0,     0,     0,     0,
       0,   420,     0,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,     0,   444,   417,   418,
     419,     0,     0,     0,     0,     0,     0,     0,     0,   445,
       0,     0,     0,     0,     0,     0,     0,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,   417,   418,   419,     0,     0,     0,
       0,     0,     0,     0,     0,   445,     0,     0,     0,     0,
       0,     0,     0,   420,     0,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   250,
     251,   445,   252,   253,     0,     0,   254,   255,   256,   257,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   258,     0,     0,     0,     0,     0,     0,
       0,   552,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   260,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   262,   263,   264,   265,   266,
     267,   268,     0,     0,     0,    36,   731,     0,     0,     0,
       0,     0,     0,     0,     0,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,    48,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
       0,     0,   756,   303,   304,   305,   306,     0,     0,     0,
     307,   542,   543,   250,   251,     0,   252,   253,     0,     0,
     254,   255,   256,   257,     0,     0,     0,     0,     0,   544,
       0,     0,     0,     0,     0,    86,    87,   258,    88,   168,
      90,   312,     0,   313,     0,     0,   314,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   260,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   262,
     263,   264,   265,   266,   267,   268,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
      48,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,     0,     0,     0,  1156,   304,   305,
     306,     0,     0,     0,   307,   542,   543,   250,   251,     0,
     252,   253,     0,     0,   254,   255,   256,   257,     0,     0,
       0,     0,     0,   544,     0,     0,     0,     0,     0,    86,
      87,   258,    88,   168,    90,   312,     0,   313,     0,     0,
     314,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   260,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   262,   263,   264,   265,   266,   267,   268,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,    48,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,     0,     0,
       0,     0,   304,   305,   306,     0,     0,     0,   307,   542,
     543,   951,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   544,     0,     0,
       0,     0,     0,    86,    87,     0,    88,   168,    90,   312,
       0,   313,     0,    28,   314,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,     0,   197,     0,     0,     0,
       0,     0,     0,   198,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   199,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   952,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
     785,   786,     0,    92,     0,     0,   787,     0,   788,     0,
       0,     0,     0,     0,     0,     0,     0,    97,     0,     0,
     789,     0,   201,     0,     0,     0,     0,   103,    33,    34,
      35,    36,     0,     0,     0,     0,     0,   417,   418,   419,
     198,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,   420,     0,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,     0,   444,     0,     0,     0,     0,   790,     0,    74,
      75,    76,    77,    78,   445,     0,     0,     0,     0,     0,
     200,     0,     0,     0,     0,   167,    82,    83,    84,   791,
       0,    86,    87,    28,    88,   168,    90,     0,     0,     0,
      92,    33,    34,    35,    36,     0,   197,     0,     0,   792,
       0,     0,     0,   198,    97,     0,     0,     0,     0,   793,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     499,     0,     0,     0,     0,     0,   199,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   579,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
      28,     0,   906,    92,     0,     0,     0,     0,    33,    34,
      35,    36,     0,   197,     0,     0,     0,    97,     0,     0,
     198,     0,   201,     0,     0,     0,     0,   103,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   199,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     200,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,    28,    88,   168,    90,     0,     0,     0,
      92,    33,    34,    35,    36,     0,   197,     0,     0,     0,
       0,     0,     0,   198,    97,     0,     0,     0,     0,   201,
       0,     0,     0,     0,   103,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   199,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1035,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,    28,    88,   168,    90,
       0,     0,     0,    92,    33,    34,    35,    36,     0,   197,
       0,     0,     0,     0,     0,     0,   198,    97,     0,     0,
       0,     0,   201,     0,     0,     0,     0,   103,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   199,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,     0,     0,     0,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   201,     0,     0,     0,   420,
     103,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,   417,   418,   419,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
       0,     0,     0,     0,     0,   420,     0,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   445,     0,   417,   418,   419,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   892,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,   990,   991,   992,     0,     0,     0,     0,     0,     0,
       0,     0,   445,     0,     0,     0,     0,     0,     0,   936,
     993,     0,     0,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1016,     0,
     990,   991,   992,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1234,   993,
       0,     0,   994,   995,   996,   997,   998,   999,  1000,  1001,
    1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,   990,   991,   992,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1016,     0,     0,
       0,     0,     0,   993,  1292,     0,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,     0,     0,
       0,    33,    34,    35,    36,     0,   197,     0,     0,     0,
       0,  1016,     0,   198,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,  1374,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   215,     0,     0,     0,
       0,     0,   216,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,  1460,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
       0,     0,     0,    92,    33,    34,    35,    36,     0,   197,
       0,     0,     0,     0,     0,     0,   602,    97,     0,     0,
       0,     0,   217,     0,     0,     0,     0,   103,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   199,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,    33,    34,    35,
      36,     0,   197,     0,     0,     0,     0,     0,     0,   198,
      97,     0,     0,     0,     0,   603,     0,     0,     0,     0,
     103,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   215,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,     0,     0,     0,    92,
       0,     0,     0,   417,   418,   419,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   217,     0,
       0,   768,   420,   103,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     445,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   417,   418,   419,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   769,   420,   889,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
       0,   445,     0,     0,     0,     0,     0,     0,     0,   420,
       0,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,   990,   991,   992,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
       0,     0,     0,     0,     0,   993,  1297,     0,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
     990,   991,   992,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1016,     0,     0,     0,     0,     0,   993,
       0,     0,   994,   995,   996,   997,   998,   999,  1000,  1001,
    1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,   991,   992,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1016,     0,     0,
       0,     0,   993,     0,     0,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,   419,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1016,     0,     0,     0,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   445,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1016,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   445,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1016
};

static const yytype_int16 yycheck[] =
{
       5,     6,   121,     8,     9,    10,    11,    12,   171,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   146,    54,    28,    29,   100,   644,    54,    91,     4,
       4,     4,    95,    96,   372,    55,   372,    42,     4,    32,
     372,  1057,   615,   484,   503,    50,   151,    52,   614,   218,
      55,    44,    57,   172,   854,    54,    49,   120,   213,   865,
      30,   100,   444,  1053,   146,    30,    30,    30,   515,  1044,
     480,   481,   100,   774,    79,    42,   476,   885,   100,     9,
     595,     9,     9,     9,   767,     9,     9,    14,   476,     9,
      58,     9,   733,   901,     4,   100,    14,     9,   950,   509,
     696,     9,   549,     9,    34,     9,    14,     9,    47,     9,
       4,   511,    79,    81,   229,    30,    84,     9,     9,     9,
       9,     9,     9,   511,     9,   230,     9,    30,     9,   937,
     169,     9,    68,     9,     9,     9,    47,     9,     9,     9,
       9,   169,    87,     9,   743,    87,    81,   169,  1590,    47,
      47,    68,    36,   131,   132,     9,   112,   152,   158,     0,
     152,    99,   201,    36,   169,    99,     4,    81,    81,   616,
     508,   176,   152,   201,   131,   132,    36,   173,   217,   201,
     187,   173,   104,   105,   106,    14,   986,    55,    47,   217,
     190,   187,   187,   173,     8,   187,   201,    81,   190,    67,
      36,    30,   103,  1645,   149,    36,    36,   149,    81,   448,
      68,    68,   217,   191,   170,    68,   152,   152,   156,    68,
      49,    81,   156,    68,    68,    68,   231,   121,   187,   234,
     187,    68,   259,    68,   261,    68,   241,   242,   477,  1229,
      68,    68,   357,   482,    68,    81,   409,    68,   174,   188,
      81,    81,   166,    68,   190,   173,  1231,    68,   188,   960,
     188,   962,   189,  1238,   188,  1240,   189,   190,   188,   235,
     187,   189,    52,   239,   185,   190,  1128,   189,   172,   192,
    1088,   189,   166,   189,  1259,   189,   189,   189,   188,   316,
     188,   188,   324,   174,   131,   132,    81,   189,   189,   189,
     189,   189,   189,   174,   189,   904,   189,  1107,   146,   882,
     349,   189,   759,   189,   189,   189,   188,   764,   188,   188,
     174,   349,   188,   486,   152,   324,   185,   349,   843,   188,
     166,   152,   190,   190,   187,   166,   166,   190,   493,   458,
     403,   190,   187,   348,   349,   190,   190,   190,    68,  1339,
     355,  1341,   187,   190,   187,   360,   190,   190,    81,   187,
     187,   188,   190,   331,   332,   333,   190,    81,   400,   190,
     397,    68,   173,   400,   379,  1350,   187,   157,    29,   190,
     119,   348,   387,    68,   459,   187,   187,    29,   127,   830,
     453,   454,   455,   456,   399,   187,   364,    48,   103,   104,
      51,   400,    81,   103,   104,   190,    48,   192,    52,    51,
      81,   187,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   459,
     445,  1124,   447,   448,   449,   187,  1042,  1305,   157,  1439,
     605,   450,   166,     4,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,  1108,   378,   192,
     190,   871,   477,   478,   444,   480,   481,   482,   483,   444,
     444,   444,   487,   871,   187,   490,   191,   166,   898,  1289,
    1056,   191,   386,   190,   499,   166,   501,   517,   497,  1072,
      51,   187,  1075,    54,   509,   190,   621,   632,   623,   536,
     957,    29,   517,   157,   519,    68,   854,   187,   854,   120,
      71,   690,   854,   173,   906,   173,   127,    68,   172,   131,
     132,   686,    81,  1391,   152,   698,    30,   187,    89,   187,
      91,    48,    49,   508,    95,    96,   187,    81,   386,   450,
     632,     4,   721,    87,   559,   173,   522,  1415,  1358,  1417,
     158,    79,   173,   457,   603,    73,    74,    81,   187,   120,
      88,   187,   190,   171,   187,   476,   187,   156,   131,   132,
     156,   187,   100,    68,   611,   612,    36,   650,   556,    81,
     131,   132,   190,   620,    47,    87,   497,   189,   603,   152,
     149,   150,   151,  1593,   189,    81,  1247,   508,  1249,    81,
     511,    87,   189,   189,   190,    79,   150,   151,   102,   103,
     104,    81,   737,   738,   189,   794,   781,    87,   189,   744,
     745,  1090,   801,   184,   187,   153,   100,   642,   156,   744,
     190,   159,   160,  1239,   162,   163,   164,    81,   986,   654,
     986,    79,  1218,    87,   986,    68,   109,   189,   150,   151,
    1233,   114,   196,   116,   117,   118,   119,   120,   121,   122,
     184,  1081,   100,   149,   150,   151,   190,   152,   150,   151,
      73,    74,  1092,   688,   235,   187,  1127,   187,   239,   149,
     150,   151,   243,   157,    68,   159,   160,   161,   162,   163,
     164,    99,   100,   195,   152,   615,   159,   160,    79,   162,
     187,   716,   156,  1729,    46,  1356,   150,   151,    79,   102,
     103,   104,   189,   187,  1320,   116,   117,   118,  1744,   100,
     183,   159,   160,  1723,   162,   163,   164,    67,   191,   100,
     173,    81,   152,    81,  1191,   187,   751,    87,  1738,    87,
      26,    27,    51,    52,    53,   187,   783,   784,   194,  1200,
       9,     4,   767,    51,    52,    53,   152,    55,    67,  1107,
     763,  1107,   152,   324,     8,  1107,   931,   189,  1351,    67,
     129,   130,   153,    26,    27,   156,   189,   190,   159,   160,
     187,   162,   163,   164,   632,   156,  1392,   187,   159,   160,
     152,   162,   163,   164,    14,  1446,   772,   189,   190,   152,
     150,   151,   150,   151,  1261,   331,   332,   844,  1619,  1620,
     191,   976,  1615,  1616,   189,  1235,   127,   127,   983,   190,
    1277,   783,   784,   860,   839,   386,    48,    49,    50,    51,
      52,    53,   905,    55,    14,   396,   873,   188,   853,   400,
     173,    14,   403,    99,   188,    67,   193,   100,   857,   188,
     188,   188,   187,   757,   108,   187,   187,   842,   842,   842,
     108,     9,   149,   188,   879,   902,   842,   188,   116,   117,
     118,   119,   120,   121,   889,   188,   188,   892,    91,   894,
       9,  1697,   189,   898,  1063,   173,    79,    14,   187,   450,
     451,   452,   453,   454,   455,   456,     9,   187,    81,   803,
    1716,   129,    68,   188,  1361,   188,   188,   100,  1724,   757,
     189,   187,    30,  1370,   188,   476,   169,  1337,    79,   130,
     172,   936,   842,  1380,   210,   152,   906,   964,   133,     9,
     967,   906,   906,   906,   152,   183,   497,  1102,   842,   100,
     943,  1289,   188,  1289,  1595,   865,   857,  1289,   201,    14,
     511,   185,  1037,     9,     9,   803,   188,   210,   869,   863,
     871,   522,   882,   156,   217,   174,   159,   160,   944,   162,
     163,   164,     9,    14,   129,   194,   985,   194,     9,   988,
     541,   191,   235,   152,    14,   188,   239,   194,  1153,   188,
     194,   187,    99,   188,   842,  1160,  1453,   558,   159,   160,
     189,   162,   163,   164,   189,    88,   152,  1037,   133,     9,
    1358,  1639,  1358,   188,   152,   863,  1358,   187,  1055,   187,
    1057,   152,  1037,   584,   585,   190,   187,   938,   190,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   946,    14,   948,   189,    14,  1084,   190,     4,
    1087,    79,   338,  1068,   194,   190,  1041,  1041,  1041,   189,
      14,   347,   188,   349,    30,  1041,  1081,   187,   354,   185,
     187,    30,   100,    14,    14,   361,   187,  1092,  1093,   187,
      65,    66,   187,    50,   187,   338,  1714,     9,   188,   650,
     189,  1256,    47,   189,   347,    79,   349,    81,   946,   187,
     948,   354,  1139,     9,   133,    14,  1143,   133,   361,  1124,
     188,    67,   194,  1116,   187,     9,   100,    81,     9,  1134,
     133,  1041,    48,    49,    50,    51,    52,    53,  1585,   157,
    1587,   159,   160,   386,   162,   163,   164,  1041,   187,  1596,
      14,    67,   189,  1119,    81,     4,   131,   132,   188,   187,
     133,   190,  1072,   187,   109,  1075,   188,  1194,  1195,   114,
       9,   116,   117,   118,   119,   120,   121,   122,    79,   190,
     190,   189,   733,   157,   735,   159,   160,   194,   162,   163,
     164,    88,   190,    30,   149,  1642,  1321,  1316,    47,   100,
      75,   189,   188,  1041,   189,  1210,   757,   174,   484,  1214,
     133,  1216,    30,   188,   159,   160,   190,   162,   192,  1224,
     771,   772,   188,     4,   125,   188,   133,     9,   188,  1234,
    1235,   116,   117,   118,   119,   120,   121,  1131,   183,   191,
       9,   484,    14,   191,   133,   188,   191,   358,   190,    81,
    1413,   362,   803,   187,  1281,   188,  1283,   808,   159,   160,
     109,   162,   163,   164,    79,   114,    47,   116,   117,   118,
     119,   120,   121,   122,   188,   188,   827,   187,   389,   522,
     391,   392,   393,   394,   189,   100,   187,   190,  1198,  1316,
     188,   842,     9,  1131,   188,  1742,   188,  1207,   183,    30,
     190,   189,  1749,   189,   189,   188,   857,   189,   188,   109,
     159,   160,   863,   162,   161,   189,   157,    14,   869,    81,
     871,   114,  1385,  1233,   190,   188,   188,   133,   109,  1334,
     188,   133,  1337,   114,   183,   116,   117,   118,   119,   120,
     121,   122,   191,    14,   159,   160,   173,   162,   163,   164,
     190,   189,    81,    14,   905,    14,    81,     4,   188,   187,
    1198,    76,    77,    78,    79,   641,   917,   918,   919,  1207,
     190,   188,   187,   133,    14,   189,   189,    14,   159,   160,
     189,   162,    14,   190,     9,   100,   191,   938,    57,  1388,
      81,   187,    81,   944,   173,   946,     9,   948,   641,   189,
      47,  1367,   183,   112,    99,  1299,   152,    99,   164,    14,
     191,    34,  1575,   187,   189,   188,   187,   968,   170,   174,
     167,  1315,   188,   699,    81,  1430,     9,    81,   189,   188,
    1340,   188,   190,    14,   985,    81,  1346,   988,  1348,    14,
      81,  1351,    14,    81,   159,   160,  1473,   162,   163,   164,
    1360,    14,    81,  1705,   556,  1610,   699,   456,   451,   453,
     899,  1299,   109,   739,   845,   741,  1017,   114,  1362,   116,
     117,   118,   119,   120,   121,   122,  1720,  1371,  1125,  1274,
    1452,  1716,   561,  1321,  1442,  1451,  1452,    26,    27,  1475,
    1041,  1559,  1311,   769,  1393,  1748,   739,  1571,   741,  1736,
    1307,  1438,  1340,  1045,   984,  1104,  1103,   918,  1346,   933,
    1348,   869,   159,   160,   757,   162,   116,   117,   118,   119,
     120,   121,  1360,   400,  1362,   783,   769,  1421,  1300,   772,
      79,     4,   355,  1371,  1444,  1673,   183,  1028,  1570,   815,
     969,    -1,    -1,  1017,   191,    -1,    -1,  1574,    -1,    -1,
      -1,   100,    -1,  1104,   830,   831,  1450,  1108,    -1,    -1,
     803,    -1,  1456,    -1,    -1,    -1,  1691,  1461,  1119,    -1,
    1633,  1570,   815,    -1,    47,  1580,    -1,    -1,    -1,    -1,
    1131,    -1,    79,   183,    81,    -1,    -1,   830,   831,    -1,
    1709,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,   842,
      -1,    -1,    -1,   100,   153,    -1,  1444,   156,   157,     4,
     159,   160,  1450,   162,   163,   164,    -1,   100,  1456,    -1,
     863,    -1,    -1,  1461,    -1,   108,   109,    -1,    -1,  1656,
      -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,    26,
      27,   114,  1193,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    47,    -1,    -1,    -1,    -1,    26,    27,    -1,
     157,    30,   159,   160,    -1,   162,   163,   164,    -1,    -1,
      -1,   210,    -1,   156,   950,   951,   159,   160,    -1,   162,
     163,   164,    -1,    -1,    -1,    54,   159,   160,    79,   162,
      81,    82,    -1,   190,    -1,   192,  1247,    -1,  1249,    -1,
      -1,   944,  1729,   946,    -1,   948,  1600,   950,   951,   100,
     183,    -1,    -1,    -1,   109,    -1,    -1,  1744,   191,   114,
      -1,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,  1640,  1641,  1299,  1754,
      -1,    -1,    -1,  1647,  1305,    -1,    -1,  1762,    -1,    -1,
    1311,    -1,  1600,  1768,   159,   160,  1771,   162,   159,   160,
      -1,   162,   163,   164,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    66,    -1,    -1,    -1,    -1,  1697,   183,    -1,
    1684,  1067,    -1,    -1,    -1,    -1,   191,    -1,  1041,   338,
      -1,    -1,  1640,  1641,    -1,  1356,  1716,    -1,   347,  1647,
      -1,  1362,    -1,    -1,  1724,   354,  1367,    -1,    -1,    -1,
    1371,    -1,   361,   210,  1067,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   372,  1385,    -1,  1112,  1388,    -1,    -1,
    1391,   210,    -1,    -1,    -1,    -1,  1684,   131,   132,    -1,
    1401,  1127,  1128,  1691,    -1,    -1,  1750,  1408,    -1,    -1,
      -1,    -1,    -1,  1757,  1415,    -1,  1417,    -1,    -1,  1112,
      -1,    -1,  1423,    -1,    -1,    -1,  1119,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1127,  1128,    -1,    -1,  1131,    -1,
     259,    -1,   261,    -1,    -1,  1446,    -1,    -1,    -1,  1450,
    1451,  1452,    -1,    -1,   188,  1456,    -1,    -1,    -1,    -1,
    1461,    -1,  1750,    -1,    -1,    -1,    -1,    -1,    -1,  1757,
      -1,    -1,    -1,    -1,  1200,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   116,   117,
     118,   119,   120,   121,    -1,   484,    29,   316,    -1,   127,
     128,   338,    -1,    -1,    -1,    -1,    -1,  1200,    -1,    -1,
     347,    -1,    -1,    -1,    -1,    -1,    -1,   354,    -1,   338,
      -1,    -1,    -1,    -1,   361,    -1,    65,    66,   347,    29,
      -1,    -1,    -1,    -1,    -1,   354,    -1,   165,    -1,    -1,
      -1,    -1,   361,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,   372,    -1,   183,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,  1570,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   397,    79,
      -1,   400,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,
    1591,    -1,   131,   132,  1595,    -1,    -1,    -1,    -1,  1600,
     100,    -1,   135,   136,    -1,    -1,  1299,  1608,   108,    -1,
      -1,    -1,    -1,    -1,  1615,  1616,    -1,    -1,  1619,  1620,
     153,    -1,    -1,   156,   157,   444,   159,   160,    -1,   162,
     163,   164,  1633,    -1,    -1,   135,   136,    -1,    -1,  1640,
    1641,    -1,    -1,    -1,    -1,    -1,  1647,   484,    29,   188,
      -1,    -1,   641,   153,    -1,    -1,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,   484,    -1,    -1,    -1,  1362,
      -1,    -1,    -1,    -1,  1367,    -1,    57,   177,  1371,    -1,
      -1,    -1,    -1,  1684,    -1,    -1,    -1,   187,    -1,  1690,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    29,    -1,    -1,    -1,  1708,    -1,    -1,
     699,    -1,    -1,    -1,    -1,    -1,    -1,   536,   537,   100,
      -1,   540,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    57,    -1,    -1,    -1,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,    -1,    -1,   565,    -1,    -1,  1750,
     739,    -1,   741,    79,   135,   136,  1757,  1450,  1451,  1452,
      -1,    29,    -1,  1456,    -1,    -1,    -1,    -1,  1461,    -1,
      -1,    -1,   153,    -1,   100,   156,   157,    -1,   159,   160,
     769,   162,   163,   164,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,   611,   612,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   620,   183,    -1,   641,    -1,   187,    -1,    -1,   135,
     136,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   641,    -1,    48,    49,   815,   153,    -1,    -1,
     156,   157,   100,   159,   160,    -1,   162,   163,   164,    -1,
     166,   830,   831,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,   177,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,   187,   699,    -1,    88,   854,    -1,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
     699,    -1,    -1,    -1,    -1,   153,    -1,    -1,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,    -1,   166,    -1,
      -1,    -1,   739,    -1,   741,    -1,    -1,  1600,    -1,   177,
      -1,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,
     739,    -1,   741,    -1,   148,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   769,    -1,    -1,   159,   160,    -1,   162,   163,
     164,    -1,    -1,    -1,    -1,    -1,    -1,  1640,  1641,    -1,
     769,   770,    -1,   177,  1647,    -1,    -1,    -1,    -1,    79,
      -1,   950,   951,    -1,   783,   784,   785,   786,   787,   788,
     789,    26,    27,    -1,   793,    30,    -1,    -1,   815,    -1,
     100,    -1,    -1,    -1,    -1,   804,    -1,    -1,    -1,    -1,
      -1,  1684,    -1,   830,   831,    -1,   815,   986,    -1,    -1,
      -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,   828,
      -1,   830,   831,    -1,    -1,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   844,   845,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,   854,   156,   157,    -1,   159,
     160,   860,   162,   163,   164,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   873,    -1,    -1,  1750,    -1,    -1,
      -1,    -1,   881,    -1,  1757,   884,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1067,    -1,
      -1,    -1,    -1,   902,    -1,    -1,    29,   906,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,   950,   951,    -1,    -1,    -1,  1107,    -1,
      -1,    -1,    -1,  1112,    67,    -1,    -1,    -1,    -1,    -1,
      -1,   950,   951,    -1,    -1,    -1,    -1,    -1,  1127,  1128,
      -1,    -1,    -1,    -1,    -1,   964,    -1,    -1,   967,    -1,
     969,    -1,    -1,    -1,    -1,   210,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   984,    -1,   986,    -1,    -1,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1025,    29,    -1,    -1,
      -1,  1200,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
    1067,    -1,    -1,    -1,    -1,    57,  1055,    -1,  1057,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1067,    -1,
      -1,   194,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1084,    65,    66,  1087,    -1,
      -1,    -1,    -1,    -1,    -1,  1112,    -1,    -1,   100,    -1,
      -1,    -1,    -1,   338,    -1,    -1,    -1,    -1,  1107,    -1,
    1127,  1128,   347,  1112,    -1,    -1,    -1,    -1,    -1,   354,
    1289,    -1,    -1,    -1,    -1,    -1,   361,    -1,  1127,  1128,
      -1,  1130,    -1,   135,   136,    -1,    -1,   372,    -1,    -1,
    1139,    -1,    -1,    -1,  1143,    -1,    -1,  1146,    -1,  1148,
      -1,   153,   131,   132,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,    -1,    -1,  1164,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1200,    -1,   187,    -1,    -1,    -1,  1358,
      -1,    -1,    -1,    -1,    -1,  1194,  1195,    -1,  1197,    -1,
      -1,  1200,    -1,    -1,    -1,    10,    11,    12,    -1,   444,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    54,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,   484,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,  1281,    -1,  1283,    -1,    -1,    -1,    -1,  1288,
    1289,    -1,    -1,  1292,    -1,  1294,    -1,    -1,  1297,    76,
      77,    78,    -1,    -1,    -1,   540,  1305,  1306,    -1,    -1,
    1309,    88,    10,    11,    12,    65,    66,  1316,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     565,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,  1358,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    67,
      -1,   148,    -1,    -1,    -1,  1374,    -1,   154,   155,    -1,
      -1,   131,   132,    -1,  1383,  1384,   191,    -1,    -1,    -1,
      -1,   168,  1391,    -1,  1393,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   641,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,  1415,    -1,  1417,    -1,
      -1,    -1,    -1,    -1,  1423,    -1,    -1,    -1,    -1,    -1,
     259,    29,   261,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
    1459,  1460,    -1,    -1,   699,    -1,  1465,    -1,  1467,    67,
      -1,    -1,    -1,    -1,  1473,    -1,  1475,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   316,    10,    11,
      12,    -1,    -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   739,    -1,   741,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,   769,   770,    -1,   116,   117,   118,
     119,   120,   121,    -1,    -1,    67,    -1,    -1,   127,   128,
     785,   786,   787,   788,   789,    -1,    -1,    -1,   793,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   397,   804,
      -1,   400,    -1,    -1,    -1,  1574,    -1,    -1,    -1,    -1,
     815,    -1,    -1,    -1,   163,    -1,   165,    -1,    -1,    -1,
      -1,   189,  1591,   828,    -1,   830,   831,    -1,    -1,   178,
      -1,   180,    -1,    68,   183,    -1,    -1,    -1,    -1,  1608,
     845,    76,    77,    78,    79,  1614,    81,    -1,    -1,   854,
      -1,    -1,    -1,    88,    -1,    -1,  1625,    -1,    -1,    -1,
      -1,    -1,  1631,    -1,    -1,   100,  1635,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   881,    -1,    -1,   884,
      -1,    -1,    -1,    -1,    -1,    -1,   121,  1656,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   191,
     135,   906,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,  1695,   162,   163,   164,
      -1,    -1,    -1,   168,    -1,  1704,    -1,   536,   537,    -1,
      -1,   540,    -1,    -1,    -1,   950,   951,   182,    -1,    -1,
      -1,  1720,   187,    -1,    -1,   190,    -1,   192,    -1,    -1,
    1729,    -1,    -1,    -1,    -1,    -1,   565,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1744,    -1,    -1,    -1,   984,
      -1,   986,    -1,    -1,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,   611,   612,    -1,    10,    11,    12,    -1,    -1,
    1025,   620,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    12,
      55,    -1,  1067,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    29,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,  1107,    -1,    -1,    -1,    -1,  1112,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1127,  1128,    -1,  1130,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,  1146,    -1,  1148,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,  1164,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   770,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,   783,   784,   785,   786,   787,   788,
     789,    -1,  1197,    -1,   793,  1200,   191,    26,    27,    29,
      -1,    30,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   844,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   860,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   873,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   881,  1288,  1289,    -1,    -1,  1292,    -1,  1294,
      -1,    -1,  1297,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,  1306,    -1,   902,  1309,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,  1358,    -1,    -1,    -1,    -1,   188,    -1,
      -1,    -1,    -1,    -1,    -1,   964,    -1,    -1,   967,  1374,
     969,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1383,  1384,
      -1,   210,    -1,    -1,    -1,   984,    -1,    -1,  1393,    -1,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1025,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1459,  1460,  1055,    67,  1057,    -1,
    1465,    -1,  1467,    48,    49,    -1,    -1,    -1,    -1,    54,
    1475,    56,    -1,    -1,   191,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,  1084,    -1,    -1,  1087,    -1,
      -1,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,   338,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,   347,    -1,
      -1,    -1,    -1,    -1,    -1,   354,    -1,    -1,    -1,    -1,
      -1,  1130,   361,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1139,    -1,    -1,   372,  1143,    -1,    -1,  1146,    -1,  1148,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,  1164,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,  1194,  1195,   182,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,  1614,
      -1,    -1,    -1,    -1,    -1,   444,    -1,    -1,    -1,    -1,
    1625,    -1,    -1,    -1,    -1,    -1,  1631,    -1,    -1,    -1,
    1635,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   484,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,  1281,    -1,  1283,    -1,    -1,    -1,    -1,  1288,
    1695,    -1,    67,  1292,    -1,  1294,    -1,    -1,  1297,  1704,
      -1,    -1,    -1,    -1,    -1,    -1,  1305,    -1,    -1,    -1,
      -1,   540,    -1,    -1,    -1,  1720,    -1,  1316,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,   565,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,   540,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,  1374,   565,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,  1391,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,   641,    -1,   189,    -1,  1415,    -1,  1417,    -1,
      -1,    -1,    -1,    -1,  1423,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,   133,    10,    11,    12,    -1,    -1,    -1,
    1459,  1460,    -1,    67,    -1,    -1,  1465,    -1,    -1,    -1,
     699,    -1,    -1,    29,  1473,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     739,    67,   741,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
     769,   770,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,   785,   786,   787,   788,
     789,    -1,    -1,    -1,   793,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1574,    -1,    -1,    -1,    -1,
      -1,   770,    -1,    -1,    -1,    -1,   815,   191,    -1,    -1,
      -1,    -1,  1591,    -1,    -1,    -1,   785,   786,   787,   788,
      -1,   830,   831,    -1,   793,    -1,    -1,    -1,    -1,  1608,
      -1,    -1,    -1,    -1,    -1,  1614,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   854,  1625,    -1,    -1,    -1,
      -1,    -1,  1631,    -1,    -1,   191,  1635,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   881,    -1,    -1,    -1,    -1,  1656,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,   906,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,   881,    -1,    29,    -1,  1695,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      -1,   950,   951,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1729,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1744,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   984,    -1,   986,    -1,    -1,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1025,    -1,    -1,    -1,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,  1025,    -1,  1067,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,  1107,    -1,
      -1,    -1,    -1,  1112,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1127,  1128,
      -1,  1130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1146,    -1,  1148,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,  1164,    -1,    -1,    -1,    -1,
      -1,  1130,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,   133,    -1,    -1,    -1,    -1,    -1,  1146,    -1,  1148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1200,    55,    -1,    -1,  1164,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    79,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,    -1,   127,   128,   129,   130,    -1,    -1,
      -1,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,  1288,
    1289,    -1,    -1,  1292,    -1,  1294,    -1,    -1,  1297,    -1,
     153,    -1,    -1,    -1,    -1,    -1,   159,   160,    -1,   162,
     163,   164,   165,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1288,
      -1,    -1,    -1,  1292,    -1,  1294,    -1,   190,  1297,   192,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,  1358,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,  1374,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    -1,    -1,    -1,    -1,  1374,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
    1459,  1460,    88,    -1,    -1,    -1,  1465,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,  1474,    -1,    -1,    -1,    -1,
      -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     116,   117,   118,   119,   120,   121,    -1,    -1,   124,   125,
    1459,  1460,    -1,    -1,    -1,    -1,  1465,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,   177,    10,    11,    12,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,    -1,    -1,    -1,   192,   193,    -1,   195,
     196,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,  1614,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1625,    -1,    -1,    -1,
      -1,    -1,  1631,    -1,    -1,    -1,  1635,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,  1614,    55,    -1,    -1,  1658,
      -1,    -1,    -1,    -1,    -1,    -1,  1625,    -1,    67,    -1,
      -1,    -1,  1631,    -1,    -1,    -1,  1635,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,  1695,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      49,    -1,    -1,   191,    -1,    54,  1695,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    72,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,
      89,    90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,   108,
     109,   110,   111,    -1,   113,   114,   115,   116,   117,   118,
     119,   120,   121,    -1,   123,   124,   125,   126,   127,   128,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,   165,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,   178,
      -1,   180,    -1,   182,   183,   184,    -1,    -1,   187,    -1,
     189,   190,   191,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    72,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,   120,   121,    -1,   123,   124,
     125,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
     145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
     165,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    -1,   177,   178,    -1,   180,    -1,   182,   183,   184,
      -1,    -1,   187,    -1,   189,   190,   191,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    -1,
      -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,
      91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,
     101,    -1,    -1,    -1,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,   116,   117,   118,   119,   120,
     121,    -1,   123,   124,   125,   126,   127,   128,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,   165,    -1,    -1,   168,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,   190,
      -1,   192,   193,    -1,   195,   196,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    72,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    88,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      97,    -1,    -1,   100,   101,    -1,    -1,    -1,   105,   106,
     107,   108,    -1,   110,   111,    -1,   113,    -1,   115,   116,
     117,   118,   119,   120,   121,    -1,   123,   124,   125,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,   145,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,   165,    -1,
      -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
     187,    -1,   189,   190,   191,   192,   193,    -1,   195,   196,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,    -1,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    72,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,
      93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,    -1,
      -1,    -1,   105,   106,   107,   108,    -1,   110,   111,    -1,
     113,    -1,   115,   116,   117,   118,   119,   120,   121,    -1,
     123,   124,   125,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,   165,    -1,    -1,   168,    -1,    -1,   171,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,   187,    -1,   189,   190,   191,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    72,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,
      89,    90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,   108,
      -1,   110,   111,    -1,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,    -1,   123,   124,   125,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,   165,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,
     189,   190,   191,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    72,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,
     105,   106,   107,   108,    -1,   110,   111,    -1,   113,    -1,
     115,   116,   117,   118,   119,   120,   121,    -1,   123,   124,
     125,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
     145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
     165,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,   187,    -1,   189,   190,   191,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    -1,
      -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    -1,    95,    -1,    97,    -1,    -1,   100,
     101,    -1,    -1,    -1,   105,   106,   107,   108,    -1,   110,
     111,    -1,   113,    -1,   115,   116,   117,   118,   119,   120,
     121,    -1,   123,   124,   125,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,   165,    -1,    -1,   168,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,   190,
      -1,   192,   193,    -1,   195,   196,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    72,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    88,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      97,    98,    -1,   100,   101,    -1,    -1,    -1,   105,   106,
     107,   108,    -1,   110,   111,    -1,   113,    -1,   115,   116,
     117,   118,   119,   120,   121,    -1,   123,   124,   125,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,   145,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,   165,    -1,
      -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
     187,    -1,   189,   190,    -1,   192,   193,    -1,   195,   196,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,    -1,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    72,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,
      93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,    -1,
      -1,    -1,   105,   106,   107,   108,    -1,   110,   111,    -1,
     113,    -1,   115,   116,   117,   118,   119,   120,   121,    -1,
     123,   124,   125,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,   165,    -1,    -1,   168,    -1,    -1,   171,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,   187,    -1,   189,   190,   191,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    72,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,
      89,    90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,   108,
      -1,   110,   111,    -1,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,    -1,   123,   124,   125,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,   165,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,
     189,   190,   191,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    72,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,
      95,    96,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,
     105,   106,   107,   108,    -1,   110,   111,    -1,   113,    -1,
     115,   116,   117,   118,   119,   120,   121,    -1,   123,   124,
     125,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
     145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
     165,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,   187,    -1,   189,   190,    -1,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    -1,
      -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,
      91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,
     101,    -1,    -1,    -1,   105,   106,   107,   108,    -1,   110,
     111,    -1,   113,    -1,   115,   116,   117,   118,   119,   120,
     121,    -1,   123,   124,   125,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,   165,    -1,    -1,   168,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,   190,
     191,   192,   193,    -1,   195,   196,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    72,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    88,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      97,    -1,    -1,   100,   101,    -1,    -1,    -1,   105,   106,
     107,   108,    -1,   110,   111,    -1,   113,    -1,   115,   116,
     117,   118,   119,   120,   121,    -1,   123,   124,   125,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,   145,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,   165,    -1,
      -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
     187,    -1,   189,   190,   191,   192,   193,    -1,   195,   196,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,    -1,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    72,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,
      93,    94,    95,    -1,    97,    -1,    -1,   100,   101,    -1,
      -1,    -1,   105,   106,   107,   108,    -1,   110,   111,    -1,
     113,    -1,   115,   116,   117,   118,   119,   120,   121,    -1,
     123,   124,   125,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,   165,    -1,    -1,   168,    -1,    -1,   171,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,   187,    -1,   189,   190,    -1,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    72,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,
      89,    90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,   108,
      -1,   110,   111,    -1,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,    -1,   123,   124,   125,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,   165,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,
     189,   190,   191,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    72,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,
     105,   106,   107,   108,    -1,   110,   111,    -1,   113,    -1,
     115,   116,   117,   118,   119,   120,   121,    -1,   123,   124,
     125,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
     145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
     165,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,   187,    -1,   189,   190,   191,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    -1,
      -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,
      91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,
     101,    -1,    -1,    -1,   105,   106,   107,   108,    -1,   110,
     111,    -1,   113,    -1,   115,   116,   117,   118,   119,   120,
     121,    -1,   123,   124,   125,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,   165,    -1,    -1,   168,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,   190,
     191,   192,   193,    -1,   195,   196,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    72,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    88,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      97,    -1,    -1,   100,   101,    -1,    -1,    -1,   105,   106,
     107,   108,    -1,   110,   111,    -1,   113,    -1,   115,   116,
     117,   118,   119,   120,   121,    -1,   123,   124,   125,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,   145,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,   165,    -1,
      -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
     187,    -1,   189,   190,    -1,   192,   193,    -1,   195,   196,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    72,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,
      93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,    -1,
      -1,    -1,   105,   106,   107,   108,    -1,   110,   111,    -1,
     113,    -1,   115,   116,   117,   118,   119,   120,   121,    -1,
     123,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,   187,    -1,   189,   190,    -1,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    72,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,
      89,    90,    91,    -1,    93,    -1,    95,    -1,    97,    -1,
      -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,   108,
      -1,   110,   111,    -1,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,    -1,   123,   124,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,
     189,   190,    -1,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    72,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,
     105,   106,   107,   108,    -1,   110,   111,    -1,   113,    -1,
     115,   116,   117,   118,   119,   120,   121,    -1,   123,   124,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
     145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,   187,    -1,   189,   190,    -1,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,
      -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,
      91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,
     101,    -1,    -1,    -1,   105,   106,   107,   108,    -1,   110,
     111,    -1,   113,    -1,   115,   116,   117,   118,   119,   120,
     121,    -1,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,   190,
      -1,   192,   193,    -1,   195,   196,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    72,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    88,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      97,    -1,    -1,   100,   101,    -1,    -1,    -1,   105,   106,
     107,   108,    -1,   110,   111,    -1,   113,    -1,   115,   116,
     117,   118,   119,   120,   121,    -1,   123,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,   145,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,
      -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
     187,    -1,   189,   190,    -1,   192,   193,    -1,   195,   196,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    72,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    88,    89,    90,    91,    -1,
      93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,    -1,
      -1,    -1,   105,   106,   107,   108,    -1,   110,   111,    -1,
     113,    -1,   115,   116,   117,   118,   119,   120,   121,    -1,
     123,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,   187,    -1,   189,   190,    -1,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,   120,   121,    -1,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,
     189,    -1,    -1,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   116,   117,   118,   119,   120,   121,    -1,    -1,   124,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,   166,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,
      -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,
     121,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,    -1,   190,
      -1,   192,   193,    -1,   195,   196,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    36,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,
     117,   118,   119,   120,   121,    -1,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,    -1,   166,
      -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
     187,    -1,    -1,    -1,    -1,   192,   193,    -1,   195,   196,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   116,   117,   118,   119,   120,   121,    -1,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,   187,    -1,    10,    11,    12,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    -1,    -1,    67,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,   105,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,   120,   121,    -1,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,   191,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,
      -1,    -1,    -1,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   116,   117,   118,   119,   120,   121,    -1,    -1,   124,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,   187,    -1,    10,    11,    12,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,
      -1,    67,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,
     121,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,
     171,    -1,    -1,    -1,    -1,   191,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,    11,
      12,   192,   193,    -1,   195,   196,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    -1,    -1,    67,    -1,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,
     117,   118,   119,   120,   121,    -1,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,
      -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
     187,    -1,   189,    -1,    -1,   192,   193,    -1,   195,   196,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   116,   117,   118,   119,   120,   121,    -1,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,   187,    -1,    10,    11,    12,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    -1,    -1,    67,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,   120,   121,    -1,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,   189,    -1,    -1,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,   188,
      -1,    -1,    -1,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   116,   117,   118,   119,   120,   121,    -1,    -1,   124,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    48,    49,    -1,
      -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,
     121,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,    -1,    -1,
      -1,   192,   193,    -1,   195,   196,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,
     117,   118,   119,   120,   121,    -1,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,
      -1,   168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,
     187,    -1,    -1,    -1,    -1,   192,   193,    -1,   195,   196,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   116,   117,   118,   119,   120,   121,    -1,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    71,    -1,    -1,    -1,    -1,    76,    77,    78,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,   120,   121,    -1,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,
      10,    11,    12,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    49,    -1,    -1,    67,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   116,   117,   118,   119,   120,   121,    -1,    -1,   124,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,   189,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,   187,    -1,    10,    11,    12,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,
      -1,    67,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,
     121,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,     3,     4,    -1,     6,     7,    -1,   148,    10,    11,
      12,    13,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,    -1,    27,    -1,   168,    -1,    -1,
     171,    -1,    -1,   189,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,    -1,    -1,
      -1,   192,   193,    55,   195,   196,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    79,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    -1,    -1,    -1,   127,   128,   129,   130,    -1,
      -1,    -1,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,   153,    -1,    -1,    -1,    -1,    -1,   159,   160,    -1,
     162,   163,   164,   165,    27,   167,    29,    -1,   170,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,
     192,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,   191,    -1,
      -1,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
      -1,    -1,    -1,    -1,   128,   129,   130,    -1,    -1,    -1,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,   153,
      -1,    -1,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,   165,    27,   167,    29,    -1,   170,    -1,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,   191,    -1,    -1,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,   127,   128,   129,   130,    -1,    -1,    -1,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   153,    -1,
      -1,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
     165,    27,   167,    29,    -1,   170,    -1,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    -1,    -1,
      -1,    -1,   128,   129,   130,    -1,    -1,    -1,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   153,    -1,    -1,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,   165,
      27,   167,    29,    -1,   170,    -1,    -1,    -1,    -1,    -1,
      -1,   177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
      -1,   128,   129,   130,    -1,    -1,    -1,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,   165,    -1,
     167,    -1,    -1,   170,     3,     4,     5,     6,     7,    -1,
     177,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   153,   154,   155,    -1,    -1,    -1,
     159,   160,    -1,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,    10,    11,    12,    -1,    -1,    -1,   177,   178,
      -1,   180,    -1,   182,   183,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    67,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    79,   188,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
      -1,    -1,   188,   127,   128,   129,   130,    -1,    -1,    -1,
     134,   135,   136,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   153,
      -1,    -1,    -1,    -1,    -1,   159,   160,    27,   162,   163,
     164,   165,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    74,    75,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,    -1,    -1,    -1,   127,   128,   129,
     130,    -1,    -1,    -1,   134,   135,   136,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,    -1,   159,
     160,    27,   162,   163,   164,   165,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    -1,    -1,
      -1,    -1,   128,   129,   130,    -1,    -1,    -1,   134,   135,
     136,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,
      -1,    -1,    -1,   159,   160,    -1,   162,   163,   164,   165,
      -1,   167,    -1,    68,   170,    -1,    -1,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      48,    49,    -1,   168,    -1,    -1,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,
      68,    -1,   187,    -1,    -1,    -1,    -1,   192,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,   135,    -1,   137,
     138,   139,   140,   141,    67,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    68,   162,   163,   164,    -1,    -1,    -1,
     168,    76,    77,    78,    79,    -1,    81,    -1,    -1,   177,
      -1,    -1,    -1,    88,   182,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      68,    -1,    70,   168,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    -1,    81,    -1,    -1,    -1,   182,    -1,    -1,
      88,    -1,   187,    -1,    -1,    -1,    -1,   192,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    68,   162,   163,   164,    -1,    -1,    -1,
     168,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,   182,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    -1,    -1,   192,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,
     155,   156,   157,    -1,   159,   160,    68,   162,   163,   164,
      -1,    -1,    -1,   168,    76,    77,    78,    79,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,   182,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    29,
     192,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,   133,
      29,    -1,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,
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
      -1,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      -1,    67,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    -1,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,    -1,   133,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,    -1,    -1,   168,    76,    77,    78,    79,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    88,   182,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
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
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    29,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67
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
     433,   435,   437,   119,   120,   121,   134,   153,   163,   187,
     204,   232,   313,   332,   410,   332,   187,   332,   332,   332,
     105,   332,   332,   396,   397,   332,   332,   332,   332,   332,
     332,   332,   332,   332,   332,   332,   332,    81,    88,   121,
     148,   187,   210,   351,   368,   371,   376,   410,   413,   410,
      36,   332,   424,   425,   332,   121,   127,   187,   210,   245,
     368,   369,   370,   372,   376,   407,   408,   409,   417,   421,
     422,   187,   323,   373,   187,   323,   342,   324,   332,   218,
     323,   187,   187,   187,   323,   189,   332,   204,   189,   332,
       3,     4,     6,     7,    10,    11,    12,    13,    27,    29,
      55,    57,    69,    70,    71,    72,    73,    74,    75,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   127,   128,   129,   130,   134,   135,   136,
     153,   157,   165,   167,   170,   177,   187,   204,   205,   206,
     212,   438,   453,   454,   456,   189,   329,   332,   190,   225,
     332,   108,   109,   156,   207,   208,   209,    81,   192,   279,
     280,   120,   127,   119,   127,    81,   281,   187,   187,   187,
     187,   204,   251,   441,   187,   187,   324,    81,    87,   149,
     150,   151,   430,   431,   156,   190,   209,   209,   204,   252,
     441,   157,   187,   441,   441,    81,   184,   190,   343,    27,
     322,   326,   332,   333,   410,   414,   214,   190,   419,    87,
     374,   430,    87,   430,   430,    30,   156,   173,   442,   187,
       9,   189,    36,   231,   157,   250,   441,   121,   183,   232,
     314,   189,   189,   189,   189,   189,   189,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    55,    67,   189,    68,    68,   190,
     152,   128,   163,   165,   178,   180,   253,   312,   313,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    65,    66,   131,   132,   400,    68,   190,   405,
     187,   187,    68,   190,   187,   231,   232,    14,   332,   189,
     133,    46,   204,   395,    87,   322,   333,   152,   410,   133,
     194,     9,   381,   246,   322,   333,   410,   442,   152,   187,
     375,   400,   405,   188,   332,    30,   216,     8,   344,     9,
     189,   216,   217,   324,   325,   332,   204,   265,   220,   189,
     189,   189,   135,   136,   456,   456,   173,   187,   108,   456,
      14,   152,   135,   136,   153,   204,   206,   189,   189,   226,
     112,   170,   189,   207,   207,   209,     9,   189,    99,   190,
     410,     9,   189,   127,   127,    14,     9,   189,   410,   434,
     324,   322,   333,   410,   413,   414,   188,   173,   243,   134,
     410,   423,   424,   189,    68,   400,   149,   431,    80,   332,
     410,    87,   149,   431,   209,   203,   189,   190,   238,   248,
     358,   360,    88,   187,   345,   346,   348,   371,   416,   418,
     435,    14,    99,   436,   339,   340,   341,   275,   276,   398,
     399,   188,   188,   188,   188,   188,   191,   215,   216,   233,
     240,   247,   398,   332,   193,   195,   196,   204,   443,   444,
     456,    36,   166,   277,   278,   332,   438,   187,   441,   241,
     231,   332,   332,   332,    30,   332,   332,   332,   332,   332,
     332,   332,   332,   332,   332,   332,   332,   332,   332,   332,
     332,   332,   332,   332,   332,   332,   332,   332,   332,   372,
     332,   332,   420,   420,   332,   426,   427,   127,   190,   205,
     206,   419,   251,   204,   252,   441,   441,   250,   232,    36,
     326,   329,   332,   332,   332,   332,   332,   332,   332,   332,
     332,   332,   332,   332,   332,   157,   190,   204,   401,   402,
     403,   404,   419,   420,   332,   277,   277,   420,   332,   423,
     231,   188,   332,   187,   394,     9,   381,   188,   188,    36,
     332,    36,   332,   375,   188,   188,   188,   417,   418,   419,
     277,   190,   204,   401,   402,   419,   188,   214,   269,   190,
     329,   332,   332,    91,    30,   216,   263,   189,    28,    99,
      14,     9,   188,    30,   190,   266,   456,    29,    88,   212,
     450,   451,   452,   187,     9,    48,    49,    54,    56,    68,
     135,   157,   177,   187,   210,   212,   353,   368,   376,   377,
     378,   204,   455,   214,   187,   224,   189,   189,    99,   208,
     204,   332,   280,   377,    81,     9,   188,   188,   188,   188,
     188,   188,   188,   189,    48,    49,   448,   449,   129,   256,
     187,     9,   188,   188,    81,    82,   204,   432,   204,    68,
     191,   191,   200,   202,    30,   130,   255,   172,    52,   157,
     172,   362,   333,   133,     9,   381,   188,   152,   456,   456,
      14,   344,   275,   214,   185,     9,   382,   456,   457,   400,
     405,   400,   191,     9,   381,   174,   410,   332,   188,     9,
     382,    14,   336,   234,   129,   254,   187,   441,   332,    30,
     194,   194,   133,   191,     9,   381,   332,   442,   187,   244,
     239,   249,    14,   436,   242,   231,    70,   410,   332,   442,
     194,   191,   188,   188,   194,   191,   188,    48,    49,    68,
      76,    77,    78,    88,   135,   148,   177,   204,   384,   386,
     387,   390,   393,   204,   410,   410,   133,   254,   400,   405,
     188,   332,   270,    73,    74,   271,   214,   323,   214,   325,
      99,    36,   134,   260,   410,   377,   204,    30,   216,   264,
     189,   267,   189,   267,     9,   174,    88,   133,   152,     9,
     381,   188,   166,   443,   444,   445,   443,   377,   377,   377,
     377,   377,   380,   383,   187,   152,   187,   377,   152,   190,
      10,    11,    12,    29,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    67,   152,   442,   191,
     368,   190,   228,   204,   191,    14,   410,   189,     9,   174,
     204,   257,   368,   190,   423,   134,   410,    14,   194,   332,
     191,   200,   456,   257,   190,   361,    14,   188,   332,   345,
     419,   189,   456,   185,   191,    30,   446,   399,    36,    81,
     166,   401,   402,   404,   401,   402,   456,    36,   166,   332,
     377,   275,   187,   368,   255,   337,   235,   332,   332,   332,
     191,   187,   277,   256,    30,   255,   456,    14,   254,   441,
     372,   191,   187,    14,    76,    77,    78,   204,   385,   385,
     387,   388,   389,    50,   187,    87,   149,   187,     9,   381,
     188,   394,    36,   332,   255,   191,    73,    74,   272,   323,
     216,   191,   189,    92,   189,   260,   410,   187,   133,   259,
      14,   214,   267,   102,   103,   104,   267,   191,   456,   133,
     456,   204,   450,     9,   188,   381,   133,   194,     9,   381,
     380,   205,   345,   347,   349,   188,   127,   205,   377,   428,
     429,   377,   377,   377,    30,   377,   377,   377,   377,   377,
     377,   377,   377,   377,   377,   377,   377,   377,   377,   377,
     377,   377,   377,   377,   377,   377,   377,   377,   377,   455,
      81,   229,   377,   449,    99,   100,   447,     9,   285,   188,
     187,   326,   329,   332,   194,   191,   436,   285,   158,   171,
     190,   357,   364,   158,   190,   363,   133,   189,   446,   456,
     344,   457,    81,   166,    14,    81,   442,   410,   332,   188,
     275,   190,   275,   187,   133,   187,   277,   188,   190,   456,
     190,   189,   456,   255,   236,   375,   277,   133,   194,     9,
     381,   386,   388,   149,   345,   391,   392,   387,   410,   190,
     323,    30,    75,   216,   189,   325,   259,   423,   260,   188,
     377,    98,   102,   189,   332,    30,   189,   268,   191,   174,
     456,   133,   166,    30,   188,   377,   377,   188,   133,     9,
     381,   188,   133,   191,     9,   381,   377,    30,   188,   214,
     204,   456,   456,   368,     4,   109,   114,   120,   122,   159,
     160,   162,   191,   286,   311,   312,   313,   318,   319,   320,
     321,   398,   423,   191,   190,   191,    52,   332,   332,   332,
     344,    36,    81,   166,    14,    81,   332,   187,   446,   188,
     285,   188,   275,   332,   277,   188,   285,   436,   285,   189,
     190,   187,   188,   387,   387,   188,   133,   188,     9,   381,
     285,    30,   214,   189,   188,   188,   188,   221,   189,   189,
     268,   214,   456,   456,   133,   377,   345,   377,   377,   377,
     190,   191,   447,   129,   130,   178,   205,   439,   456,   258,
     368,   109,   321,    29,   122,   135,   136,   157,   163,   295,
     296,   297,   298,   368,   161,   303,   304,   125,   187,   204,
     305,   306,   287,   232,   456,     9,   189,     9,   189,   189,
     436,   312,   188,   282,   157,   359,   191,   191,    81,   166,
      14,    81,   332,   277,   114,   334,   446,   191,   446,   188,
     188,   191,   190,   191,   285,   275,   133,   387,   345,   191,
     214,   219,   222,    30,   216,   262,   214,   188,   377,   133,
     133,   214,   368,   368,   441,    14,   205,     9,   189,   190,
     439,   436,   298,   173,   190,     9,   189,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    55,    69,
      70,    71,    72,    73,    74,    75,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   134,   135,   137,
     138,   139,   140,   141,   153,   154,   155,   165,   167,   168,
     170,   177,   178,   180,   182,   183,   204,   365,   366,     9,
     189,   157,   161,   204,   306,   307,   308,   189,    81,   317,
     231,   288,   439,   439,    14,   232,   191,   283,   284,   439,
      14,    81,   332,   188,   187,   190,   189,   190,   309,   334,
     446,   282,   191,   188,   387,   133,    30,   216,   261,   262,
     214,   377,   377,   191,   189,   189,   377,   368,   291,   456,
     299,   300,   376,   296,    14,    30,    49,   301,   304,     9,
      34,   188,    29,    48,    51,    14,     9,   189,   206,   440,
     317,    14,   456,   231,   189,    14,   332,    36,    81,   356,
     214,   214,   190,   309,   191,   446,   387,   214,    96,   227,
     191,   204,   212,   292,   293,   294,     9,   174,     9,   381,
     191,   377,   366,   366,    57,   302,   307,   307,    29,    48,
      51,   377,    81,   173,   187,   189,   377,   441,   377,    81,
       9,   382,   191,   191,   214,   309,    94,   189,   112,   223,
     152,    99,   456,   376,   164,    14,   448,   289,   187,    36,
      81,   188,   191,   189,   187,   170,   230,   204,   312,   313,
     174,   377,   174,   273,   274,   399,   290,    81,   368,   228,
     167,   204,   189,   188,     9,   382,   116,   117,   118,   315,
     316,   273,    81,   258,   189,   446,   399,   457,   188,   188,
     189,   189,   190,   310,   315,    36,    81,   166,   446,   190,
     214,   457,    81,   166,    14,    81,   310,   214,   191,    36,
      81,   166,    14,    81,   332,   191,    81,   166,    14,    81,
     332,    14,    81,   332,   332
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
#line 2618 "hphp.y"
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
#line 2633 "hphp.y"
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
#line 2768 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 2844 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { (yyval).reset();;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 850:

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

  case 851:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 856:

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

  case 857:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { (yyval).reset();;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2937 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 892:

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

  case 893:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3088 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    {;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    {;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
    {;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 949:

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

  case 950:

/* Line 1455 of yacc.c  */
#line 3157 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3162 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3163 "hphp.y"
    { ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3189 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3196 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3212 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3243 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13612 "hphp.7.tab.cpp"
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
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

