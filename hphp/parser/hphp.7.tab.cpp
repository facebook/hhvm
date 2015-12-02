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
#define YYLAST   16805

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  197
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  266
/* YYNRULES -- Number of rules.  */
#define YYNRULES  984
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1807

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
    2702,  2707,  2711,  2715,  2717,  2719,  2721,  2723,  2725,  2729,
    2733,  2738,  2743,  2747,  2749,  2751,  2759,  2769,  2777,  2784,
    2793,  2795,  2800,  2805,  2807,  2809,  2814,  2817,  2819,  2820,
    2822,  2824,  2826,  2830,  2834,  2838,  2839,  2841,  2843,  2847,
    2851,  2854,  2858,  2865,  2866,  2868,  2873,  2876,  2877,  2883,
    2887,  2891,  2893,  2900,  2905,  2910,  2913,  2916,  2917,  2923,
    2927,  2931,  2933,  2936,  2937,  2943,  2947,  2951,  2953,  2956,
    2959,  2961,  2964,  2966,  2971,  2975,  2979,  2986,  2990,  2992,
    2994,  2996,  3001,  3006,  3011,  3016,  3021,  3026,  3029,  3032,
    3037,  3040,  3043,  3045,  3049,  3053,  3057,  3058,  3061,  3067,
    3074,  3081,  3089,  3091,  3094,  3096,  3099,  3101,  3106,  3108,
    3113,  3117,  3118,  3120,  3124,  3127,  3131,  3133,  3135,  3136,
    3137,  3140,  3143,  3146,  3151,  3154,  3160,  3164,  3166,  3168,
    3169,  3173,  3178,  3184,  3188,  3190,  3193,  3194,  3199,  3201,
    3205,  3208,  3211,  3214,  3216,  3218,  3220,  3222,  3226,  3231,
    3238,  3240,  3249,  3256,  3258
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     198,     0,    -1,    -1,   199,   200,    -1,   200,   201,    -1,
      -1,   221,    -1,   238,    -1,   245,    -1,   242,    -1,   252,
      -1,   442,    -1,   126,   187,   188,   189,    -1,   153,   214,
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
      -1,   215,   447,    -1,   215,   447,    -1,   218,     9,   443,
      14,   382,    -1,   109,   443,    14,   382,    -1,   219,   220,
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
     189,    -1,   123,   187,   439,   188,   189,    -1,   189,    -1,
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
      -1,   108,    -1,    -1,   237,   236,   446,   239,   187,   280,
     188,   451,   314,    -1,    -1,   318,   237,   236,   446,   240,
     187,   280,   188,   451,   314,    -1,    -1,   403,   317,   237,
     236,   446,   241,   187,   280,   188,   451,   314,    -1,    -1,
     163,   204,   243,    30,   461,   441,   190,   287,   191,    -1,
      -1,   403,   163,   204,   244,    30,   461,   441,   190,   287,
     191,    -1,    -1,   258,   255,   246,   259,   260,   190,   290,
     191,    -1,    -1,   403,   258,   255,   247,   259,   260,   190,
     290,   191,    -1,    -1,   128,   256,   248,   261,   190,   290,
     191,    -1,    -1,   403,   128,   256,   249,   261,   190,   290,
     191,    -1,    -1,   127,   251,   380,   259,   260,   190,   290,
     191,    -1,    -1,   165,   257,   253,   260,   190,   290,   191,
      -1,    -1,   403,   165,   257,   254,   260,   190,   290,   191,
      -1,   446,    -1,   157,    -1,   446,    -1,   446,    -1,   127,
      -1,   120,   127,    -1,   120,   119,   127,    -1,   119,   120,
     127,    -1,   119,   127,    -1,   129,   373,    -1,    -1,   130,
     262,    -1,    -1,   129,   262,    -1,    -1,   373,    -1,   262,
       9,   373,    -1,   373,    -1,   263,     9,   373,    -1,   133,
     265,    -1,    -1,   415,    -1,    36,   415,    -1,   134,   187,
     428,   188,    -1,   221,    -1,    30,   219,    94,   189,    -1,
     221,    -1,    30,   219,    96,   189,    -1,   221,    -1,    30,
     219,    92,   189,    -1,   221,    -1,    30,   219,    98,   189,
      -1,   204,    14,   382,    -1,   270,     9,   204,    14,   382,
      -1,   190,   272,   191,    -1,   190,   189,   272,   191,    -1,
      30,   272,   102,   189,    -1,    30,   189,   272,   102,   189,
      -1,   272,   103,   337,   273,   219,    -1,   272,   104,   273,
     219,    -1,    -1,    30,    -1,   189,    -1,   274,    73,   328,
     221,    -1,    -1,   275,    73,   328,    30,   219,    -1,    -1,
      74,   221,    -1,    -1,    74,    30,   219,    -1,    -1,   279,
       9,   404,   320,   462,   166,    81,    -1,   279,     9,   404,
     320,   462,    36,   166,    81,    -1,   279,     9,   404,   320,
     462,   166,    -1,   279,   387,    -1,   404,   320,   462,   166,
      81,    -1,   404,   320,   462,    36,   166,    81,    -1,   404,
     320,   462,   166,    -1,    -1,   404,   320,   462,    81,    -1,
     404,   320,   462,    36,    81,    -1,   404,   320,   462,    36,
      81,    14,   337,    -1,   404,   320,   462,    81,    14,   337,
      -1,   279,     9,   404,   320,   462,    81,    -1,   279,     9,
     404,   320,   462,    36,    81,    -1,   279,     9,   404,   320,
     462,    36,    81,    14,   337,    -1,   279,     9,   404,   320,
     462,    81,    14,   337,    -1,   281,     9,   404,   462,   166,
      81,    -1,   281,     9,   404,   462,    36,   166,    81,    -1,
     281,     9,   404,   462,   166,    -1,   281,   387,    -1,   404,
     462,   166,    81,    -1,   404,   462,    36,   166,    81,    -1,
     404,   462,   166,    -1,    -1,   404,   462,    81,    -1,   404,
     462,    36,    81,    -1,   404,   462,    36,    81,    14,   337,
      -1,   404,   462,    81,    14,   337,    -1,   281,     9,   404,
     462,    81,    -1,   281,     9,   404,   462,    36,    81,    -1,
     281,     9,   404,   462,    36,    81,    14,   337,    -1,   281,
       9,   404,   462,    81,    14,   337,    -1,   283,   387,    -1,
      -1,   337,    -1,    36,   415,    -1,   166,   337,    -1,   283,
       9,   337,    -1,   283,     9,   166,   337,    -1,   283,     9,
      36,   415,    -1,   284,     9,   285,    -1,   285,    -1,    81,
      -1,   192,   415,    -1,   192,   190,   337,   191,    -1,   286,
       9,    81,    -1,   286,     9,    81,    14,   382,    -1,    81,
      -1,    81,    14,   382,    -1,   287,   288,    -1,    -1,   289,
     189,    -1,   444,    14,   382,    -1,   290,   291,    -1,    -1,
      -1,   316,   292,   322,   189,    -1,    -1,   318,   461,   293,
     322,   189,    -1,   323,   189,    -1,   324,   189,    -1,   325,
     189,    -1,    -1,   317,   237,   236,   445,   187,   294,   278,
     188,   451,   315,    -1,    -1,   403,   317,   237,   236,   446,
     187,   295,   278,   188,   451,   315,    -1,   159,   300,   189,
      -1,   160,   308,   189,    -1,   162,   310,   189,    -1,     4,
     129,   373,   189,    -1,     4,   130,   373,   189,    -1,   114,
     263,   189,    -1,   114,   263,   190,   296,   191,    -1,   296,
     297,    -1,   296,   298,    -1,    -1,   217,   152,   204,   167,
     263,   189,    -1,   299,    99,   317,   204,   189,    -1,   299,
      99,   318,   189,    -1,   217,   152,   204,    -1,   204,    -1,
     301,    -1,   300,     9,   301,    -1,   302,   370,   306,   307,
      -1,   157,    -1,    29,   303,    -1,   303,    -1,   135,    -1,
     135,   173,   461,   174,    -1,   135,   173,   461,     9,   461,
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
      14,   382,    -1,   323,     9,   444,    14,   382,    -1,   109,
     444,    14,   382,    -1,   324,     9,   444,    -1,   120,   109,
     444,    -1,   120,   326,   441,    -1,   326,   441,    14,   461,
      -1,   109,   178,   446,    -1,   187,   327,   188,    -1,    70,
     377,   380,    -1,    70,   250,    -1,    69,   337,    -1,   362,
      -1,   357,    -1,   187,   337,   188,    -1,   329,     9,   337,
      -1,   337,    -1,   329,    -1,    -1,    27,    -1,    27,   337,
      -1,    27,   337,   133,   337,    -1,   187,   331,   188,    -1,
     415,    14,   331,    -1,   134,   187,   428,   188,    14,   331,
      -1,    28,   337,    -1,   415,    14,   334,    -1,   134,   187,
     428,   188,    14,   334,    -1,   338,    -1,   415,    -1,   327,
      -1,   419,    -1,   418,    -1,   134,   187,   428,   188,    14,
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
      30,   337,    -1,   337,    31,   337,    -1,   438,    -1,    64,
     337,    -1,    63,   337,    -1,    62,   337,    -1,    61,   337,
      -1,    60,   337,    -1,    59,   337,    -1,    58,   337,    -1,
      71,   378,    -1,    57,   337,    -1,   384,    -1,   356,    -1,
     355,    -1,   193,   379,   193,    -1,    13,   337,    -1,   359,
      -1,   114,   187,   361,   387,   188,    -1,    -1,    -1,   237,
     236,   187,   341,   280,   188,   451,   339,   190,   219,   191,
      -1,    -1,   318,   237,   236,   187,   342,   280,   188,   451,
     339,   190,   219,   191,    -1,    -1,   183,    81,   344,   349,
      -1,    -1,   183,   184,   345,   280,   185,   451,   349,    -1,
      -1,   183,   190,   346,   219,   191,    -1,    -1,    81,   347,
     349,    -1,    -1,   184,   348,   280,   185,   451,   349,    -1,
       8,   337,    -1,     8,   334,    -1,     8,   190,   219,   191,
      -1,    88,    -1,   440,    -1,   351,     9,   350,   133,   337,
      -1,   350,   133,   337,    -1,   352,     9,   350,   133,   382,
      -1,   350,   133,   382,    -1,   351,   386,    -1,    -1,   352,
     386,    -1,    -1,   177,   187,   353,   188,    -1,   135,   187,
     429,   188,    -1,    68,   429,   194,    -1,   373,   190,   431,
     191,    -1,   373,   190,   433,   191,    -1,   359,    68,   425,
     194,    -1,   360,    68,   425,   194,    -1,   356,    -1,   440,
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
     375,   152,   424,    -1,   374,    -1,   421,    -1,   376,   152,
     424,    -1,   373,    -1,   121,    -1,   426,    -1,   187,   188,
      -1,   328,    -1,    -1,    -1,    87,    -1,   435,    -1,   187,
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
     127,    -1,   215,    -1,    80,    -1,   440,    -1,   381,    -1,
     195,   435,   195,    -1,   196,   435,   196,    -1,   148,   435,
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
     191,    -1,   406,    -1,   424,    -1,   204,    -1,   190,   337,
     191,    -1,   408,    -1,   424,    -1,    68,   425,   194,    -1,
     190,   337,   191,    -1,   416,   410,    -1,   187,   327,   188,
     410,    -1,   427,   410,    -1,   187,   327,   188,   410,    -1,
     187,   327,   188,   405,   407,    -1,   187,   338,   188,   405,
     407,    -1,   187,   327,   188,   405,   406,    -1,   187,   338,
     188,   405,   406,    -1,   422,    -1,   372,    -1,   420,    -1,
     421,    -1,   411,    -1,   413,    -1,   415,   405,   407,    -1,
     376,   152,   424,    -1,   417,   187,   282,   188,    -1,   418,
     187,   282,   188,    -1,   187,   415,   188,    -1,   372,    -1,
     420,    -1,   421,    -1,   411,    -1,   415,   405,   407,    -1,
     414,    -1,   417,   187,   282,   188,    -1,   187,   415,   188,
      -1,   376,   152,   424,    -1,   422,    -1,   411,    -1,   372,
      -1,   356,    -1,   381,    -1,   187,   415,   188,    -1,   187,
     338,   188,    -1,   418,   187,   282,   188,    -1,   417,   187,
     282,   188,    -1,   187,   419,   188,    -1,   340,    -1,   343,
      -1,   415,   405,   409,   447,   187,   282,   188,    -1,   187,
     327,   188,   405,   409,   447,   187,   282,   188,    -1,   376,
     152,   206,   447,   187,   282,   188,    -1,   376,   152,   424,
     187,   282,   188,    -1,   376,   152,   190,   337,   191,   187,
     282,   188,    -1,   423,    -1,   423,    68,   425,   194,    -1,
     423,   190,   337,   191,    -1,   424,    -1,    81,    -1,   192,
     190,   337,   191,    -1,   192,   424,    -1,   337,    -1,    -1,
     422,    -1,   412,    -1,   413,    -1,   426,   405,   407,    -1,
     375,   152,   422,    -1,   187,   415,   188,    -1,    -1,   412,
      -1,   414,    -1,   426,   405,   406,    -1,   187,   415,   188,
      -1,   428,     9,    -1,   428,     9,   415,    -1,   428,     9,
     134,   187,   428,   188,    -1,    -1,   415,    -1,   134,   187,
     428,   188,    -1,   430,   386,    -1,    -1,   430,     9,   337,
     133,   337,    -1,   430,     9,   337,    -1,   337,   133,   337,
      -1,   337,    -1,   430,     9,   337,   133,    36,   415,    -1,
     430,     9,    36,   415,    -1,   337,   133,    36,   415,    -1,
      36,   415,    -1,   432,   386,    -1,    -1,   432,     9,   337,
     133,   337,    -1,   432,     9,   337,    -1,   337,   133,   337,
      -1,   337,    -1,   434,   386,    -1,    -1,   434,     9,   382,
     133,   382,    -1,   434,     9,   382,    -1,   382,   133,   382,
      -1,   382,    -1,   435,   436,    -1,   435,    87,    -1,   436,
      -1,    87,   436,    -1,    81,    -1,    81,    68,   437,   194,
      -1,    81,   405,   204,    -1,   150,   337,   191,    -1,   150,
      80,    68,   337,   194,   191,    -1,   151,   415,   191,    -1,
     204,    -1,    82,    -1,    81,    -1,   124,   187,   329,   188,
      -1,   125,   187,   415,   188,    -1,   125,   187,   338,   188,
      -1,   125,   187,   419,   188,    -1,   125,   187,   418,   188,
      -1,   125,   187,   327,   188,    -1,     7,   337,    -1,     6,
     337,    -1,     5,   187,   337,   188,    -1,     4,   337,    -1,
       3,   337,    -1,   415,    -1,   439,     9,   415,    -1,   376,
     152,   205,    -1,   376,   152,   127,    -1,    -1,    99,   461,
      -1,   178,   446,    14,   461,   189,    -1,   403,   178,   446,
      14,   461,   189,    -1,   180,   446,   441,    14,   461,   189,
      -1,   403,   180,   446,   441,    14,   461,   189,    -1,   206,
      -1,   461,   206,    -1,   205,    -1,   461,   205,    -1,   206,
      -1,   206,   173,   453,   174,    -1,   204,    -1,   204,   173,
     453,   174,    -1,   173,   449,   174,    -1,    -1,   461,    -1,
     448,     9,   461,    -1,   448,   386,    -1,   448,     9,   166,
      -1,   449,    -1,   166,    -1,    -1,    -1,    30,   461,    -1,
      99,   461,    -1,   100,   461,    -1,   453,     9,   454,   204,
      -1,   454,   204,    -1,   453,     9,   454,   204,   452,    -1,
     454,   204,   452,    -1,    48,    -1,    49,    -1,    -1,    88,
     133,   461,    -1,    29,    88,   133,   461,    -1,   217,   152,
     204,   133,   461,    -1,   456,     9,   455,    -1,   455,    -1,
     456,   386,    -1,    -1,   177,   187,   457,   188,    -1,   217,
      -1,   204,   152,   460,    -1,   204,   447,    -1,    29,   461,
      -1,    57,   461,    -1,   217,    -1,   135,    -1,   136,    -1,
     458,    -1,   459,   152,   460,    -1,   135,   173,   461,   174,
      -1,   135,   173,   461,     9,   461,   174,    -1,   157,    -1,
     187,   108,   187,   450,   188,    30,   461,   188,    -1,   187,
     461,     9,   448,   386,   188,    -1,   461,    -1,    -1
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
    1005,  1006,  1010,  1008,  1017,  1015,  1022,  1032,  1026,  1036,
    1034,  1038,  1039,  1043,  1044,  1045,  1046,  1047,  1048,  1049,
    1050,  1051,  1059,  1059,  1064,  1070,  1074,  1074,  1082,  1083,
    1087,  1088,  1092,  1098,  1096,  1111,  1108,  1124,  1121,  1138,
    1137,  1146,  1144,  1156,  1155,  1174,  1172,  1191,  1190,  1199,
    1197,  1208,  1208,  1215,  1214,  1226,  1224,  1237,  1238,  1242,
    1245,  1248,  1249,  1250,  1253,  1254,  1257,  1259,  1262,  1263,
    1266,  1267,  1270,  1271,  1275,  1276,  1281,  1282,  1285,  1286,
    1287,  1291,  1292,  1296,  1297,  1301,  1302,  1306,  1307,  1312,
    1313,  1319,  1320,  1321,  1322,  1325,  1328,  1330,  1333,  1334,
    1338,  1340,  1343,  1346,  1349,  1350,  1353,  1354,  1358,  1364,
    1370,  1377,  1379,  1384,  1389,  1395,  1399,  1403,  1407,  1412,
    1417,  1422,  1427,  1433,  1442,  1447,  1452,  1458,  1460,  1464,
    1468,  1473,  1477,  1480,  1483,  1487,  1491,  1495,  1499,  1504,
    1512,  1514,  1517,  1518,  1519,  1520,  1522,  1524,  1529,  1530,
    1533,  1534,  1535,  1539,  1540,  1542,  1543,  1547,  1549,  1552,
    1556,  1562,  1564,  1567,  1567,  1571,  1570,  1574,  1576,  1579,
    1582,  1580,  1596,  1592,  1606,  1608,  1610,  1612,  1614,  1616,
    1618,  1622,  1623,  1624,  1627,  1633,  1637,  1643,  1646,  1651,
    1653,  1658,  1663,  1667,  1668,  1672,  1673,  1675,  1677,  1683,
    1684,  1686,  1690,  1691,  1696,  1700,  1701,  1705,  1706,  1710,
    1712,  1718,  1723,  1724,  1726,  1730,  1731,  1732,  1733,  1737,
    1738,  1739,  1740,  1741,  1742,  1744,  1749,  1752,  1753,  1757,
    1758,  1762,  1763,  1766,  1767,  1770,  1771,  1774,  1775,  1779,
    1780,  1781,  1782,  1783,  1784,  1785,  1789,  1790,  1793,  1794,
    1795,  1798,  1800,  1802,  1803,  1806,  1808,  1812,  1814,  1818,
    1822,  1826,  1831,  1832,  1834,  1835,  1836,  1837,  1840,  1844,
    1845,  1849,  1850,  1854,  1855,  1856,  1857,  1861,  1865,  1870,
    1874,  1878,  1883,  1884,  1885,  1886,  1887,  1891,  1893,  1894,
    1895,  1898,  1899,  1900,  1901,  1902,  1903,  1904,  1905,  1906,
    1907,  1908,  1909,  1910,  1911,  1912,  1913,  1914,  1915,  1916,
    1917,  1918,  1919,  1920,  1921,  1922,  1923,  1924,  1925,  1926,
    1927,  1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,  1936,
    1937,  1938,  1939,  1940,  1942,  1943,  1945,  1946,  1948,  1949,
    1950,  1951,  1952,  1953,  1954,  1955,  1956,  1957,  1958,  1959,
    1960,  1961,  1962,  1963,  1964,  1965,  1966,  1967,  1971,  1975,
    1980,  1979,  1994,  1992,  2010,  2009,  2028,  2027,  2046,  2045,
    2063,  2063,  2078,  2078,  2096,  2097,  2098,  2103,  2105,  2109,
    2113,  2119,  2123,  2129,  2131,  2135,  2137,  2141,  2145,  2146,
    2150,  2157,  2164,  2166,  2171,  2172,  2173,  2174,  2176,  2180,
    2181,  2182,  2183,  2187,  2193,  2202,  2215,  2216,  2219,  2222,
    2225,  2226,  2229,  2233,  2236,  2239,  2246,  2247,  2251,  2252,
    2254,  2258,  2259,  2260,  2261,  2262,  2263,  2264,  2265,  2266,
    2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,  2275,  2276,
    2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,  2286,
    2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,  2295,  2296,
    2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,  2305,  2306,
    2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314,  2315,  2316,
    2317,  2318,  2319,  2320,  2321,  2322,  2323,  2324,  2325,  2326,
    2327,  2328,  2329,  2330,  2331,  2332,  2333,  2334,  2335,  2336,
    2337,  2341,  2346,  2347,  2351,  2352,  2353,  2354,  2356,  2360,
    2361,  2372,  2373,  2375,  2387,  2388,  2389,  2393,  2394,  2395,
    2399,  2400,  2401,  2404,  2406,  2410,  2411,  2412,  2413,  2415,
    2416,  2417,  2418,  2419,  2420,  2421,  2422,  2423,  2424,  2427,
    2432,  2433,  2434,  2436,  2437,  2439,  2440,  2441,  2442,  2444,
    2446,  2448,  2450,  2452,  2453,  2454,  2455,  2456,  2457,  2458,
    2459,  2460,  2461,  2462,  2463,  2464,  2465,  2466,  2467,  2468,
    2470,  2472,  2474,  2476,  2477,  2480,  2481,  2485,  2489,  2491,
    2495,  2498,  2501,  2507,  2508,  2509,  2510,  2511,  2512,  2513,
    2518,  2520,  2524,  2525,  2528,  2529,  2533,  2536,  2538,  2540,
    2544,  2545,  2546,  2547,  2550,  2554,  2555,  2556,  2557,  2561,
    2563,  2570,  2571,  2572,  2573,  2574,  2575,  2577,  2578,  2583,
    2585,  2588,  2591,  2593,  2595,  2598,  2600,  2604,  2606,  2609,
    2612,  2618,  2620,  2623,  2624,  2629,  2632,  2636,  2636,  2641,
    2644,  2645,  2649,  2650,  2654,  2655,  2656,  2660,  2665,  2670,
    2671,  2675,  2680,  2685,  2686,  2690,  2691,  2696,  2698,  2703,
    2714,  2728,  2740,  2755,  2756,  2757,  2758,  2759,  2760,  2761,
    2771,  2780,  2782,  2784,  2788,  2789,  2790,  2791,  2792,  2808,
    2809,  2811,  2813,  2820,  2821,  2822,  2823,  2824,  2825,  2826,
    2827,  2829,  2834,  2838,  2839,  2843,  2846,  2853,  2857,  2866,
    2873,  2881,  2883,  2884,  2888,  2889,  2891,  2896,  2897,  2908,
    2909,  2910,  2911,  2922,  2925,  2928,  2929,  2930,  2931,  2942,
    2946,  2947,  2948,  2950,  2951,  2952,  2956,  2958,  2961,  2963,
    2964,  2965,  2966,  2969,  2971,  2972,  2976,  2978,  2981,  2983,
    2984,  2985,  2989,  2991,  2994,  2997,  2999,  3001,  3005,  3006,
    3008,  3009,  3015,  3016,  3018,  3028,  3030,  3032,  3035,  3036,
    3037,  3041,  3042,  3043,  3044,  3045,  3046,  3047,  3048,  3049,
    3050,  3051,  3055,  3056,  3060,  3062,  3070,  3072,  3076,  3080,
    3085,  3089,  3097,  3098,  3102,  3103,  3109,  3110,  3119,  3120,
    3128,  3131,  3135,  3138,  3143,  3148,  3150,  3151,  3152,  3156,
    3157,  3161,  3162,  3165,  3168,  3170,  3174,  3180,  3181,  3182,
    3186,  3190,  3200,  3208,  3210,  3214,  3216,  3221,  3227,  3230,
    3235,  3243,  3246,  3249,  3250,  3253,  3256,  3257,  3262,  3265,
    3269,  3273,  3279,  3289,  3290
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
     416,   416,   416,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   418,   419,   419,   420,   420,   421,   421,   421,
     422,   423,   423,   423,   424,   424,   424,   425,   425,   426,
     426,   426,   426,   426,   426,   427,   427,   427,   427,   427,
     428,   428,   428,   428,   428,   428,   429,   429,   430,   430,
     430,   430,   430,   430,   430,   430,   431,   431,   432,   432,
     432,   432,   433,   433,   434,   434,   434,   434,   435,   435,
     435,   435,   436,   436,   436,   436,   436,   436,   437,   437,
     437,   438,   438,   438,   438,   438,   438,   438,   438,   438,
     438,   438,   439,   439,   440,   440,   441,   441,   442,   442,
     442,   442,   443,   443,   444,   444,   445,   445,   446,   446,
     447,   447,   448,   448,   449,   450,   450,   450,   450,   451,
     451,   452,   452,   453,   453,   453,   453,   454,   454,   454,
     455,   455,   455,   456,   456,   457,   457,   458,   459,   460,
     460,   461,   461,   461,   461,   461,   461,   461,   461,   461,
     461,   461,   461,   462,   462
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
       4,     3,     3,     1,     1,     1,     1,     1,     3,     3,
       4,     4,     3,     1,     1,     7,     9,     7,     6,     8,
       1,     4,     4,     1,     1,     4,     2,     1,     0,     1,
       1,     1,     3,     3,     3,     0,     1,     1,     3,     3,
       2,     3,     6,     0,     1,     4,     2,     0,     5,     3,
       3,     1,     6,     4,     4,     2,     2,     0,     5,     3,
       3,     1,     2,     0,     5,     3,     3,     1,     2,     2,
       1,     2,     1,     4,     3,     3,     6,     3,     1,     1,
       1,     4,     4,     4,     4,     4,     4,     2,     2,     4,
       2,     2,     1,     3,     3,     3,     0,     2,     5,     6,
       6,     7,     1,     2,     1,     2,     1,     4,     1,     4,
       3,     0,     1,     3,     2,     3,     1,     1,     0,     0,
       2,     2,     2,     4,     2,     5,     3,     1,     1,     0,
       3,     4,     5,     3,     1,     2,     0,     4,     1,     3,
       2,     2,     2,     1,     1,     1,     1,     3,     4,     6,
       1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   423,     0,   787,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   877,     0,
     865,   669,     0,   675,   676,   677,    25,   734,   854,   150,
     151,   678,     0,   131,     0,     0,     0,     0,    26,     0,
       0,     0,     0,   182,     0,     0,     0,     0,     0,     0,
     389,   390,   391,   394,   393,   392,     0,     0,     0,     0,
     211,     0,     0,     0,   682,   684,   685,   679,   680,     0,
       0,     0,   686,   681,     0,   653,    27,    28,    29,    31,
      30,     0,   683,     0,     0,     0,     0,   687,   395,   522,
       0,   149,   121,     0,   670,     0,     0,     4,   111,   113,
     733,     0,   652,     0,     6,   181,     7,     9,     8,    10,
       0,     0,   387,   434,     0,     0,     0,     0,     0,     0,
       0,   432,   843,   844,   504,   503,   417,   507,     0,   416,
     814,   654,   661,     0,   736,   502,   386,   817,   818,   829,
     433,     0,     0,   436,   435,   815,   816,   813,   850,   853,
     492,   735,    11,   394,   393,   392,     0,     0,    31,     0,
     111,   181,     0,   921,   433,   920,     0,   918,   917,   506,
       0,   424,   429,     0,     0,   474,   475,   476,   477,   501,
     499,   498,   497,   496,   495,   494,   493,   854,   678,   656,
       0,     0,   941,   836,   654,     0,   655,   456,     0,   454,
       0,   881,     0,   743,   415,   665,   201,     0,   941,   414,
     664,   659,     0,   674,   655,   860,   861,   867,   859,   666,
       0,     0,   668,   500,     0,     0,     0,     0,   420,     0,
     129,   422,     0,     0,   135,   137,     0,     0,   139,     0,
      72,    71,    66,    65,    57,    58,    49,    69,    80,     0,
      52,     0,    64,    56,    62,    82,    75,    74,    47,    70,
      89,    90,    48,    85,    45,    86,    46,    87,    44,    91,
      79,    83,    88,    76,    77,    51,    78,    81,    43,    73,
      59,    92,    67,    60,    50,    42,    41,    40,    39,    38,
      37,    61,    93,    95,    54,    35,    36,    63,   974,   975,
      55,   980,    34,    53,    84,     0,     0,   111,    94,   932,
     973,     0,   976,     0,     0,   141,     0,     0,   172,     0,
       0,     0,     0,     0,     0,   745,     0,    99,   101,   300,
       0,     0,   299,     0,   215,     0,   212,   305,     0,     0,
       0,     0,     0,   938,   197,   209,   873,   877,     0,   902,
       0,   689,     0,     0,     0,   900,     0,    16,     0,   115,
     189,   203,   210,   559,   534,     0,   926,   514,   516,   518,
     791,   423,   434,     0,     0,   432,   433,   435,     0,     0,
     856,   671,     0,   672,     0,     0,     0,   171,     0,     0,
     117,   291,     0,    24,   180,     0,   208,   193,   207,   392,
     395,   181,   388,   164,   165,   166,   167,   168,   170,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   865,     0,   163,   858,
     858,   887,     0,     0,     0,     0,     0,     0,     0,     0,
     385,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   455,   453,   792,   793,     0,   858,
       0,   805,   291,   291,   858,     0,   873,     0,   181,     0,
       0,   143,     0,   789,   784,   743,     0,   434,   432,     0,
     885,     0,   539,   742,   876,   674,   434,   432,   433,   117,
       0,   291,   413,     0,   807,   667,     0,   121,   251,     0,
     521,     0,   146,     0,     0,   421,     0,     0,     0,     0,
       0,   138,   162,   140,   974,   975,   971,   972,     0,   966,
       0,     0,     0,     0,    68,    33,    55,    32,   933,   169,
     142,   121,     0,   159,   161,     0,     0,     0,     0,   102,
       0,   744,   100,    18,     0,    96,     0,   301,     0,   144,
     214,   213,     0,     0,   145,   922,     0,     0,   434,   432,
     433,   436,   435,     0,   959,   221,     0,   874,     0,     0,
     147,     0,     0,   688,   901,   734,     0,     0,   899,   739,
     898,   114,     5,    13,    14,     0,   219,     0,     0,   527,
       0,     0,   743,     0,     0,   662,   657,   528,     0,     0,
       0,     0,   791,   121,     0,   745,   790,   984,   412,   426,
     488,   823,   842,   126,   120,   122,   123,   124,   125,   386,
       0,   505,   737,   738,   112,   743,     0,   942,     0,     0,
       0,   745,   292,     0,   510,   183,   217,     0,   459,   461,
     460,     0,     0,   491,   457,   458,   462,   464,   463,   479,
     478,   481,   480,   482,   484,   486,   485,   483,   473,   472,
     466,   467,   465,   468,   469,   471,   487,   470,   857,     0,
       0,   891,     0,   743,   925,     0,   924,   941,   820,   199,
     191,   205,     0,   926,   195,   181,     0,   427,   430,   438,
     452,   451,   450,   449,   448,   447,   446,   445,   444,   443,
     442,   441,   795,     0,   794,   797,   819,   801,   941,   798,
       0,     0,     0,     0,     0,     0,     0,     0,   919,   425,
     782,   786,   742,   788,     0,   658,     0,   880,     0,   879,
     217,     0,   658,   864,   863,   850,   853,     0,     0,   794,
     797,   862,   798,   418,   253,   255,   121,   525,   524,   419,
       0,   121,   235,   130,   422,     0,     0,     0,     0,     0,
     247,   247,   136,     0,     0,     0,     0,   964,   743,     0,
     948,     0,     0,     0,     0,     0,   741,     0,   653,     0,
       0,   691,   652,   696,     0,   690,   119,   695,   941,   977,
       0,     0,     0,     0,    19,     0,    20,     0,    97,     0,
       0,     0,   108,   745,     0,   106,   101,    98,   103,     0,
     298,   306,   303,     0,     0,   911,   916,   913,   912,   915,
     914,    12,   957,   958,     0,     0,     0,     0,   873,   870,
       0,   538,   910,   909,   908,     0,   904,     0,   905,   907,
       0,     5,     0,     0,     0,   553,   554,   562,   561,     0,
     432,     0,   742,   533,   537,     0,     0,   927,     0,   515,
       0,     0,   949,   791,   277,   983,     0,     0,   806,     0,
     855,   742,   944,   940,   293,   294,   651,   744,   290,     0,
     791,     0,     0,   219,   512,   185,   490,     0,   542,   543,
       0,   540,   742,   886,     0,     0,   291,   221,     0,   219,
       0,     0,   217,     0,   865,   439,     0,     0,   803,   804,
     821,   822,   851,   852,     0,     0,     0,   770,   750,   751,
     752,   759,     0,     0,     0,   763,   761,   762,   776,   743,
       0,   784,   884,   883,     0,   219,     0,   808,   673,     0,
     257,     0,     0,   127,     0,     0,     0,     0,     0,     0,
       0,   227,   228,   239,     0,   121,   237,   156,   247,     0,
     247,     0,     0,   978,     0,     0,     0,   742,   965,   967,
     947,   743,   946,     0,   743,   717,   718,   715,   716,   749,
       0,   743,   741,     0,   536,     0,     0,   893,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   970,   173,     0,   176,
     160,     0,     0,   104,   109,   110,   102,   744,   107,     0,
     302,     0,   923,   148,   959,   939,   954,   220,   222,   312,
       0,     0,   871,     0,   903,     0,    17,     0,   926,   218,
     312,     0,     0,   658,   530,     0,   663,   928,     0,   949,
     519,     0,     0,   984,     0,   282,   280,   797,   809,   941,
     797,   810,   943,     0,     0,   295,   118,     0,   791,   216,
       0,   791,     0,   489,   890,   889,     0,   291,     0,     0,
       0,     0,     0,     0,   219,   187,   674,   796,   291,     0,
     755,   756,   757,   758,   764,   765,   774,     0,   743,     0,
     770,     0,   754,   778,   742,   781,   783,   785,     0,   878,
       0,   796,     0,     0,     0,     0,   254,   526,   132,     0,
     422,   227,   229,   873,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   241,     0,     0,   960,     0,   963,   742,
       0,     0,     0,   693,   742,   740,     0,   731,     0,   743,
       0,   697,   732,   730,   897,     0,   743,   700,   702,   701,
       0,     0,   698,   699,   703,   705,   704,   720,   719,   722,
     721,   723,   725,   727,   726,   724,   713,   712,   707,   708,
     706,   709,   710,   711,   714,   969,     0,   121,     0,     0,
     105,    21,   304,     0,     0,     0,   956,     0,   386,   875,
     873,   428,   431,   437,     0,    15,     0,   386,   565,     0,
       0,   567,   560,   563,     0,   558,     0,   930,     0,   950,
     523,     0,   283,     0,     0,   278,     0,   297,   296,   949,
       0,   312,     0,   791,     0,   291,     0,   848,   312,   926,
     312,   929,     0,     0,     0,   440,     0,     0,   767,   742,
     769,   760,     0,   753,     0,     0,   743,   775,   882,   312,
       0,   121,     0,   250,   236,     0,     0,     0,   226,   152,
     240,     0,     0,   243,     0,   248,   249,   121,   242,   979,
     961,     0,   945,     0,   982,   748,   747,   692,     0,   742,
     535,   694,     0,   541,   742,   892,   729,     0,     0,     0,
      22,    23,   953,   951,   952,   223,     0,     0,     0,   393,
     384,     0,     0,     0,   198,   311,   313,     0,   383,     0,
       0,     0,   926,   386,     0,   906,   308,   204,   556,     0,
       0,   529,   517,     0,   286,   276,     0,   279,   285,   291,
     509,   949,   386,   949,     0,   888,     0,   847,   386,     0,
     386,   931,   312,   791,   845,   773,   772,   766,     0,   768,
     742,   777,   386,   121,   256,   128,   133,   154,   230,     0,
     238,   244,   121,   246,   962,     0,     0,   532,     0,   896,
     895,   728,   121,   177,   955,     0,     0,     0,   934,     0,
       0,     0,   224,     0,   926,     0,   349,   345,   351,   653,
      31,     0,   339,     0,   344,   348,   361,     0,   359,   364,
       0,   363,     0,   362,     0,   181,   315,     0,   317,     0,
     318,   319,     0,     0,   872,     0,   557,   555,   566,   564,
     287,     0,     0,   274,   284,     0,     0,     0,     0,   194,
     509,   949,   849,   200,   308,   206,   386,     0,     0,   780,
       0,   202,   252,     0,     0,   121,   233,   153,   245,   981,
     746,     0,     0,     0,     0,     0,   411,     0,   935,     0,
     329,   333,   408,   409,   343,     0,     0,     0,   324,   617,
     616,   613,   615,   614,   634,   636,   635,   605,   576,   577,
     595,   611,   610,   572,   582,   583,   585,   584,   604,   588,
     586,   587,   589,   590,   591,   592,   593,   594,   596,   597,
     598,   599,   600,   601,   603,   602,   573,   574,   575,   578,
     579,   581,   619,   620,   629,   628,   627,   626,   625,   624,
     612,   631,   621,   622,   623,   606,   607,   608,   609,   632,
     633,   637,   639,   638,   640,   641,   618,   643,   642,   645,
     647,   646,   580,   650,   648,   649,   644,   630,   571,   356,
     568,     0,   325,   377,   378,   376,   369,     0,   370,   326,
     403,     0,     0,     0,     0,   407,     0,   181,   190,   307,
       0,     0,     0,   275,   289,   846,     0,   121,   379,   121,
     184,     0,     0,     0,   196,   949,   771,     0,   121,   231,
     134,   155,     0,   531,   894,   175,   327,   328,   406,   225,
       0,     0,   743,     0,   352,   340,     0,     0,     0,   358,
     360,     0,     0,   365,   372,   373,   371,     0,     0,   314,
     936,     0,     0,     0,   410,     0,   309,     0,   288,     0,
     551,   745,     0,     0,   121,   186,   192,     0,   779,     0,
       0,   157,   330,   111,     0,   331,   332,     0,     0,   346,
     742,   354,   350,   355,   569,   570,     0,   341,   374,   375,
     367,   368,   366,   404,   401,   959,   320,   316,   405,     0,
     310,   552,   744,     0,   511,   380,     0,   188,     0,   234,
       0,   179,     0,   386,     0,   353,   357,     0,     0,   791,
     322,     0,   549,   508,   513,   232,     0,     0,   158,   337,
       0,   385,   347,   402,   937,     0,   745,   397,   791,   550,
       0,   178,     0,     0,   336,   949,   791,   261,   398,   399,
     400,   984,   396,     0,     0,     0,   335,     0,   397,     0,
     949,     0,   334,   381,   121,   321,   984,     0,   266,   264,
       0,   121,     0,     0,   267,     0,     0,   262,   323,     0,
     382,     0,   270,   260,     0,   263,   269,   174,   271,     0,
       0,   258,   268,     0,   259,   273,   272
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   861,   602,   170,  1418,   697,
     334,   335,   336,   337,   823,   824,   825,   109,   110,   111,
     112,   113,   388,   634,   635,   524,   239,  1483,   530,  1399,
    1484,  1721,   812,   329,   551,  1681,  1040,  1217,  1738,   405,
     171,   636,   901,  1102,  1274,   117,   605,   918,   637,   656,
     922,   585,   917,   219,   505,   638,   606,   919,   407,   354,
     371,   120,   903,   864,   847,  1057,  1421,  1155,   971,  1630,
    1487,   773,   977,   529,   782,   979,  1307,   765,   960,   963,
    1144,  1745,  1746,   624,   625,   650,   651,   341,   342,   348,
    1455,  1609,  1610,  1228,  1345,  1444,  1603,  1729,  1748,  1640,
    1685,  1686,  1687,  1431,  1432,  1433,  1434,  1642,  1643,  1649,
    1697,  1437,  1438,  1442,  1596,  1597,  1598,  1620,  1775,  1346,
    1347,   172,   122,  1761,  1762,  1601,  1349,  1350,  1351,  1352,
     123,   232,   525,   526,   124,   125,   126,   127,   128,   129,
     130,   131,  1467,   132,   900,  1101,   133,   621,   622,   623,
     236,   380,   520,   611,   612,  1179,   613,  1180,   134,   135,
     136,   803,   137,   138,  1671,   139,   607,  1457,   608,  1071,
     869,  1245,  1242,  1589,  1590,   140,   141,   142,   222,   143,
     223,   233,   392,   512,   144,   999,   807,   145,  1000,   892,
     562,  1001,   946,  1124,   947,  1126,  1127,  1128,   949,  1285,
    1286,   950,   741,   495,   183,   184,   639,   627,   478,  1087,
    1088,   727,   728,   888,   147,   225,   148,   149,   174,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   689,   229,
     230,   588,   212,   213,   692,   693,  1185,  1186,   364,   365,
     855,   160,   576,   161,   620,   162,   321,  1611,  1661,   355,
     400,   645,   646,   993,  1082,  1226,   844,   845,   787,   788,
     789,   322,   323,   809,  1420,   886
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1437
static const yytype_int16 yypact[] =
{
   -1437,   147, -1437, -1437,  5696, 13262, 13262,   -10, 13262, 13262,
   13262, 11128, 13262, -1437, 13262, 13262, 13262, 13262, 13262, 13262,
   13262, 13262, 13262, 13262, 13262, 13262, 15552, 15552, 11322, 13262,
   16007,    -4,     5, -1437, -1437, -1437, -1437, -1437,   246, -1437,
   -1437,   287, 13262, -1437,     5,   194,   196,   299, -1437,     5,
   11516,  1092, 11710, -1437, 14012, 10158,   331, 13262,  1664,   167,
   -1437, -1437, -1437,   286,   484,    65,   337,   357,   370,   373,
   -1437,  1092,   421,   423, -1437, -1437, -1437, -1437, -1437, 13262,
     499,   587, -1437, -1437,  1092, -1437, -1437, -1437, -1437,  1092,
   -1437,  1092, -1437,   391,   425,  1092,  1092, -1437,   313, -1437,
   11904, -1437, -1437,   320,   456,   549,   549, -1437,   584,   478,
       3,   465, -1437,    81, -1437,   607, -1437, -1437, -1437, -1437,
    2175,    41, -1437, -1437,   466,   468,   479,   491,   500,   508,
    4231, -1437, -1437, -1437, -1437,   141, -1437,   648,   651, -1437,
      74,   537, -1437,   593,   211, -1437,  2101,   157, -1437, -1437,
    3090,    73,   552,   155, -1437,   132,   182,   561,   222, -1437,
   -1437,   686, -1437, -1437, -1437,   605,   576,   612, -1437, 13262,
   -1437,   607,    41, 13441,  3726, 13441, 13262, 13441, 13441, 14459,
     589, 15041, 14459,   727,  1092,   714,   714,   136,   714,   714,
     714,   714,   714,   714,   714,   714,   714, -1437, -1437, -1437,
     230, 13262,   617, -1437, -1437,   628,   604,   254,   610,   254,
   15552, 15251,   600,   792, -1437,   605, -1437, 13262,   617, -1437,
     653, -1437,   654,   616, -1437,   142, -1437, -1437, -1437,   254,
      73, 12098, -1437, -1437, 13262,  8994,   804,    92, 13441,  9964,
   -1437, 13262, 13262,  1092, -1437, -1437,  4318,   624, -1437,  4403,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,  3707,
   -1437,  3707, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,    83,    85,
     612, -1437, -1437, -1437, -1437,   635,  2099,    99, -1437, -1437,
     673,   813, -1437,   676, 14597, -1437,   640,  4710, -1437,    44,
    5249,  1074,  1074,  1092,   642,   829,   655, -1437,   355, -1437,
    4732,    96, -1437,   712, -1437,   718, -1437,   833,   100, 15552,
   13262, 13262,   660,   677, -1437, -1437, 15269, 11322,   111,   436,
     545, -1437, 13456, 15552,   510, -1437,  1092, -1437,   215,   478,
   -1437, -1437, -1437, -1437, 16100,   837,   754, -1437, -1437, -1437,
      64, 13262,   666,   667, 13441,   671,  1161,   675,  5890, 13262,
   -1437,   404,   668,   633,   404,   496,   449, -1437,  1092,  3707,
     678, 10352, 14012, -1437, -1437,   709, -1437, -1437, -1437, -1437,
   -1437,   607, -1437, -1437, -1437, -1437, -1437, -1437, -1437, 13262,
   13262, 13262, 12292, 13262, 13262, 13262, 13262, 13262, 13262, 13262,
   13262, 13262, 13262, 13262, 13262, 13262, 13262, 13262, 13262, 13262,
   13262, 13262, 13262, 13262, 13262, 13262, 16193, 13262, -1437, 13262,
   13262, 13262,  5044,  1092,  1092,  1092,  1092,  1092,  2175,   766,
     859,  4905, 13262, 13262, 13262, 13262, 13262, 13262, 13262, 13262,
   13262, 13262, 13262, 13262, -1437, -1437, -1437, -1437,  1229, 13262,
   13262, -1437, 10352, 10352, 13262, 13262, 15269,   688,   607, 12486,
   11113, -1437, 13262, -1437,   691,   871,   732,   701,   705, 13595,
     254, 12680, -1437, 12874, -1437,   616,   715,   720,  2095, -1437,
     297, 10352, -1437,  1241, -1437, -1437, 11501, -1437, -1437, 10546,
   -1437, 13262, -1437,   809,  9188,   892,   724, 16355,   898,    89,
      61, -1437, -1437, -1437,   741, -1437, -1437, -1437,  3707,   723,
     728,   910, 15176,  1092, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437,   734, -1437, -1437,   733,   742,   740,   744,   403,
    2046,  1565, -1437, -1437,  1092,  1092, 13262,   254,   167, -1437,
   -1437, -1437, 15176,   853, -1437,   254,    91,    95,   747,   748,
    2172,   283,   750,   751,   361,   810,   758,   254,   103,   753,
   -1437,  1469,  1092, -1437, -1437,   883,  2500,   388, -1437, -1437,
   -1437,   478, -1437, -1437, -1437,   922,   830,   791,   350,   832,
   13262,   855,   956,   798,   838, -1437,   143, -1437,  3707,  3707,
     977,   804,    64, -1437,   807,   986, -1437,  3707,    75, -1437,
      70,   166, -1437, -1437, -1437, -1437, -1437, -1437, -1437,  1550,
    2621, -1437, -1437, -1437, -1437,   989,   826, -1437, 15552, 13262,
     815,   992, 13441,   990, -1437, -1437,   876,  1059, 11695, 16552,
   14459, 13262, 13247, 16667, 16738, 10332, 10719,  3096, 12463, 12657,
   12657, 12657, 12657,  3499,  3499,  3499,  3499,  3499,  1375,  1375,
     670,   670,   670,   136,   136,   136, -1437,   714, 13441,   812,
     814, 15714,   816,  1001,    -6, 13262,   195,   617,   174, -1437,
   -1437, -1437,   999,   754, -1437,   607, 15366, -1437, -1437, 14459,
   14459, 14459, 14459, 14459, 14459, 14459, 14459, 14459, 14459, 14459,
   14459, 14459, -1437, 13262,   358, -1437,   148, -1437,   617,   414,
     820,  2799,   828,   835,   823,  2845,   105,   831, -1437, 13441,
    2636, -1437,  1092, -1437,    75,    21, 15552, 13441, 15552, 15760,
     876,    75,   254,   153, -1437,   143,   868,   840, 13262, -1437,
     160, -1437, -1437, -1437,  8800,   502, -1437, -1437, 13441, 13441,
       5, -1437, -1437, -1437, 13262,   926, 15059, 15176,  1092,  9382,
     841,   843, -1437,    84,   933,   896,   884, -1437,  1026,   849,
    2340,  3707, 15176, 15176, 15176, 15176, 15176,   854,   886,   860,
   15176,   398,   894, -1437,   866, -1437, 16467, -1437,    14, -1437,
    6084,  1907,   867,  1565, -1437,  1565, -1437,  1092,  1092,  1565,
    1565,  1092, -1437,  1040,   870, -1437,   426, -1437, -1437,  2928,
   -1437, 16467,  1036, 15552,   874, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437,    93,  1092,  1907,   869, 15269, 15459,
    1051, -1437, -1437, -1437, -1437,   872, -1437, 13262, -1437, -1437,
    5308, -1437,  3707,  1907,   878, -1437, -1437, -1437, -1437,  1055,
     885, 13262, 16100, -1437, -1437,  5044,   887, -1437,  3707, -1437,
     890,  6278,  1048,   135, -1437, -1437,   152,  1229, -1437,  1241,
   -1437,  3707, -1437, -1437,   254, 13441, -1437, 10740, -1437, 15176,
     107,   893,  1907,   830, -1437, -1437, 16667, 13262, -1437, -1437,
   13262, -1437, 13262, -1437,  3023,   895, 10352,   810,  1053,   830,
    3707,  1071,   876,  1092, 16193,   254,  3446,   899, -1437, -1437,
     175,   901, -1437, -1437,  1076,  1191,  1191,  2636, -1437, -1437,
   -1437,  1041,   905,   262,   907, -1437, -1437, -1437, -1437,  1087,
     915,   691,   254,   254, 13068,   830,  1241, -1437, -1437,  3548,
     611,     5,  9964, -1437,  6472,   909,  6666,   916, 15059, 15552,
     917,   973,   254, 16467,  1093, -1437, -1437, -1437, -1437,   631,
   -1437,    47,  3707, -1437,   975,  3707,  1092,   723, -1437, -1437,
   -1437,  1101, -1437,   924,   989,   659,   659,  1054,  1054, 15865,
     925,  1109, 15176, 14865, 16100, 12083, 14731, 15176, 15176, 15176,
   15176, 14966, 15176, 15176, 15176, 15176, 15176, 15176, 15176, 15176,
   15176, 15176, 15176, 15176, 15176, 15176, 15176, 15176, 15176, 15176,
   15176, 15176, 15176, 15176, 15176,  1092, -1437, -1437,  1042, -1437,
   -1437,   934,   936, -1437, -1437, -1437,   429,  2046, -1437,   943,
   -1437, 15176,   254, -1437,   361, -1437,   603,  1124, -1437, -1437,
     124,   948,   254, 10934, -1437,  2278, -1437,  5502,   754,  1124,
   -1437,     7,   318, -1437, 13441,  1004,   954, -1437,   953,  1048,
   -1437,  3707,   804,  3707,   277,  1129,  1063,   177, -1437,   617,
     179, -1437, -1437, 15552, 13262, 13441, 16467,   960,   107, -1437,
     959,   107,   964, 16667, 13441, 15819,   969, 10352,   970,   967,
    3707,   971,   974,  3707,   830, -1437,   616,   450, 10352, 13262,
   -1437, -1437, -1437, -1437, -1437, -1437,  1027,   968,  1155,  1077,
    2636,  1017, -1437, 16100,  2636, -1437, -1437, -1437, 15552, 13441,
     978, -1437,     5,  1142,  1113,  9964, -1437, -1437, -1437,  1000,
   13262,   973,   254, 15269, 15059,  1002, 15176,  6860,   713,  1005,
   13262,   115,   349, -1437,  1019,  3707, -1437,  1058, -1437,  3964,
    1167,  1013, 15176, -1437, 15176, -1437,  1016, -1437,  1073,  1198,
    1022, -1437, -1437, -1437, 15924,  1029,  1204, 16510, 16594, 16630,
   15176, 16423,  3402,  5203,  4524, 11300,  4045, 12851, 12851, 12851,
   12851,  3317,  3317,  3317,  3317,  3317,  1414,  1414,   659,   659,
     659,  1054,  1054,  1054,  1054, -1437,  1037, -1437,  1039,  1043,
   -1437, -1437, 16467,  1092,  3707,  3707, -1437,  1907,  1253, -1437,
   15269, -1437, -1437, 14459,  1038, -1437,  1050,  1402, -1437,    63,
   13262, -1437, -1437, -1437, 13262, -1437, 13262, -1437,   804, -1437,
   -1437,   158,  1228,  1163, 13262, -1437,  1060,   254, 13441,  1048,
    1057, -1437,  1062,   107, 13262, 10352,  1065, -1437, -1437,   754,
   -1437, -1437,  1069,  1072,  1078, -1437,  1084,  2636, -1437,  2636,
   -1437, -1437,  1085, -1437,  1128,  1086,  1255, -1437,   254, -1437,
    1236, -1437,  1088, -1437, -1437,  1090,  1091,   126, -1437, -1437,
   16467,  1094,  1096, -1437,  3916, -1437, -1437, -1437, -1437, -1437,
   -1437,  3707, -1437,  3707, -1437, 16467, 15968, -1437, 15176, 16100,
   -1437, -1437, 15176, -1437, 15176, -1437, 16703, 15176,  1097,  7054,
   -1437, -1437,   603, -1437, -1437, -1437,    43, 14151,  1907,  1171,
   -1437,   636,  1115,   735, -1437, -1437, -1437,   766,  3156,   112,
     113,  1099,   754,   859,   127, -1437, -1437, -1437,  1132,  3609,
    3784, 13441, -1437,   285,  1267,  1205, 13262, -1437, 13441, 10352,
    1176,  1048,  1756,  1048,  1111, 13441,  1114, -1437,  1805,  1107,
    1896, -1437, -1437,   107, -1437, -1437,  1168, -1437,  2636, -1437,
   16100, -1437,  1913, -1437,  8800, -1437, -1437, -1437, -1437,  9576,
   -1437, -1437, -1437,  8800, -1437,  1117, 15176, 16467,  1170, 16467,
   16026, 16703, -1437, -1437, -1437,  1907,  1907,  1092, -1437,  1292,
   14865,    79, -1437, 14151,   754,  1746, -1437,  1134, -1437,   114,
    1121,   117, -1437, 14458, -1437, -1437, -1437,   118, -1437, -1437,
    1530, -1437,  1123, -1437,  1232,   607, -1437, 14290, -1437, 14290,
   -1437, -1437,  1304,   766, -1437, 13734, -1437, -1437, -1437, -1437,
    1309,  1233, 13262, -1437, 13441,  1136,  1138,  1144,   519, -1437,
    1176,  1048, -1437, -1437, -1437, -1437,  1981,  1140,  2636, -1437,
    1202, -1437,  8800,  9770,  9576, -1437, -1437, -1437,  8800, -1437,
   16467, 15176, 15176,  7248,  1147,  1148, -1437, 15176, -1437,  1907,
   -1437, -1437, -1437, -1437, -1437,  3707,  2323,   636, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437, -1437,   544,
   -1437,  1115, -1437, -1437, -1437, -1437, -1437,    98,   625, -1437,
    1324,   119, 14597,  1232,  1326, -1437,  3707,   607, -1437, -1437,
    1153,  1330, 13262, -1437, 13441, -1437,   108, -1437, -1437, -1437,
   -1437,  1156,   519, 13873, -1437,  1048, -1437,  2636, -1437, -1437,
   -1437, -1437,  7442, 16467, 16467, -1437, -1437, -1437, 16467, -1437,
    1052,   125,  1338,  1165, -1437, -1437, 15176, 14458, 14458,  1303,
   -1437,  1530,  1530,   650, -1437, -1437, -1437, 15176,  1276, -1437,
    1190,  1179,   120, 15176, -1437,  1092, -1437, 15176, 13441,  1287,
   -1437,  1367,  7636,  7830, -1437, -1437, -1437,   519, -1437,  8024,
    1188,  1266, -1437,  1281,  1230, -1437, -1437,  1282,  3707, -1437,
    2323, -1437, -1437, 16467, -1437, -1437,  1219, -1437,  1350, -1437,
   -1437, -1437, -1437, 16467,  1371,   361, -1437, -1437, 16467,  1203,
   16467, -1437,   138,  1206, -1437, -1437,  8218, -1437,  1208, -1437,
    1212,  1225,  1092,   859,  1235, -1437, -1437, 15176,   131,   123,
   -1437,  1329, -1437, -1437, -1437, -1437,  1907,   867, -1437,  1240,
    1092,   851, -1437, 16467, -1437,  1223,  1405,   702,   123, -1437,
    1335, -1437,  1907,  1243, -1437,  1048,   128, -1437, -1437, -1437,
   -1437,  3707, -1437,  1234,  1246,   121, -1437,   541,   702,   159,
    1048,  1247, -1437, -1437, -1437, -1437,  3707,   366,  1424,  1365,
     541, -1437,  8412,   163,  1434,  1370, 13262, -1437, -1437,  8606,
   -1437,   369,  1438,  1373, 13262, -1437, 13441, -1437,  1441,  1379,
   13262, -1437, 13441, 13262, -1437, 13441, 13441
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1437, -1437, -1437,  -526, -1437, -1437, -1437,   164,    71,   -27,
     410, -1437,  -262,  -483, -1437, -1437,   422,   296,  1596, -1437,
    2891, -1437,   223, -1437,    57, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437, -1437, -1437,  -269, -1437, -1437,  -149,
     106,     4, -1437, -1437, -1437, -1437, -1437, -1437,    28, -1437,
   -1437, -1437, -1437, -1437, -1437,    30, -1437, -1437,  1014,  1018,
    1020,   -92,  -670,  -816,   556,   614,  -273,   332,  -883, -1437,
       0, -1437, -1437, -1437, -1437,  -698,   183, -1437, -1437, -1437,
   -1437,  -260, -1437,  -559, -1437,  -425, -1437, -1437,   918, -1437,
      15, -1437, -1437,  -995, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437,   -17, -1437,    72, -1437, -1437, -1437,
   -1437, -1437,   -99, -1437,   156,  -892, -1437, -1436,  -285, -1437,
    -144,    67,  -100,  -272, -1437,  -105, -1437, -1437, -1437,   165,
     -32,    -3,    39,  -701,   -64, -1437, -1437,   -20, -1437, -1437,
      -5,   -38,    32, -1437, -1437, -1437, -1437, -1437, -1437, -1437,
   -1437, -1437,  -577,  -806, -1437, -1437, -1437, -1437, -1437,   178,
   -1437, -1437, -1437, -1437, -1437,   432, -1437, -1437, -1437, -1437,
   -1437, -1437, -1437, -1437,  -882, -1437,  2383,    26, -1437,  1332,
    -386, -1437, -1437,  -463,  3685,  3612, -1437, -1437,   503,  -164,
    -592, -1437, -1437,   570,   379,  -644,   382, -1437, -1437, -1437,
   -1437, -1437,   562, -1437, -1437, -1437,   102,  -819,  -113,  -396,
    -389, -1437,   630,  -111, -1437, -1437,    35,    37,   569, -1437,
   -1437,  1300,   -21, -1437,  -346,    23,  -326,   173,   191, -1437,
   -1437,  -448,  1158, -1437, -1437, -1437, -1437, -1437,   737,  1580,
   -1437, -1437, -1437,  -323,  -660, -1437,  1125,  -845, -1437,   -65,
    -134,   -15,   736, -1437,  -758,   193,  -177,   475,   547, -1437,
   -1437, -1437, -1437,   497,   -29, -1044
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -969
static const yytype_int16 yytable[] =
{
     173,   175,   459,   177,   178,   179,   181,   182,   116,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   412,   487,   211,   214,   324,   372,   319,   615,   235,
     375,   376,   118,   884,   119,   326,   383,   238,   736,  1251,
     481,   240,   750,   921,   879,   246,   244,   249,   616,   504,
     327,   617,   330,   228,   458,   408,   221,   732,   733,   898,
     686,   114,   385,   880,  1083,   226,  1075,   227,   382,   556,
     558,   121,   412,   967,   238,  1237,   860,   822,   827,   387,
     955,   237,   725,   981,   509,  1151,   757,  1100,  1499,   726,
     402,   780,   -68,   982,   -33,   384,   948,   -68,   778,   -33,
     833,   521,  1054,  1111,   521,   568,   146,  1651,   -32,   573,
     115,    13,   849,   -32,   849,  1358,   513,   760,   358,   514,
     521,  1447,  1449,  -342,   761,   318,  1507,  1591,  1658,  1658,
    1499,   385,  1652,   849,  1688,   849,   849,   382,  -548,  1140,
    1054,   479,  -824,   479,  1669,  1305,   347,     3,   387,    53,
    1160,  1161,   476,   477,    13,  -941,   552,    60,    61,    62,
     163,   164,   409,   498,   384,  1238,  1035,   -95,   108,   497,
      13,   490,  1415,  1416,  1731,    13,   399,   176,  1239,   507,
     387,   -95,    13,   231,   755,   506,  1675,   399,  1084,  1670,
    -941,   446,   234,  -941,  1363,  1777,   384,  1240,  1178,  1791,
    -825,   476,   477,   447,   203,   203,   476,   477,  -839,  -544,
    -866,   484,   384,   460,   553,   247,  -828,  -656,   317,  1732,
     373,  -869,  -658,  -546,   410,  -827,   516,   411,  -868,   516,
     536,  1048,   537,  1085,  -831,   353,   238,   527,  1163,  1364,
    1778,  1717,  -832,  -830,  1792,  -811,   592,  -812,   339,  -281,
    -826,   781,  1114,   370,  -520,   353,   538,  -839,   983,   353,
     353,  -835,   657,   480,  -824,   480,  1372,  1055,  1500,  1501,
     403,  1298,   -68,  1378,   -33,  1380,   390,   779,   488,   834,
    1158,   522,  1162,   835,   353,   569,  1653,   541,   -32,   574,
     484,   850,   518,   934,  1392,  -281,   523,   548,  1273,  1689,
     590,  1448,  1450,  -342,  1306,  1744,  1508,  1592,  1659,  1707,
    1772,  -265,  1229,   579,  1398,  1454,  -744,   496,  1086,   578,
    -744,  1248,  -825,  -744,  1365,  1779,  -663,  1284,  -836,  1793,
     582,   743,  -866,   485,  -662,  1067,  1044,  1045,  -828,   737,
     655,  1097,   483,  -869,  -834,   238,   384,  -827,   494,  1131,
    -868,  -546,   211,  -838,   338,  -547,  -831,   596,  1252,   340,
     412,   916,  -841,  -655,  -832,  -830,  1460,  -811,   -94,  -812,
     647,   398,  -826,   324,  -657,   319,   181,   368,   197,   361,
     369,   241,   -94,   242,   640,   476,   477,  1476,   203,   577,
     372,   702,   703,   408,   377,   318,   652,   707,  -837,   108,
    1060,   197,   866,   108,   603,   604,   343,   528,  1236,   842,
     843,  1132,   485,   344,   658,   659,   660,   662,   663,   664,
     665,   666,   667,   668,   669,   670,   671,   672,   673,   674,
     675,   676,   677,   678,   679,   680,   681,   682,   683,   684,
     685,   708,   687,  1253,   688,   688,   691,  1784,   873,  1295,
    1798,  1461,  1160,  1161,   564,   121,   709,   710,   711,   712,
     713,   714,   715,   716,   717,   718,   719,   720,   721,   228,
     483,   839,   221,   318,   688,   731,  1243,   652,   652,   688,
     735,   226,   626,   227,   709,   359,   243,   739,   547,   103,
    1287,  1108,  1419,  1090,   115,   459,   747,   378,   749,   767,
    1091,  1370,   817,   379,   591,  1250,   652,   867,  1244,   783,
     389,   565,   103,  1408,   768,   887,   769,   889,   203,   476,
     477,   328,   868,   696,   349,   564,   615,   203,   817,   913,
     359,  -799,  1785,   754,   203,  1799,   598,   359,  1116,  1260,
    1308,   203,  1262,   391,   350,  -799,   616,   458,   373,   617,
    -941,  1041,   108,  1042,   362,   363,   923,   351,  1646,   818,
     352,   829,   644,   915,   822,   705,   317,   476,   477,   353,
     318,   399,   870,   150,  1647,   961,   962,   359,  1502,   859,
     359,   772,   398,   598,  1480,   398,   360,  -802,  -941,   876,
     877,   359,   905,  1648,   927,   207,   209,   598,   885,   362,
     363,  -802,  1604,   345,  1605,   384,   362,   363,   356,  1379,
     357,   346,   374,  1468,   397,  1470,   547,   353,   700,   353,
     353,   353,   353,  -800,   988,   698,   359,   338,   338,   559,
     359,   887,   889,  1385,   398,  1386,   394,  -800,   956,   889,
     957,   690,   724,   404,   895,   643,   362,   363,   361,   362,
     363,   729,   401,  1275,  1654,   413,   906,   414,   615,   599,
     362,   363,   601,   547,   203,  1425,    36,   509,   415,   386,
     730,  1362,   698,  1655,  1036,   734,  1656,   759,   616,  1700,
     416,   617,  1266,   756,  1142,  1143,   762,    48,   108,   417,
     914,   642,  1452,  1276,   593,   362,   363,   418,  1701,   362,
     363,  1702,  1224,  1225,  1374,  1297,   460,   808,  1618,  1619,
    1031,  1032,  1033,  1622,   359,    36,   449,  1769,   926,   450,
     598,   443,   444,   445,   626,   446,  1034,   451,   828,   644,
    1773,  1774,  1783,  1159,  1160,  1161,    48,   447,   386,   482,
     764,   555,   557,   366,  1479,   452,    86,    87,  -833,    88,
     168,    90,   784,   959,  -545,   854,   856,  -656,  1426,  1698,
    1699,   647,   647,   486,  1503,  1694,  1695,   965,   366,   238,
     386,  1427,  1428,   493,   810,   991,   994,   367,   491,   500,
     499,   447,  1354,   362,   363,  1135,   508,   615,    36,   167,
     399,  -837,    84,  1429,   502,    86,    87,   483,    88,  1430,
      90,   503,    36,   511,   150,  -654,   510,   616,   150,    48,
     617,   785,   519,   532,    36,  1302,  1160,  1161,  1758,  1759,
    1760,   353,   539,    48,  1477,  -968,   203,   542,   543,   549,
    1171,   121,   560,  1068,  1626,    48,   976,  1175,   561,   570,
    1376,   393,   395,   396,   563,   571,   881,   572,   583,  1078,
     584,   618,  1065,   619,   628,   629,   826,   826,  1115,   630,
    1439,   641,  1092,   632,   116,  -116,  1074,  1677,    86,    87,
     115,    88,   168,    90,    53,   654,   167,   121,   740,    84,
     742,   593,    86,    87,   203,    88,   168,    90,   118,   744,
     119,  1112,  1095,   745,    86,    87,   654,    88,   168,    90,
     770,   521,  1103,   751,   945,  1104,   951,  1105,   752,   567,
    1747,   652,   777,   774,   538,   790,   115,   114,   575,   791,
     580,   811,  1440,   813,   203,   587,   203,   121,   108,  1747,
     815,   814,   597,   816,   832,   836,   837,  1768,   840,   846,
     841,   851,   974,   108,  1465,   848,   696,   228,   121,  1139,
     221,   857,   862,  1164,   203,  1256,  1166,   150,  1145,   226,
     863,   227,   146,   865,  1280,   872,   115,    60,    61,    62,
     163,   164,   409,   615,   108,    60,    61,    62,   163,   164,
     409,  1043,   644,  1678,  -678,   626,   874,   115,   871,   964,
     875,   878,   882,   616,   966,   883,   617,  1767,   891,  1231,
     893,   897,   626,   896,   899,   902,   908,   911,   909,  1056,
     912,   203,  1780,   920,   928,  1320,   930,   932,   904,  1146,
    -660,   984,  1325,   931,   108,   968,   203,   203,   958,   985,
     978,   121,   980,   121,   410,   987,   986,   989,  1003,   547,
    1754,  1002,   410,  1232,   615,   108,  1006,  1004,  1076,  1047,
    1051,   724,  1249,   759,   885,   587,  1007,  1039,  1233,  1059,
     729,  1049,   762,  1053,   616,  1063,  1064,   617,  1070,  1072,
     115,   116,   115,  1073,  1177,  1079,  1077,  1183,  1081,  1713,
    1098,  1269,  1107,  1110,  1272,  1113,  1118,   353,  -840,  1258,
    1119,  1129,  1130,   150,  1133,   118,  1134,   119,  1148,  1123,
    1123,   945,   652,  1136,  1153,  1150,  1154,  1156,  1165,   826,
    1169,   826,  1170,   652,  1233,   826,   826,  1046,  1174,  1173,
     759,  1034,  1391,  1216,   114,  1218,   108,  1219,   108,   762,
     108,    36,  1221,  1227,   121,  1230,  1310,  1246,    36,  1290,
    1092,   916,  1247,  1254,  1255,   238,   203,   203,  1259,  1261,
    1167,  1263,    48,    36,  1757,  1304,  1265,  1268,  1267,    48,
    1277,  1270,  1278,  1271,  1279,   941,  1283,   547,  1289,   146,
     547,    36,  1291,   115,    48,   489,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,  1292,  1294,
    1299,  1311,    48,  1309,  1303,  1333,  1334,  1313,  1157,   808,
     626,  1314,  1293,   626,  1317,   167,  1318,  1319,    84,  1453,
    1321,    86,    87,  1324,    88,   168,    90,   894,    86,    87,
    1323,    88,   168,    90,   121,  1328,   474,   475,  1330,  1355,
     333,   108,  1331,    86,    87,  1359,    88,   168,    90,  1360,
    1356,  1361,  1366,  1682,  1367,  1371,   904,  1369,   412,  1368,
    1373,    86,    87,  1377,    88,   168,    90,  1336,  1381,  1375,
     652,  1388,  1382,   115,  1390,  1383,  1393,  1120,  1121,  1122,
      36,   203,  1384,  1387,  1389,   925,  1436,  1395,  1396,  1397,
    1423,  1462,  1404,  1400,  1405,  1401,  1463,  1412,  1451,  1456,
    1466,    48,   476,   477,   945,  1348,  1602,  1474,   945,  1471,
      13,  1478,  1472,  1491,  1348,  1489,  1497,  1505,    36,   108,
     197,  1506,  1599,  1600,  1613,   952,   203,   953,  1606,  1446,
      36,   108,   197,  1612,  1615,  1616,   208,   208,  1625,    48,
    1353,   203,   203,   150,  1617,  1627,  1636,  1637,  1657,  1353,
    1663,    48,  1666,   826,  1667,   972,  1674,  1690,   150,   631,
      86,    87,  1496,    88,   168,    90,  1692,  1704,   205,   205,
    1696,  1464,  1337,  1705,   652,   626,  1706,  1338,  1711,    60,
      61,    62,   163,  1339,   409,  1340,  1712,  1719,  1720,   150,
    -338,  1723,  1722,  1726,  1652,  1727,   722,  1332,    86,    87,
    1730,    88,   168,    90,  1733,  1737,   121,  1735,   722,  1736,
      86,    87,  1052,    88,   168,    90,  1336,  1752,   203,  1742,
    1749,  1755,  1341,  1342,  1756,  1343,  1764,   587,  1062,   723,
     460,   103,  1770,   440,   441,   442,   443,   444,   445,   150,
     446,   758,  1766,   103,  1771,   115,   410,  1781,  1786,  1348,
    1329,   945,   447,   945,  1344,  1348,  1787,  1348,  1794,    13,
     150,  1795,  1800,  1445,  1801,  1803,  1486,  1614,  1665,  1348,
    1804,   121,  1028,  1029,  1030,  1031,  1032,  1033,  1751,  1220,
     121,   699,   704,  1109,  1353,   701,  1641,  1069,  1691,  1765,
    1353,  1034,  1353,  1296,  1631,   626,   830,  1402,  1763,  1623,
    1645,  1498,  1650,   108,  1353,  1788,  1776,  1504,  1662,  1443,
     115,   317,  1621,  1241,  1424,  1176,  1125,  1441,  1281,   115,
     208,  1337,  1282,  1137,  1394,   589,  1338,  1089,    60,    61,
      62,   163,  1339,   409,  1340,  1414,   992,   653,  1728,  1223,
    1403,   150,  1215,   150,  1168,   150,     0,   972,  1152,     0,
    1629,  1486,   205,  1348,     0,     0,     0,     0,    36,   121,
     852,   853,   945,     0,     0,   121,     0,     0,   108,  1607,
     121,  1341,  1342,   108,  1343,     0,     0,   108,     0,    48,
       0,     0,     0,     0,     0,  1660,     0,  1664,  1353,  1740,
       0,   353,     0,     0,   547,   410,     0,   317,   115,     0,
       0,     0,     0,  1357,   115,     0,     0,  1588,     0,   115,
    1709,     0,     0,     0,  1595,     0,     0,  1668,     0,    36,
       0,   317,     0,   317,     0,     0,  1482,     0,     0,   317,
       0,     0,   202,   202,     0,  1488,   218,     0,    86,    87,
      48,    88,   168,    90,     0,  1493,   150,     0,     0,     0,
     208,   412,   945,     0,    36,     0,   108,   108,   108,   208,
     218,   581,   108,     0,     0,     0,   208,   108,     0,  1724,
       0,     0,  1257,   208,     0,    48,    60,    61,    62,    63,
      64,   409,   205,   318,     0,     0,     0,    70,   453,     0,
       0,   205,     0,     0,     0,     0,     0,  1593,   205,    86,
      87,  1594,    88,   168,    90,   205,     0,     0,     0,   121,
       0,     0,     0,     0,     0,     0,   614,  1288,  1632,     0,
       0,     0,     0,     0,   150,   455,     0,  1440,     0,     0,
       0,   821,   587,   972,    86,    87,   150,    88,   168,    90,
       0,     0,   885,   410,     0,     0,     0,     0,   115,   121,
     121,     0,     0,    36,     0,     0,   121,   885,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1336,     0,     0,     0,    48,     0,   547,     0,     0,     0,
       0,     0,   331,   332,     0,     0,     0,     0,   115,   115,
       0,  1796,     0,   121,     0,   115,   208,   317,     0,  1802,
    1741,   945,     0,     0,     0,  1805,   108,     0,  1806,   587,
       0,     0,     0,    13,  1683,     0,   202,     0,     0,  1336,
       0,  1588,  1588,     0,     0,  1595,  1595,     0,   205,     0,
     333,     0,   115,    86,    87,    36,    88,   168,    90,   353,
       0,   626,     0,     0,     0,     0,   108,   108,     0,     0,
    1672,     0,  1673,   108,     0,     0,    48,     0,     0,   121,
     626,  1679,    13,     0,     0,   218,   121,   218,   626,     0,
       0,     0,     0,     0,     0,  1337,     0,     0,  1426,     0,
    1338,     0,    60,    61,    62,   163,  1339,   409,  1340,     0,
     108,  1427,  1428,     0,     0,     0,  1739,     0,   115,     0,
       0,     0,     0,     0,     0,   115,     0,  1716,   150,   167,
    1336,     0,    84,    85,  1753,    86,    87,     0,    88,  1430,
      90,     0,   218,     0,  1337,  1341,  1342,  1336,  1343,  1338,
       0,    60,    61,    62,   163,  1339,   409,  1340,     0,     0,
       0,     0,     0,     0,     0,     0,   202,     0,     0,   410,
     594,     0,     0,    13,   600,   202,   108,  1469,   208,     0,
       0,     0,   202,   108,     0,     0,     0,     0,     0,   202,
      13,     0,     0,   150,  1341,  1342,     0,  1343,   150,     0,
     218,   594,   150,   600,   594,   600,   600,     0,     0,     0,
     205,     0,     0,     0,     0,  1336,    36,     0,   410,     0,
       0,     0,     0,     0,     0,   218,  1473,  1782,   218,     0,
       0,     0,     0,     0,  1789,  1337,   208,    48,     0,     0,
    1338,     0,    60,    61,    62,   163,  1339,   409,  1340,     0,
       0,     0,  1337,     0,     0,     0,     0,  1338,    13,    60,
      61,    62,   163,  1339,   409,  1340,     0,     0,   205,     0,
       0,     0,   218,     0,     0,     0,   208,     0,   208,     0,
       0,   150,   150,   150,     0,  1341,  1342,   150,  1343,     0,
     167,     0,   150,    84,    85,     0,    86,    87,     0,    88,
     168,    90,  1341,  1342,     0,  1343,   208,     0,   205,   410,
     205,     0,   202,     0,     0,     0,     0,  1475,     0,     0,
    1337,     0,     0,     0,     0,  1338,   410,    60,    61,    62,
     163,  1339,   409,  1340,  1481,     0,     0,     0,   205,   489,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,     0,     0,     0,    36,     0,     0,   259,     0,
       0,     0,     0,   208,   218,   218,     0,     0,   801,     0,
    1341,  1342,     0,  1343,     0,     0,    48,     0,   208,   208,
       0,     0,     0,     0,   819,   820,   261,     0,     0,     0,
     474,   475,     0,     0,   410,   205,     0,     0,   801,     0,
       0,     0,  1624,     0,     0,     0,     0,     0,    36,     0,
     205,   205,     0,     0,     0,     0,   489,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,    48,
       0,   150,   821,     0,   614,    86,    87,   540,    88,   168,
      90,     0,     0,     0,   218,   218,     0,    60,    61,    62,
      63,    64,   409,   218,     0,     0,   476,   477,    70,   453,
       0,     0,     0,     0,   534,   535,     0,   474,   475,     0,
       0,   150,   150,     0,   202,     0,     0,     0,   150,     0,
       0,     0,   167,     0,    36,    84,   311,     0,    86,    87,
       0,    88,   168,    90,   454,     0,   455,     0,   208,   208,
       0,     0,     0,     0,     0,    48,   315,     0,     0,   456,
       0,   457,     0,   753,   410,   150,   316,     0,   419,   420,
     421,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     205,   205,   202,   476,   477,     0,     0,   422,     0,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   406,   446,    86,    87,   614,    88,   168,    90,
       0,     0,   202,     0,   202,   447,     0,     0,     0,     0,
       0,   150,     0,     0,     0,     0,     0,     0,   150,     0,
     838,     0,     0,     0,     0,     0,     0,     0,     0,   259,
       0,     0,   202,   801,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   218,   218,   801,   801,
     801,   801,   801,   208,     0,     0,   801,   261,     0,    33,
      34,    35,     0,     0,     0,     0,     0,   218,     0,   204,
     204,   198,     0,   220,     0,     0,     0,     0,     0,    36,
       0,     0,     0,     0,     0,   205,     0,     0,     0,   202,
       0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
      48,     0,   218,     0,   202,   202,     0,     0,     0,     0,
       0,     0,     0,   208,   208,     0,     0,     0,   218,   218,
      74,    75,    76,    77,    78,   614,     0,     0,   218,     0,
     205,   200,  1234,     0,   218,   534,   535,    82,    83,     0,
       0,     0,     0,     0,     0,   205,   205,   218,     0,     0,
       0,    92,     0,   167,     0,   801,    84,   311,   218,    86,
      87,     0,    88,   168,    90,    97,   990,     0,     0,     0,
     419,   420,   421,     0,     0,     0,   218,   315,     0,     0,
     218,     0,     0,     0,     0,     0,     0,   316,     0,   422,
     208,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,     0,   446,     0,     0,     0,     0,
       0,     0,   205,     0,   202,   202,     0,   447,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   218,     0,
       0,   218,     0,   218,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   204,     0,     0,     0,     0,   801,     0,
     218,     0,     0,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   419,   420,   421,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   801,     0,     0,
     422,   614,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,     0,   446,   218,     0,   218,
       0,     0,     0,     0,   935,   936,     0,     0,   447,   202,
       0,   858,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   937,     0,   218,     0,     0,   218,
       0,     0,   938,   939,   940,    36,     0,     0,     0,     0,
       0,     0,   614,   204,   941,     0,     0,     0,     0,   218,
       0,     0,   204,     0,   202,     0,    48,     0,     0,   204,
       0,     0,     0,     0,     0,     0,   204,     0,     0,   202,
     202,     0,   801,     0,     0,     0,     0,   204,     0,     0,
       0,   218,     0,     0,     0,   218,     0,     0,   801,     0,
     801,   942,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   943,     0,   801,     0,     0,     0,
       0,     0,     0,     0,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,     0,     0,     0,     0,     0,   419,
     420,   421,   890,   944,     0,     0,     0,     0,     0,     0,
     218,   218,     0,   218,     0,     0,   202,     0,   422,   220,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,     0,   446,   419,   420,   421,     0,     0,
       0,     0,     0,     0,     0,     0,   447,     0,     0,   204,
       0,     0,     0,     0,   422,     0,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,     0,
     446,     0,     0,     0,     0,     0,     0,   218,     0,   218,
       0,     0,   447,     0,   801,   218,     0,     0,   801,     0,
     801,     0,     0,   801,     0,   804,     0,     0,     0,     0,
       0,     0,     0,   218,   218,     0,     0,   218,   419,   420,
     421,     0,     0,     0,   218,   320,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   804,     0,   422,     0,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,     0,   446,     0,     0,   218,     0,     0,     0,
     929,     0,     0,     0,     0,   447,     0,     0,     0,     0,
       0,     0,   801,     0,     0,     0,     0,     0,     0,     0,
       0,   218,   218,     0,     0,     0,     0,     0,     0,   218,
       0,   218,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   204,     0,   419,   420,   421,   933,     0,     0,     0,
       0,     0,     0,   218,     0,   218,     0,     0,     0,     0,
       0,   218,   422,     0,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,     0,   446,     0,
       0,     0,     0,     0,     0,     0,     0,   801,   801,   204,
     447,     0,     0,   801,     0,   218,     0,     0,     0,     0,
       0,   218,     0,   218,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,     0,     0,  1050,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   204,
       0,   204,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     320,   446,   320,     0,     0,   474,   475,     0,     0,   204,
     804,     0,     0,   447,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   804,   804,   804,   804,   804,
       0,     0,     0,   804,     0,   259,     0,     0,     0,     0,
       0,     0,     0,     0,  1038,     0,     0,     0,     0,     0,
       0,     0,   218,     0,     0,     0,     0,   320,     0,     0,
       0,     0,     0,   261,  1106,     0,   204,     0,     0,   218,
       0,   476,   477,     0,     0,     0,     0,     0,     0,  1058,
       0,   204,   204,     0,     0,    36,   218,     0,     0,     0,
       0,     0,   801,     0,     0,     0,  1058,     0,     0,     0,
       0,     0,     0,   801,     0,   204,    48,     0,     0,   801,
       0,     0,     0,   801,  -385,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   163,   164,   409,     0,     0,
       0,     0,   804,     0,   218,  1099,     0,     0,     0,     0,
     320,   534,   535,   320,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   220,     0,   167,
       0,     0,    84,   311,     0,    86,    87,     0,    88,   168,
      90,     0,     0,   801,     0,     0,     0,     0,     0,     0,
       0,     0,   218,   315,     0,     0,     0,     0,     0,   410,
       0,     0,     0,   316,     0,     0,     0,     0,   218,     0,
       0,   204,   204,     0,     0,     0,     0,   218,  -969,  -969,
    -969,  -969,  -969,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,     0,   218,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1034,   804,     0,   204,     0,     0,
     804,   804,   804,   804,   804,   804,   804,   804,   804,   804,
     804,   804,   804,   804,   804,   804,   804,   804,   804,   804,
     804,   804,   804,   804,   804,   804,   804,   804,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   320,
     786,     0,     0,   802,   804,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,  1032,  1033,   419,   420,   421,     0,
       0,     0,     0,   802,     0,     0,     0,     0,     0,  1034,
       0,     0,     0,     0,     0,   422,   204,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
       0,   446,     0,     0,     0,     0,     0,     0,     0,   320,
     320,     0,     0,   447,     0,     0,   204,     0,   320,     0,
       0,   204,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   204,   204,     0,   804,
    -969,  -969,  -969,  -969,  -969,   438,   439,   440,   441,   442,
     443,   444,   445,     0,   446,   804,     0,   804,   419,   420,
     421,     0,     0,     0,     0,     0,   447,     0,     0,     0,
       0,     0,     0,   804,     0,     0,     0,   422,     0,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,     0,   446,     0,     0,     0,     0,     0,     0,
    1335,     0,     0,   204,     0,   447,     0,     0,     0,   419,
     420,   421,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1117,   422,     0,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,     0,   446,     0,     0,     0,   802,     0,
       0,     0,     0,     0,     0,     0,   447,     0,     0,     0,
       0,   320,   320,   802,   802,   802,   802,   802,     0,     0,
       0,   802,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   804,   204,     0,     0,   804,     0,   804,     0,     0,
     804,   206,   206,     0,     0,   224,     0,     0,     0,     0,
       0,  1422,     0,     0,  1435,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   259,     0,     0,  1141,
     489,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   320,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   261,     0,     0,     0,     0,   320,
       0,     0,     0,   204,     0,     0,     0,     0,     0,     0,
       0,     0,   320,     0,     0,     0,    36,     0,     0,   804,
     802,   474,   475,     0,   419,   420,   421,     0,  1494,  1495,
    1458,     0,     0,     0,     0,     0,     0,    48,  1435,     0,
       0,   320,     0,   422,     0,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,     0,   446,
       0,     0,   534,   535,     0,     0,     0,     0,     0,     0,
       0,   447,     0,     0,     0,     0,     0,   476,   477,     0,
     167,     0,     0,    84,   311,     0,    86,    87,     0,    88,
     168,    90,     0,   320,   804,   804,   320,     0,   786,     0,
     804,     0,  1639,     0,   315,     0,     0,     0,     0,     0,
    1435,     0,     0,   802,   316,   206,     0,     0,   802,   802,
     802,   802,   802,   802,   802,   802,   802,   802,   802,   802,
     802,   802,   802,   802,   802,   802,   802,   802,   802,   802,
     802,   802,   802,   802,   802,   802,   419,   420,   421,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   802,     0,     0,   422,  1305,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
       0,   446,   320,     0,   320,  1459,     0,     0,     0,     0,
       0,     0,     0,   447,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   259,     0,     0,     0,     0,     0,     0,
       0,   320,     0,     0,   320,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   261,     0,     0,     0,   206,     0,     0,     0,   804,
       0,     0,     0,     0,   206,     0,     0,     0,     0,     0,
     804,   206,     0,    36,     0,     0,   804,   802,   206,     0,
     804,     0,     0,     0,     0,     0,   320,     0,     0,   224,
     320,     0,     0,   802,    48,   802,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   802,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,   534,
     535,     0,     0,     0,     0,  1306,     0,     0,     0,     0,
     804,     0,  1034,     0,     0,   320,   320,   167,     0,  1750,
      84,   311,     0,    86,    87,     0,    88,   168,    90,     0,
    1312,   224,     0,     0,     0,  1422,     0,     0,     0,     0,
       0,   315,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   316,     0,     0,   806,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   831,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   320,     0,   320,     0,     0,     0,     0,   802,
       0,     0,     0,   802,     0,   802,     0,     0,   802,     0,
       0,     0,     0,     0,     0,     0,     0,   805,   320,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   320,
       0,   419,   420,   421,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   805,     0,     0,
     422,     0,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,     0,   446,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   802,   447,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   320,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   419,   420,
     421,     0,     0,   206,     0,     0,     0,     0,   320,     0,
     320,     0,     0,     0,     0,     0,   320,   422,     0,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,     0,   446,     0,     0,     0,     0,     0,     0,
       0,     0,   802,   802,     0,   447,     0,     0,   802,   973,
       0,   206,     0,     0,     0,     0,   320,     0,     0,     0,
       0,     0,     0,     0,   995,   996,   997,   998,     0,     0,
       0,     0,  1005,   419,   420,   421,     0,     0,     0,     0,
     448,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   206,   422,   206,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,     0,   446,     0,
       0,   206,   805,     0,     0,     0,     0,     0,     0,     0,
     447,     0,     0,     0,     0,     0,     0,   805,   805,   805,
     805,   805,     0,     0,     0,   805,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   320,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   531,     0,     0,
       0,  1096,     0,     0,   320,     0,     0,     0,   206,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1684,     0,   206,   206,     0,     0,   802,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   802,     0,
       0,     0,     0,     0,   802,     0,     0,   224,   802,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,     0,   320,
       0,     0,     0,     0,   805,     0,     0,     0,     0,     0,
       0,  1034,   533,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   224,
       0,     0,     0,     0,     0,     0,     0,     0,   802,  1184,
    1187,  1188,  1189,  1191,  1192,  1193,  1194,  1195,  1196,  1197,
    1198,  1199,  1200,  1201,  1202,  1203,  1204,  1205,  1206,  1207,
    1208,  1209,  1210,  1211,  1212,  1213,  1214,     0,     0,     0,
       0,     0,   320,   206,   206,     0,     0,     0,     0,     0,
       0,     0,     0,  1222,     0,     0,     0,   320,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   805,     0,   224,
       0,     0,   805,   805,   805,   805,   805,   805,   805,   805,
     805,   805,   805,   805,   805,   805,   805,   805,   805,   805,
     805,   805,   805,   805,   805,   805,   805,   805,   805,   805,
     419,   420,   421,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   805,     0,     0,   422,
       0,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,     0,   446,     0,     0,  1300,     0,
       0,     0,     0,     0,     0,     0,     0,   447,   206,     0,
       0,     0,     0,     0,  1315,     0,  1316,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      28,     0,  1326,     0,     0,     0,     0,     0,    33,    34,
      35,    36,     0,   197,     0,     0,     0,     0,   224,     0,
     198,     0,     0,   206,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,   206,   206,
       0,   805,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   199,     0,     0,     0,   805,     0,   805,
       0,     0,     0,     0,     0,     0,     0,    73,     0,    74,
      75,    76,    77,    78,     0,   805,     0,     0,     0,     0,
     200,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,     0,     0,   550,
      92,     0,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     9,     0,    97,   206,     0,     0,    10,   201,
       0,     0,   566,     0,   103,     0,     0,     0,     0,     0,
    1407,     0,   381,    12,  1409,     0,  1410,     0,     0,  1411,
       0,   706,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   805,   224,    48,     0,   805,     0,   805,
       0,     0,   805,    53,     0,     0,     0,     0,  1490,     0,
       0,    60,    61,    62,   163,   164,   165,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   166,
      73,     0,    74,    75,    76,    77,    78,   250,   251,     0,
     252,   253,     0,    80,   254,   255,   256,   257,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
       0,   258,     0,    92,     0,   224,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,   805,   100,     0,     0,     0,     0,   103,   104,   260,
     105,   106,     0,  1633,  1634,     0,     0,     0,     0,  1638,
       0,     0,     0,   262,   263,   264,   265,   266,   267,   268,
       0,     0,     0,    36,     0,   197,     0,     0,     0,     0,
       0,     0,     0,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,    48,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,     0,     0,
       0,   694,   304,   305,   306,     0,   805,   805,   307,   544,
     545,     0,   805,     0,     0,     0,     0,     0,     0,     0,
       0,  1644,     0,     0,     0,     0,     0,   546,     0,     0,
       0,     0,     0,    86,    87,     0,    88,   168,    90,   312,
       0,   313,     0,     0,   314,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   695,     0,   103,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,     0,  1693,   419,
     420,   421,     0,     0,     0,     0,     0,     0,     0,  1703,
    1034,     0,     0,     0,     0,  1708,     0,     0,   422,  1710,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,     0,   446,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,   447,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   805,     0,     0,     0,    11,    12,     0,     0,  1743,
       0,     0,   805,     0,     0,     0,     0,     0,   805,     0,
       0,     0,   805,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,  1725,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,   805,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,    69,    70,    71,     0,   554,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,    81,    82,    83,    84,    85,     0,    86,    87,     0,
      88,    89,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,    95,     0,    96,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1066,
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
       0,   101,   102,  1235,   103,   104,     0,   105,   106,     5,
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
       0,    50,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,    81,
      82,    83,    84,    85,     0,    86,    87,     0,    88,    89,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,    95,     0,    96,     0,    97,    98,
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
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,   633,   103,   104,     0,   105,   106,     5,     6,     7,
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
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1037,   103,   104,     0,   105,
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
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1080,
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
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,  1147,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,  1149,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
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
      43,    44,     0,    45,     0,    46,     0,    47,  1301,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,    91,     0,     0,    92,     0,
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
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1413,   103,   104,     0,   105,
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
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1635,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,  1680,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,    91,     0,     0,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,  1714,   103,   104,
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
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,  1715,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,  1718,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,    91,
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
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1734,
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
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,  1790,   103,   104,     0,   105,   106,     5,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,  1797,   103,   104,
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
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,   517,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   163,   164,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,   771,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,   163,   164,    65,
       0,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   975,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,   163,   164,    65,     0,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,  1485,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,   163,   164,    65,     0,    66,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
    1628,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,   163,
     164,    65,     0,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   163,   164,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   163,   164,   165,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   166,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   169,     0,   325,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,     0,   446,   648,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   447,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   163,   164,   165,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   166,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,     0,   649,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   169,
       0,     0,     0,     0,   103,   104,     0,   105,   106,     5,
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
       0,     0,    60,    61,    62,   163,   164,   165,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     166,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   169,     0,     0,   766,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,     0,   446,     0,  1093,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   447,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   163,
     164,   165,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   166,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,     0,  1094,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   169,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   381,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   163,   164,   165,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   166,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   419,   420,   421,   103,   104,     0,   105,
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
       0,     0,     0,   180,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   163,   164,   165,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   166,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,     0,     0,    93,
       0,   738,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   169,     0,     0,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,     0,     0,     0,     0,   210,     0,
       0,     0,     0,     0,     0,     0,     0,  1034,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   163,   164,   165,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   166,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   169,
       0,   419,   420,   421,   103,   104,     0,   105,   106,     5,
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
       0,     0,    60,    61,    62,   163,   164,   165,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     166,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,    92,     0,     0,    93,     0,   763,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   169,     0,   245,   420,   421,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   422,     0,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,     0,
     446,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   447,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   163,
     164,   165,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   166,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   169,     0,   248,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   381,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   163,   164,   165,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   166,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,  1008,  1009,  1010,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1011,     0,     0,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
    1034,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   163,   164,   165,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   166,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,     0,     0,    93,
       0,  1181,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   169,   515,     0,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   661,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   163,   164,   165,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   166,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   169,
       0,     0,     0,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,     0,   446,     0,
       0,     0,   706,     0,     0,     0,     0,     0,     0,     0,
     447,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   163,   164,   165,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     166,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   169,     0,     0,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  -969,  -969,  -969,  -969,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,     0,   446,     0,     0,     0,   746,     0,     0,     0,
       0,     0,     0,     0,   447,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   163,
     164,   165,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   166,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   169,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  -969,  -969,
    -969,  -969,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,     0,     0,     0,     0,     0,
     748,     0,     0,     0,     0,     0,     0,     0,  1034,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   163,   164,   165,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   166,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   169,     0,     0,     0,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1138,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   163,   164,   165,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   166,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   169,     0,   419,   420,   421,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   422,   907,   423,   424,
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
      62,   163,   164,   165,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   166,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   169,
       0,   419,   420,   421,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     422,     0,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,     0,   446,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   447,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,   595,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   163,   164,   165,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     166,    73,     0,    74,    75,    76,    77,    78,   250,   251,
       0,   252,   253,     0,    80,   254,   255,   256,   257,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,     0,   258,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   169,     0,     0,     0,     0,   103,   104,
     260,   105,   106,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   262,   263,   264,   265,   266,   267,
     268,     0,     0,     0,    36,     0,   197,     0,     0,     0,
       0,     0,     0,     0,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    48,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,     0,
       0,     0,   303,   304,   305,   306,     0,     0,     0,   307,
     544,   545,     0,     0,     0,     0,     0,   250,   251,     0,
     252,   253,     0,     0,   254,   255,   256,   257,   546,     0,
       0,     0,     0,     0,    86,    87,     0,    88,   168,    90,
     312,   258,   313,   259,     0,   314,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   695,     0,   103,     0,   260,
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
       0,   315,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   316,     0,     0,     0,  1608,     0,     0,   260,     0,
     261,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   262,   263,   264,   265,   266,   267,   268,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,    48,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,     0,     0,     0,
       0,   304,   305,   306,     0,     0,     0,   307,   308,   309,
       0,     0,     0,     0,     0,   250,   251,     0,   252,   253,
       0,     0,   254,   255,   256,   257,   310,     0,     0,    84,
     311,     0,    86,    87,     0,    88,   168,    90,   312,   258,
     313,   259,     0,   314,     0,     0,     0,     0,     0,     0,
     315,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     316,     0,     0,     0,  1676,     0,     0,   260,     0,   261,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   262,   263,   264,   265,   266,   267,   268,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,    48,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,     0,     0,     0,   303,
     304,   305,   306,     0,     0,     0,   307,   308,   309,     0,
       0,     0,     0,     0,   250,   251,     0,   252,   253,     0,
       0,   254,   255,   256,   257,   310,     0,     0,    84,   311,
       0,    86,    87,     0,    88,   168,    90,   312,   258,   313,
     259,     0,   314,     0,     0,     0,     0,     0,     0,   315,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   316,
       0,     0,     0,     0,     0,     0,   260,     0,   261,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     262,   263,   264,   265,   266,   267,   268,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,    48,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,     0,     0,     0,     0,   304,
     305,   306,     0,     0,     0,   307,   308,   309,     0,     0,
       0,     0,     0,   250,   251,     0,   252,   253,     0,     0,
     254,   255,   256,   257,   310,     0,     0,    84,   311,     0,
      86,    87,     0,    88,   168,    90,   312,   258,   313,   259,
       0,   314,     0,     0,     0,     0,     0,     0,   315,  1417,
       0,     0,     0,     0,     0,     0,     0,     0,   316,     0,
       0,     0,     0,     0,     0,   260,     0,   261,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   262,
     263,   264,   265,   266,   267,   268,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
      48,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,     0,     0,     0,     0,   304,   305,
     306,     0,     0,     0,   307,   308,   309,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   310,     0,     0,    84,   311,     0,    86,
      87,     0,    88,   168,    90,   312,     0,   313,     0,     0,
     314,  1509,  1510,  1511,  1512,  1513,     0,   315,  1514,  1515,
    1516,  1517,     0,     0,     0,     0,     0,   316,     0,     0,
       0,     0,     0,     0,     0,  1518,  1519,     0,   422,     0,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,  1520,   446,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   447,  1521,  1522,  1523,
    1524,  1525,  1526,  1527,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1528,  1529,  1530,
    1531,  1532,  1533,  1534,  1535,  1536,  1537,  1538,    48,  1539,
    1540,  1541,  1542,  1543,  1544,  1545,  1546,  1547,  1548,  1549,
    1550,  1551,  1552,  1553,  1554,  1555,  1556,  1557,  1558,  1559,
    1560,  1561,  1562,  1563,  1564,  1565,  1566,  1567,  1568,     0,
       0,     0,  1569,  1570,     0,  1571,  1572,  1573,  1574,  1575,
     250,   251,     0,   252,   253,     0,     0,   254,   255,   256,
     257,  1576,  1577,  1578,     0,     0,     0,    86,    87,     0,
      88,   168,    90,  1579,   258,  1580,  1581,     0,  1582,     0,
       0,     0,     0,     0,     0,  1583,  1584,     0,  1585,     0,
    1586,  1587,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   260,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   262,   263,   264,   265,
     266,   267,   268,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,    48,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,     0,     0,     0,   303,   304,   305,   306,     0,     0,
       0,   307,   544,   545,   250,   251,     0,   252,   253,     0,
       0,   254,   255,   256,   257,     0,     0,     0,     0,     0,
     546,     0,     0,     0,     0,     0,    86,    87,   258,    88,
     168,    90,   312,     0,   313,     0,     0,   314,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   260,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     262,   263,   264,   265,   266,   267,   268,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,    48,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,     0,     0,     0,  1182,   304,
     305,   306,     0,     0,     0,   307,   544,   545,   250,   251,
       0,   252,   253,     0,     0,   254,   255,   256,   257,     0,
       0,     0,     0,     0,   546,     0,     0,     0,     0,     0,
      86,    87,   258,    88,   168,    90,   312,     0,   313,     0,
       0,   314,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     260,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   262,   263,   264,   265,   266,   267,
     268,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    48,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,     0,
       0,     0,     0,   304,   305,   306,  1190,     0,     0,   307,
     544,   545,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   792,   793,     0,     0,   546,     0,
     794,     0,   795,     0,    86,    87,     0,    88,   168,    90,
     312,     0,   313,     0,   796,   314,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,     0,     0,     0,     0,
       0,   419,   420,   421,   198,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
     422,     0,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   969,   446,     0,     0,     0,
       0,   797,     0,    74,    75,    76,    77,    78,   447,     0,
       0,     0,     0,     0,   200,     0,     0,     0,     0,   167,
      82,    83,    84,   798,     0,    86,    87,    28,    88,   168,
      90,     0,     0,     0,    92,    33,    34,    35,    36,     0,
     197,     0,     0,   799,     0,     0,     0,   198,    97,     0,
       0,     0,     0,   800,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   492,     0,     0,     0,     0,     0,
     199,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   970,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   200,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,   792,   793,     0,    92,     0,     0,
     794,     0,   795,     0,     0,     0,     0,     0,     0,     0,
       0,    97,     0,     0,   796,     0,   201,     0,     0,     0,
       0,   103,    33,    34,    35,    36,     0,     0,     0,     0,
       0,   419,   420,   421,   198,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
     422,     0,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,     0,   446,     0,     0,     0,
       0,   797,     0,    74,    75,    76,    77,    78,   447,     0,
       0,     0,     0,     0,   200,     0,     0,     0,     0,   167,
      82,    83,    84,   798,     0,    86,    87,    28,    88,   168,
      90,     0,     0,     0,    92,    33,    34,    35,    36,     0,
     197,     0,     0,   799,     0,     0,     0,   198,    97,     0,
       0,     0,     0,   800,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   501,     0,     0,     0,     0,     0,
     199,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   586,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   200,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,    28,     0,   924,    92,     0,     0,
       0,     0,    33,    34,    35,    36,     0,   197,     0,     0,
       0,    97,     0,     0,   198,     0,   201,     0,     0,     0,
       0,   103,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   199,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   200,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,    28,    88,   168,
      90,     0,     0,     0,    92,    33,    34,    35,    36,     0,
     197,     0,     0,     0,     0,     0,     0,   198,    97,     0,
       0,     0,     0,   201,     0,     0,     0,     0,   103,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     199,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1061,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   200,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
      28,    88,   168,    90,     0,     0,     0,    92,    33,    34,
      35,    36,     0,   197,     0,     0,     0,     0,     0,     0,
     198,    97,     0,     0,     0,     0,   201,     0,     0,     0,
       0,   103,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   199,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     200,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,     0,     0,     0,
      92,     0,     0,     0,   419,   420,   421,     0,     0,     0,
       0,     0,     0,     0,    97,     0,     0,     0,     0,   201,
       0,     0,     0,   422,   103,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,     0,   446,
     419,   420,   421,     0,     0,     0,     0,     0,     0,     0,
       0,   447,     0,     0,     0,     0,     0,     0,     0,   422,
       0,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,     0,   446,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   447,     0,   419,
     420,   421,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   910,   422,     0,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,     0,   446,  1008,  1009,  1010,     0,     0,
       0,     0,     0,     0,     0,     0,   447,     0,     0,     0,
       0,     0,     0,   954,  1011,     0,     0,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1034,     0,  1008,  1009,  1010,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1264,  1011,     0,     0,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1008,  1009,
    1010,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1034,     0,     0,     0,     0,     0,  1011,  1172,     0,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1034,  1008,  1009,  1010,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1011,     0,  1322,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
       0,     0,     0,    33,    34,    35,    36,     0,   197,     0,
       0,     0,     0,  1034,     0,   198,     0,     0,     0,     0,
       0,  1406,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,     0,   216,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,  1492,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   197,     0,     0,     0,     0,     0,     0,   609,    97,
       0,     0,     0,     0,   217,     0,     0,     0,     0,   103,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   199,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   200,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,     0,     0,     0,    92,    33,
      34,    35,    36,     0,   197,     0,     0,     0,     0,     0,
       0,   198,    97,     0,     0,     0,     0,   610,     0,     0,
       0,     0,   103,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,     0,     0,
       0,    92,     0,     0,     0,   419,   420,   421,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     217,     0,     0,   775,   422,   103,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,     0,
     446,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   447,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1008,  1009,  1010,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1011,  1327,   776,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1008,  1009,  1010,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1034,     0,     0,     0,     0,     0,  1011,     0,     0,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1009,  1010,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1034,     0,     0,     0,     0,  1011,
       0,     0,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,   421,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1034,     0,     0,
       0,   422,     0,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,  1010,   446,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   447,
       0,     0,     0,  1011,     0,     0,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1011,
       0,  1034,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1032,  1033,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1034,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,     0,   446,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   447,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1034,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,     0,   446,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   447
};

static const yytype_int16 yycheck[] =
{
       5,     6,   146,     8,     9,    10,    11,    12,     4,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   121,   171,    28,    29,    54,    91,    54,   374,    32,
      95,    96,     4,   625,     4,    55,   100,    42,   486,  1083,
     151,    44,   505,   703,   621,    50,    49,    52,   374,   213,
      55,   374,    57,    30,   146,   120,    30,   482,   483,   651,
     446,     4,   100,   622,   883,    30,   872,    30,   100,   331,
     332,     4,   172,   774,    79,  1070,   602,   560,   561,   100,
     750,    42,   478,   781,   218,   968,   511,   903,     9,   478,
       9,    30,     9,     9,     9,   100,   740,    14,     9,    14,
       9,     9,     9,   919,     9,     9,     4,     9,     9,     9,
       4,    47,     9,    14,     9,    52,   229,   513,    79,   230,
       9,     9,     9,     9,   513,    54,     9,     9,     9,     9,
       9,   169,    34,     9,     9,     9,     9,   169,    68,   955,
       9,    68,    68,    68,    36,    30,    81,     0,   169,   108,
     103,   104,   131,   132,    47,   152,   112,   116,   117,   118,
     119,   120,   121,   201,   169,   158,   152,   173,     4,   201,
      47,   176,   129,   130,    36,    47,   173,   187,   171,   217,
     201,   187,    47,   187,   510,   217,  1622,   173,    36,    81,
     187,    55,   187,   190,    36,    36,   201,   190,  1004,    36,
      68,   131,   132,    67,    26,    27,   131,   132,   187,    68,
      68,    68,   217,   146,   170,    51,    68,   152,    54,    81,
     157,    68,   152,    68,   183,    68,   231,   121,    68,   234,
     259,   823,   261,    81,    68,    71,   241,   242,   191,    81,
      81,  1677,    68,    68,    81,    68,   359,    68,    81,   185,
      68,   190,   922,    89,     8,    91,   173,   187,   174,    95,
      96,   187,   411,   190,   190,   190,  1261,   174,   189,   190,
     189,  1154,   189,  1268,   189,  1270,   103,   188,   172,   188,
     978,   189,   980,   188,   120,   189,   188,   316,   189,   189,
      68,   188,   235,   188,  1289,   188,   239,   324,  1114,   174,
     189,   189,   189,   189,   189,   174,   189,   189,   189,   189,
     189,   188,   188,   351,   188,   188,   188,    87,   166,   351,
     185,  1079,   190,   188,   166,   166,   152,  1133,   187,   166,
     351,   495,   190,   190,   152,   861,   819,   820,   190,   488,
     405,   900,   187,   190,   187,   350,   351,   190,   184,    87,
     190,    68,   357,   187,    58,    68,   190,   362,    81,   192,
     460,   187,   187,   152,   190,   190,    81,   190,   173,   190,
     399,   156,   190,   402,   152,   402,   381,    81,    81,   149,
      84,   187,   187,   187,   389,   131,   132,  1382,   210,   350,
     455,   456,   457,   458,    81,   324,   401,   461,   187,   235,
     848,    81,    52,   239,   189,   190,   120,   243,  1068,    48,
      49,   149,   190,   127,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   461,   447,   166,   449,   450,   451,    81,   612,  1150,
      81,   166,   103,   104,    99,   388,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   446,
     187,   188,   446,   402,   479,   480,   158,   482,   483,   484,
     485,   446,   380,   446,   489,    81,   187,   492,   324,   192,
    1134,   916,  1337,   889,   388,   639,   501,   184,   503,   519,
     889,  1259,    99,   190,    68,  1082,   511,   157,   190,   538,
     190,   156,   192,  1319,   519,   628,   521,   630,   340,   131,
     132,   190,   172,   452,   187,    99,   872,   349,    99,   693,
      81,   173,   166,   510,   356,   166,    87,    81,   924,  1098,
     191,   363,  1101,    87,   187,   187,   872,   639,   157,   872,
     152,   813,   388,   815,   150,   151,   705,   187,    14,   156,
     187,   566,   398,   697,  1047,   459,   402,   131,   132,   405,
     499,   173,   610,     4,    30,    73,    74,    81,  1423,   191,
      81,   524,   156,    87,  1390,   156,    87,   173,   190,   618,
     619,    81,   657,    49,   728,    26,    27,    87,   627,   150,
     151,   187,  1447,   119,  1449,   610,   150,   151,   187,  1269,
     187,   127,   187,  1371,    30,  1373,   452,   453,   454,   455,
     456,   457,   458,   173,   788,   452,    81,   331,   332,   333,
      81,   744,   745,  1277,   156,  1279,    87,   187,   751,   752,
     751,   450,   478,    36,   649,   196,   150,   151,   149,   150,
     151,   478,   187,  1116,    29,   189,   661,   189,  1004,   149,
     150,   151,   366,   499,   486,    29,    79,   801,   189,   100,
     479,  1248,   499,    48,   808,   484,    51,   513,  1004,    29,
     189,  1004,  1107,   510,    73,    74,   513,   100,   524,   189,
     695,   195,  1352,  1118,   149,   150,   151,   189,    48,   150,
     151,    51,    99,   100,  1263,  1153,   639,   543,   189,   190,
      51,    52,    53,  1471,    81,    79,    68,  1761,   723,    68,
      87,    51,    52,    53,   622,    55,    67,   190,   564,   565,
     189,   190,  1776,   102,   103,   104,   100,    67,   169,   187,
     517,   331,   332,   156,  1388,   152,   159,   160,   187,   162,
     163,   164,    29,   758,    68,   591,   592,   152,   122,  1651,
    1652,   790,   791,   187,  1424,  1647,  1648,   770,   156,   774,
     201,   135,   136,    46,   551,   790,   791,   190,   189,   210,
     152,    67,  1230,   150,   151,   949,   217,  1133,    79,   153,
     173,   187,   156,   157,   194,   159,   160,   187,   162,   163,
     164,     9,    79,   187,   235,   152,   152,  1133,   239,   100,
    1133,    88,     8,   189,    79,   102,   103,   104,   116,   117,
     118,   657,   187,   100,  1383,   152,   648,    14,   152,   189,
     994,   764,   190,   862,  1478,   100,   779,  1001,     9,   127,
    1265,   104,   105,   106,   189,   127,   623,    14,   188,   878,
     173,    14,   857,    99,   188,   188,   560,   561,   923,   188,
     125,   193,   891,   188,   860,   187,   871,  1625,   159,   160,
     764,   162,   163,   164,   108,   187,   153,   810,   187,   156,
       9,   149,   159,   160,   706,   162,   163,   164,   860,   188,
     860,   920,   897,   188,   159,   160,   187,   162,   163,   164,
      91,     9,   907,   188,   740,   910,   742,   912,   188,   340,
    1729,   916,    14,   189,   173,   187,   810,   860,   349,     9,
     351,   187,   187,   190,   746,   356,   748,   860,   764,  1748,
     190,   189,   363,   189,    81,   188,   188,  1756,   188,   129,
     189,   188,   778,   779,  1369,   187,   875,   924,   881,   954,
     924,    68,    30,   982,   776,  1089,   985,   388,   961,   924,
     130,   924,   860,   172,  1128,     9,   860,   116,   117,   118,
     119,   120,   121,  1319,   810,   116,   117,   118,   119,   120,
     121,   817,   818,  1627,   152,   883,   188,   881,   133,   766,
     152,    14,   185,  1319,   771,     9,  1319,  1755,     9,  1063,
     174,     9,   900,   188,    14,   129,   194,   191,   194,   845,
       9,   833,  1770,    14,   194,  1179,   188,   194,   187,   962,
     152,    88,  1186,   188,   860,    99,   848,   849,   188,   133,
     189,   964,   189,   966,   183,     9,   152,   188,   152,   875,
     189,   187,   183,  1063,  1390,   881,   152,   187,   875,     9,
      14,   887,  1081,   889,  1083,   486,   190,   190,  1063,   190,
     887,   191,   889,   189,  1390,    14,   194,  1390,   190,    14,
     964,  1067,   966,   188,  1003,   185,   189,  1006,    30,  1671,
     187,  1110,   187,    30,  1113,    14,   187,   923,   187,  1094,
      14,    50,   187,   524,   187,  1067,     9,  1067,   189,   935,
     936,   937,  1107,   188,   187,   189,   133,    14,   133,   813,
       9,   815,   188,  1118,  1119,   819,   820,   821,     9,   194,
     956,    67,  1286,    81,  1067,   191,   962,   191,   964,   956,
     966,    79,   189,     9,  1067,   187,  1165,   133,    79,  1142,
    1169,   187,   189,    14,    81,  1150,   968,   969,   188,   190,
     986,   187,   100,    79,  1746,  1160,   187,   190,   188,   100,
     133,   190,   194,   189,     9,    88,   149,  1003,   190,  1067,
    1006,    79,    30,  1067,   100,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    75,   189,
     188,   133,   100,   174,   189,  1224,  1225,    30,   975,  1035,
    1098,   188,  1145,  1101,   188,   153,   133,     9,   156,  1353,
     188,   159,   160,     9,   162,   163,   164,   648,   159,   160,
     191,   162,   163,   164,  1157,   188,    65,    66,   189,   191,
     156,  1067,   189,   159,   160,  1240,   162,   163,   164,  1244,
     190,  1246,    14,   191,    81,   188,   187,   187,  1348,  1254,
     188,   159,   160,   188,   162,   163,   164,     4,   189,  1264,
    1265,   133,   190,  1157,     9,   187,    30,    76,    77,    78,
      79,  1093,   188,   188,   188,   706,   161,   189,   188,   188,
     109,    14,  1311,   189,  1313,   189,    81,   190,   189,   157,
     114,   100,   131,   132,  1130,  1228,  1445,   190,  1134,   188,
      47,   133,   188,   133,  1237,   188,    14,   173,    79,  1145,
      81,   190,   189,    81,    81,   746,  1138,   748,    14,  1348,
      79,  1157,    81,    14,   188,   187,    26,    27,   188,   100,
    1228,  1153,  1154,   764,   190,   133,   189,   189,    14,  1237,
      14,   100,   189,  1047,    14,   776,   190,     9,   779,   188,
     159,   160,  1417,   162,   163,   164,   191,    81,    26,    27,
      57,  1366,   109,   173,  1369,  1263,   187,   114,    81,   116,
     117,   118,   119,   120,   121,   122,     9,   189,   112,   810,
      99,    99,   152,   164,    34,    14,   157,  1223,   159,   160,
     187,   162,   163,   164,   188,   170,  1329,   189,   157,   187,
     159,   160,   833,   162,   163,   164,     4,   167,  1230,   174,
      81,   188,   159,   160,     9,   162,    81,   848,   849,   190,
    1353,   192,   188,    48,    49,    50,    51,    52,    53,   860,
      55,   190,   189,   192,   188,  1329,   183,   190,    14,  1372,
    1217,  1277,    67,  1279,   191,  1378,    81,  1380,    14,    47,
     881,    81,    14,  1347,    81,    14,  1399,  1462,  1607,  1392,
      81,  1394,    48,    49,    50,    51,    52,    53,  1737,  1047,
    1403,   453,   458,   917,  1372,   455,  1505,   863,  1642,  1752,
    1378,    67,  1380,  1151,  1484,  1383,   568,  1304,  1748,  1474,
    1507,  1420,  1591,  1329,  1392,  1780,  1768,  1425,  1603,  1343,
    1394,  1337,  1470,  1071,  1339,  1002,   936,  1343,  1129,  1403,
     210,   109,  1130,   951,  1291,   357,   114,   887,   116,   117,
     118,   119,   120,   121,   122,  1332,   790,   402,  1705,  1054,
    1307,   962,  1035,   964,   987,   966,    -1,   968,   969,    -1,
    1483,  1484,   210,  1476,    -1,    -1,    -1,    -1,    79,  1482,
      81,    82,  1388,    -1,    -1,  1488,    -1,    -1,  1394,  1453,
    1493,   159,   160,  1399,   162,    -1,    -1,  1403,    -1,   100,
      -1,    -1,    -1,    -1,    -1,  1602,    -1,  1606,  1476,  1723,
      -1,  1417,    -1,    -1,  1420,   183,    -1,  1423,  1482,    -1,
      -1,    -1,    -1,   191,  1488,    -1,    -1,  1433,    -1,  1493,
    1665,    -1,    -1,    -1,  1440,    -1,    -1,  1612,    -1,    79,
      -1,  1447,    -1,  1449,    -1,    -1,  1393,    -1,    -1,  1455,
      -1,    -1,    26,    27,    -1,  1402,    30,    -1,   159,   160,
     100,   162,   163,   164,    -1,  1412,  1067,    -1,    -1,    -1,
     340,  1741,  1478,    -1,    79,    -1,  1482,  1483,  1484,   349,
      54,   351,  1488,    -1,    -1,    -1,   356,  1493,    -1,  1688,
      -1,    -1,  1093,   363,    -1,   100,   116,   117,   118,   119,
     120,   121,   340,  1602,    -1,    -1,    -1,   127,   128,    -1,
      -1,   349,    -1,    -1,    -1,    -1,    -1,   157,   356,   159,
     160,   161,   162,   163,   164,   363,    -1,    -1,    -1,  1632,
      -1,    -1,    -1,    -1,    -1,    -1,   374,  1138,  1485,    -1,
      -1,    -1,    -1,    -1,  1145,   165,    -1,   187,    -1,    -1,
      -1,   156,  1153,  1154,   159,   160,  1157,   162,   163,   164,
      -1,    -1,  1761,   183,    -1,    -1,    -1,    -1,  1632,  1672,
    1673,    -1,    -1,    79,    -1,    -1,  1679,  1776,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       4,    -1,    -1,    -1,   100,    -1,  1602,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,    -1,    -1,    -1,  1672,  1673,
      -1,  1786,    -1,  1716,    -1,  1679,   486,  1623,    -1,  1794,
    1723,  1627,    -1,    -1,    -1,  1800,  1632,    -1,  1803,  1230,
      -1,    -1,    -1,    47,  1640,    -1,   210,    -1,    -1,     4,
      -1,  1647,  1648,    -1,    -1,  1651,  1652,    -1,   486,    -1,
     156,    -1,  1716,   159,   160,    79,   162,   163,   164,  1665,
      -1,  1729,    -1,    -1,    -1,    -1,  1672,  1673,    -1,    -1,
    1617,    -1,  1619,  1679,    -1,    -1,   100,    -1,    -1,  1782,
    1748,  1628,    47,    -1,    -1,   259,  1789,   261,  1756,    -1,
      -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,   122,    -1,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,    -1,
    1716,   135,   136,    -1,    -1,    -1,  1722,    -1,  1782,    -1,
      -1,    -1,    -1,    -1,    -1,  1789,    -1,  1674,  1329,   153,
       4,    -1,   156,   157,  1740,   159,   160,    -1,   162,   163,
     164,    -1,   316,    -1,   109,   159,   160,     4,   162,   114,
      -1,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   340,    -1,    -1,   183,
     360,    -1,    -1,    47,   364,   349,  1782,   191,   648,    -1,
      -1,    -1,   356,  1789,    -1,    -1,    -1,    -1,    -1,   363,
      47,    -1,    -1,  1394,   159,   160,    -1,   162,  1399,    -1,
     374,   391,  1403,   393,   394,   395,   396,    -1,    -1,    -1,
     648,    -1,    -1,    -1,    -1,     4,    79,    -1,   183,    -1,
      -1,    -1,    -1,    -1,    -1,   399,   191,  1774,   402,    -1,
      -1,    -1,    -1,    -1,  1781,   109,   706,   100,    -1,    -1,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,   109,    -1,    -1,    -1,    -1,   114,    47,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,   706,    -1,
      -1,    -1,   446,    -1,    -1,    -1,   746,    -1,   748,    -1,
      -1,  1482,  1483,  1484,    -1,   159,   160,  1488,   162,    -1,
     153,    -1,  1493,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,   159,   160,    -1,   162,   776,    -1,   746,   183,
     748,    -1,   486,    -1,    -1,    -1,    -1,   191,    -1,    -1,
     109,    -1,    -1,    -1,    -1,   114,   183,   116,   117,   118,
     119,   120,   121,   122,   191,    -1,    -1,    -1,   776,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    79,    -1,    -1,    29,    -1,
      -1,    -1,    -1,   833,   538,   539,    -1,    -1,   542,    -1,
     159,   160,    -1,   162,    -1,    -1,   100,    -1,   848,   849,
      -1,    -1,    -1,    -1,   108,   109,    57,    -1,    -1,    -1,
      65,    66,    -1,    -1,   183,   833,    -1,    -1,   572,    -1,
      -1,    -1,   191,    -1,    -1,    -1,    -1,    -1,    79,    -1,
     848,   849,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   100,
      -1,  1632,   156,    -1,   872,   159,   160,   108,   162,   163,
     164,    -1,    -1,    -1,   618,   619,    -1,   116,   117,   118,
     119,   120,   121,   627,    -1,    -1,   131,   132,   127,   128,
      -1,    -1,    -1,    -1,   135,   136,    -1,    65,    66,    -1,
      -1,  1672,  1673,    -1,   648,    -1,    -1,    -1,  1679,    -1,
      -1,    -1,   153,    -1,    79,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,   163,    -1,   165,    -1,   968,   969,
      -1,    -1,    -1,    -1,    -1,   100,   177,    -1,    -1,   178,
      -1,   180,    -1,   188,   183,  1716,   187,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     968,   969,   706,   131,   132,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,   157,    55,   159,   160,  1004,   162,   163,   164,
      -1,    -1,   746,    -1,   748,    67,    -1,    -1,    -1,    -1,
      -1,  1782,    -1,    -1,    -1,    -1,    -1,    -1,  1789,    -1,
     188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    -1,   776,   777,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   790,   791,   792,   793,
     794,   795,   796,  1093,    -1,    -1,   800,    57,    -1,    76,
      77,    78,    -1,    -1,    -1,    -1,    -1,   811,    -1,    26,
      27,    88,    -1,    30,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,  1093,    -1,    -1,    -1,   833,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1138,    -1,
     100,    -1,   846,    -1,   848,   849,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1153,  1154,    -1,    -1,    -1,   862,   863,
     137,   138,   139,   140,   141,  1133,    -1,    -1,   872,    -1,
    1138,   148,   194,    -1,   878,   135,   136,   154,   155,    -1,
      -1,    -1,    -1,    -1,    -1,  1153,  1154,   891,    -1,    -1,
      -1,   168,    -1,   153,    -1,   899,   156,   157,   902,   159,
     160,    -1,   162,   163,   164,   182,   166,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,   920,   177,    -1,    -1,
     924,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    29,
    1230,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,  1230,    -1,   968,   969,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   982,    -1,
      -1,   985,    -1,   987,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   210,    -1,    -1,    -1,    -1,  1002,    -1,
    1004,    -1,    -1,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,
    1034,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1051,    -1,    -1,
      29,  1319,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,  1081,    -1,  1083,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    67,  1093,
      -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,  1110,    -1,    -1,  1113,
      -1,    -1,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,  1390,   340,    88,    -1,    -1,    -1,    -1,  1133,
      -1,    -1,   349,    -1,  1138,    -1,   100,    -1,    -1,   356,
      -1,    -1,    -1,    -1,    -1,    -1,   363,    -1,    -1,  1153,
    1154,    -1,  1156,    -1,    -1,    -1,    -1,   374,    -1,    -1,
      -1,  1165,    -1,    -1,    -1,  1169,    -1,    -1,  1172,    -1,
    1174,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,  1190,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,   160,    -1,   162,   163,
     164,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,   191,   177,    -1,    -1,    -1,    -1,    -1,    -1,
    1224,  1225,    -1,  1227,    -1,    -1,  1230,    -1,    29,   446,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,   486,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,  1311,    -1,  1313,
      -1,    -1,    67,    -1,  1318,  1319,    -1,    -1,  1322,    -1,
    1324,    -1,    -1,  1327,    -1,   542,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1337,  1338,    -1,    -1,  1341,    10,    11,
      12,    -1,    -1,    -1,  1348,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   572,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,  1390,    -1,    -1,    -1,
     191,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,  1406,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1415,  1416,    -1,    -1,    -1,    -1,    -1,    -1,  1423,
      -1,  1425,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   648,    -1,    10,    11,    12,   191,    -1,    -1,    -1,
      -1,    -1,    -1,  1447,    -1,  1449,    -1,    -1,    -1,    -1,
      -1,  1455,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1491,  1492,   706,
      67,    -1,    -1,  1497,    -1,  1499,    -1,    -1,    -1,    -1,
      -1,  1505,    -1,  1507,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,   191,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   746,
      -1,   748,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
     259,    55,   261,    -1,    -1,    65,    66,    -1,    -1,   776,
     777,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   792,   793,   794,   795,   796,
      -1,    -1,    -1,   800,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   811,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1606,    -1,    -1,    -1,    -1,   316,    -1,    -1,
      -1,    -1,    -1,    57,   191,    -1,   833,    -1,    -1,  1623,
      -1,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   846,
      -1,   848,   849,    -1,    -1,    79,  1640,    -1,    -1,    -1,
      -1,    -1,  1646,    -1,    -1,    -1,   863,    -1,    -1,    -1,
      -1,    -1,    -1,  1657,    -1,   872,   100,    -1,    -1,  1663,
      -1,    -1,    -1,  1667,   108,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   116,   117,   118,   119,   120,   121,    -1,    -1,
      -1,    -1,   899,    -1,  1688,   902,    -1,    -1,    -1,    -1,
     399,   135,   136,   402,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   924,    -1,   153,
      -1,    -1,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,    -1,    -1,  1727,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1736,   177,    -1,    -1,    -1,    -1,    -1,   183,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,  1752,    -1,
      -1,   968,   969,    -1,    -1,    -1,    -1,  1761,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,  1776,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,  1002,    -1,  1004,    -1,    -1,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   538,
     539,    -1,    -1,   542,  1051,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,   572,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    29,  1093,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   618,
     619,    -1,    -1,    67,    -1,    -1,  1133,    -1,   627,    -1,
      -1,  1138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1153,  1154,    -1,  1156,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,  1172,    -1,  1174,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,  1190,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
    1227,    -1,    -1,  1230,    -1,    67,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   191,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,    -1,   777,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,   790,   791,   792,   793,   794,   795,   796,    -1,    -1,
      -1,   800,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1318,  1319,    -1,    -1,  1322,    -1,  1324,    -1,    -1,
    1327,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,  1338,    -1,    -1,  1341,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,   191,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,   862,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,   878,
      -1,    -1,    -1,  1390,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   891,    -1,    -1,    -1,    79,    -1,    -1,  1406,
     899,    65,    66,    -1,    10,    11,    12,    -1,  1415,  1416,
     191,    -1,    -1,    -1,    -1,    -1,    -1,   100,  1425,    -1,
      -1,   920,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     153,    -1,    -1,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,    -1,   982,  1491,  1492,   985,    -1,   987,    -1,
    1497,    -1,  1499,    -1,   177,    -1,    -1,    -1,    -1,    -1,
    1507,    -1,    -1,  1002,   187,   210,    -1,    -1,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1051,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,  1081,    -1,  1083,   191,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1110,    -1,    -1,  1113,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,   340,    -1,    -1,    -1,  1646,
      -1,    -1,    -1,    -1,   349,    -1,    -1,    -1,    -1,    -1,
    1657,   356,    -1,    79,    -1,    -1,  1663,  1156,   363,    -1,
    1667,    -1,    -1,    -1,    -1,    -1,  1165,    -1,    -1,   374,
    1169,    -1,    -1,  1172,   100,  1174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1190,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,   135,
     136,    -1,    -1,    -1,    -1,   189,    -1,    -1,    -1,    -1,
    1727,    -1,    67,    -1,    -1,  1224,  1225,   153,    -1,  1736,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
     166,   446,    -1,    -1,    -1,  1752,    -1,    -1,    -1,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   187,    -1,    -1,   542,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   486,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   572,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1311,    -1,  1313,    -1,    -1,    -1,    -1,  1318,
      -1,    -1,    -1,  1322,    -1,  1324,    -1,    -1,  1327,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   542,  1337,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1348,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   572,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1406,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1423,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,   648,    -1,    -1,    -1,    -1,  1447,    -1,
    1449,    -1,    -1,    -1,    -1,    -1,  1455,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1491,  1492,    -1,    67,    -1,    -1,  1497,   777,
      -1,   706,    -1,    -1,    -1,    -1,  1505,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   792,   793,   794,   795,    -1,    -1,
      -1,    -1,   800,    10,    11,    12,    -1,    -1,    -1,    -1,
     189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   746,    29,   748,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    -1,
      -1,   776,   777,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,   792,   793,   794,
     795,   796,    -1,    -1,    -1,   800,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1606,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   189,    -1,    -1,
      -1,   899,    -1,    -1,  1623,    -1,    -1,    -1,   833,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1640,    -1,   848,   849,    -1,    -1,  1646,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1657,    -1,
      -1,    -1,    -1,    -1,  1663,    -1,    -1,   872,  1667,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,  1688,
      -1,    -1,    -1,    -1,   899,    -1,    -1,    -1,    -1,    -1,
      -1,    67,   189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   924,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1727,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,  1032,  1033,  1034,    -1,    -1,    -1,
      -1,    -1,  1761,   968,   969,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1051,    -1,    -1,    -1,  1776,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1002,    -1,  1004,
      -1,    -1,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1051,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,  1156,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,  1093,    -1,
      -1,    -1,    -1,    -1,  1172,    -1,  1174,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,  1190,    -1,    -1,    -1,    -1,    -1,    76,    77,
      78,    79,    -1,    81,    -1,    -1,    -1,    -1,  1133,    -1,
      88,    -1,    -1,  1138,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,  1153,  1154,
      -1,  1156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,    -1,    -1,    -1,  1172,    -1,  1174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,    -1,   137,
     138,   139,   140,   141,    -1,  1190,    -1,    -1,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,   189,
     168,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,    -1,   182,  1230,    -1,    -1,    13,   187,
      -1,    -1,   190,    -1,   192,    -1,    -1,    -1,    -1,    -1,
    1318,    -1,    27,    28,  1322,    -1,  1324,    -1,    -1,  1327,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1318,  1319,   100,    -1,  1322,    -1,  1324,
      -1,    -1,  1327,   108,    -1,    -1,    -1,    -1,  1406,    -1,
      -1,   116,   117,   118,   119,   120,   121,    -1,    -1,   124,
     125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
     135,    -1,   137,   138,   139,   140,   141,     3,     4,    -1,
       6,     7,    -1,   148,    10,    11,    12,    13,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,    27,    -1,   168,    -1,  1390,   171,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,  1406,   187,    -1,    -1,    -1,    -1,   192,   193,    55,
     195,   196,    -1,  1491,  1492,    -1,    -1,    -1,    -1,  1497,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    79,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    -1,    -1,
      -1,   127,   128,   129,   130,    -1,  1491,  1492,   134,   135,
     136,    -1,  1497,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1506,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,
      -1,    -1,    -1,   159,   160,    -1,   162,   163,   164,   165,
      -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   190,    -1,   192,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,  1646,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1657,
      67,    -1,    -1,    -1,    -1,  1663,    -1,    -1,    29,  1667,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    67,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1646,    -1,    -1,    -1,    27,    28,    -1,    -1,  1727,
      -1,    -1,  1657,    -1,    -1,    -1,    -1,    -1,  1663,    -1,
      -1,    -1,  1667,    -1,    -1,    47,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,  1690,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    76,    77,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,   101,
      -1,    -1,  1727,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,   120,   121,
      -1,   123,   124,   125,   126,   127,   128,    -1,   189,    -1,
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
      -1,   105,   106,   107,   108,   109,   110,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,   120,   121,    -1,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,   145,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,   165,    -1,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,   178,    -1,   180,    -1,   182,   183,
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
      84,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    72,    -1,    -1,    -1,    76,    77,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,
      90,    91,    -1,    93,    -1,    95,    -1,    97,    98,    -1,
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
      88,    89,    90,    91,    -1,    93,    -1,    95,    96,    97,
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
      -1,    -1,    88,    89,    90,    91,    -1,    93,    94,    95,
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
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
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
     182,   183,   184,    -1,    -1,   187,    -1,   189,    -1,    -1,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
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
      -1,   159,   160,    -1,   162,   163,   164,    -1,   166,    -1,
     168,    -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,
      -1,    -1,    -1,    -1,   192,   193,    -1,   195,   196,     3,
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
     184,    -1,    -1,   187,    -1,    -1,   190,    -1,   192,   193,
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
     160,    -1,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   105,    -1,    -1,   108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   116,   117,   118,   119,   120,   121,
      -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,
      -1,   153,   154,   155,   156,   157,    -1,   159,   160,    -1,
     162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,
      -1,   188,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
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
     164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,   188,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,   189,    11,    12,   192,   193,
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
      -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,
      -1,    -1,   192,   193,    -1,   195,   196,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   187,    -1,    10,    11,    12,   192,   193,    -1,   195,
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
      -1,   188,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,    -1,    -1,   187,   188,    -1,    -1,    -1,
     192,   193,    -1,   195,   196,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   192,   193,    -1,   195,   196,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    -1,
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
     184,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,
      -1,   195,   196,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    48,    49,
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
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,
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
      -1,   187,    -1,    -1,    -1,    -1,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
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
     134,   135,    -1,   137,   138,   139,   140,   141,     3,     4,
      -1,     6,     7,    -1,   148,    10,    11,    12,    13,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,    -1,    27,    -1,   168,    -1,    -1,   171,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,
     184,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,
      55,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    79,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,   127,   128,   129,   130,    -1,    -1,    -1,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   153,    -1,
      -1,    -1,    -1,    -1,   159,   160,    -1,   162,   163,   164,
     165,    27,   167,    29,    -1,   170,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   190,    -1,   192,    -1,    55,
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
      -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,   191,    -1,    -1,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
      -1,   128,   129,   130,    -1,    -1,    -1,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,   153,    -1,    -1,   156,
     157,    -1,   159,   160,    -1,   162,   163,   164,   165,    27,
     167,    29,    -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,   191,    -1,    -1,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,    -1,    -1,    -1,   127,
     128,   129,   130,    -1,    -1,    -1,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,   153,    -1,    -1,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,   165,    27,   167,
      29,    -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    57,    -1,
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
      -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,   177,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    74,    75,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,    -1,    -1,    -1,    -1,   128,   129,
     130,    -1,    -1,    -1,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   153,    -1,    -1,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,   165,    -1,   167,    -1,    -1,
     170,     3,     4,     5,     6,     7,    -1,   177,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    55,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    69,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,    -1,
      -1,    -1,   134,   135,    -1,   137,   138,   139,   140,   141,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,   153,   154,   155,    -1,    -1,    -1,   159,   160,    -1,
     162,   163,   164,   165,    27,   167,   168,    -1,   170,    -1,
      -1,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,    -1,
     182,   183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,    -1,    -1,    -1,   127,   128,   129,   130,    -1,    -1,
      -1,   134,   135,   136,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     153,    -1,    -1,    -1,    -1,    -1,   159,   160,    27,   162,
     163,   164,   165,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    -1,    -1,    -1,   127,   128,
     129,   130,    -1,    -1,    -1,   134,   135,   136,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,    -1,    -1,
     159,   160,    27,   162,   163,   164,   165,    -1,   167,    -1,
      -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,    -1,    -1,   128,   129,   130,    30,    -1,    -1,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,   153,    -1,
      54,    -1,    56,    -1,   159,   160,    -1,   162,   163,   164,
     165,    -1,   167,    -1,    68,   170,    -1,    -1,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    88,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    36,    55,    -1,    -1,    -1,
      -1,   135,    -1,   137,   138,   139,   140,   141,    67,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    68,   162,   163,
     164,    -1,    -1,    -1,   168,    76,    77,    78,    79,    -1,
      81,    -1,    -1,   177,    -1,    -1,    -1,    88,   182,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,    -1,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,    48,    49,    -1,   168,    -1,    -1,
      54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,    -1,    -1,    68,    -1,   187,    -1,    -1,    -1,
      -1,   192,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    88,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,   135,    -1,   137,   138,   139,   140,   141,    67,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    68,   162,   163,
     164,    -1,    -1,    -1,   168,    76,    77,    78,    79,    -1,
      81,    -1,    -1,   177,    -1,    -1,    -1,    88,   182,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,    -1,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,    68,    -1,    70,   168,    -1,    -1,
      -1,    -1,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,   182,    -1,    -1,    88,    -1,   187,    -1,    -1,    -1,
      -1,   192,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    68,   162,   163,
     164,    -1,    -1,    -1,   168,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,   182,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      68,   162,   163,   164,    -1,    -1,    -1,   168,    76,    77,
      78,    79,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      88,   182,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      -1,   192,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,    -1,   137,
     138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,
     168,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    -1,    29,   192,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,   133,    29,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,    29,    -1,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    29,   133,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,   133,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    -1,    67,    -1,    88,    -1,    -1,    -1,    -1,
      -1,   133,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    30,    99,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    29,
      -1,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    12,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    29,    -1,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    67,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67
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
     438,   440,   442,   119,   120,   121,   134,   153,   163,   187,
     204,   237,   318,   337,   415,   337,   187,   337,   337,   337,
     105,   337,   337,   401,   402,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,    81,    88,   121,
     148,   187,   215,   356,   373,   376,   381,   415,   418,   415,
      36,   337,   429,   430,   337,   121,   127,   187,   215,   250,
     373,   374,   375,   377,   381,   412,   413,   414,   422,   426,
     427,   187,   328,   378,   187,   328,   347,   329,   337,   223,
     328,   187,   187,   187,   328,   189,   337,   204,   189,   337,
       3,     4,     6,     7,    10,    11,    12,    13,    27,    29,
      55,    57,    69,    70,    71,    72,    73,    74,    75,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   127,   128,   129,   130,   134,   135,   136,
     153,   157,   165,   167,   170,   177,   187,   204,   205,   206,
     217,   443,   458,   459,   461,   189,   334,   337,   190,   230,
     337,   108,   109,   156,   207,   208,   209,   210,   214,    81,
     192,   284,   285,   120,   127,   119,   127,    81,   286,   187,
     187,   187,   187,   204,   256,   446,   187,   187,   329,    81,
      87,   149,   150,   151,   435,   436,   156,   190,   214,   214,
     204,   257,   446,   157,   187,   446,   446,    81,   184,   190,
     348,    27,   327,   331,   337,   338,   415,   419,   219,   190,
     424,    87,   379,   435,    87,   435,   435,    30,   156,   173,
     447,   187,     9,   189,    36,   236,   157,   255,   446,   121,
     183,   237,   319,   189,   189,   189,   189,   189,   189,    10,
      11,    12,    29,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    55,    67,   189,    68,
      68,   190,   152,   128,   163,   165,   178,   180,   258,   317,
     318,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    65,    66,   131,   132,   405,    68,
     190,   410,   187,   187,    68,   190,   187,   236,   237,    14,
     337,   189,   133,    46,   204,   400,    87,   327,   338,   152,
     415,   133,   194,     9,   386,   251,   327,   338,   415,   447,
     152,   187,   380,   405,   410,   188,   337,    30,   221,     8,
     349,     9,   189,   221,   222,   329,   330,   337,   204,   270,
     225,   189,   189,   189,   135,   136,   461,   461,   173,   187,
     108,   461,    14,   152,   135,   136,   153,   204,   206,   189,
     189,   231,   112,   170,   189,   207,   209,   207,   209,   214,
     190,     9,   387,   189,    99,   156,   190,   415,     9,   189,
     127,   127,    14,     9,   189,   415,   439,   329,   327,   338,
     415,   418,   419,   188,   173,   248,   134,   415,   428,   429,
     189,    68,   405,   149,   436,    80,   337,   415,    87,   149,
     436,   214,   203,   189,   190,   243,   253,   363,   365,    88,
     187,   350,   351,   353,   376,   421,   423,   440,    14,    99,
     441,   344,   345,   346,   280,   281,   403,   404,   188,   188,
     188,   188,   188,   191,   220,   221,   238,   245,   252,   403,
     337,   193,   195,   196,   204,   448,   449,   461,    36,   166,
     282,   283,   337,   443,   187,   446,   246,   236,   337,   337,
     337,    30,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   377,   337,   337,   425,
     425,   337,   431,   432,   127,   190,   205,   206,   424,   256,
     204,   257,   446,   446,   255,   237,    36,   331,   334,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   157,   190,   204,   406,   407,   408,   409,   424,
     425,   337,   282,   282,   425,   337,   428,   236,   188,   337,
     187,   399,     9,   386,   188,   188,    36,   337,    36,   337,
     380,   188,   188,   188,   422,   423,   424,   282,   190,   204,
     406,   407,   424,   188,   219,   274,   190,   334,   337,   337,
      91,    30,   221,   268,   189,    28,    99,    14,     9,   188,
      30,   190,   271,   461,    29,    88,   217,   455,   456,   457,
     187,     9,    48,    49,    54,    56,    68,   135,   157,   177,
     187,   215,   217,   358,   373,   381,   382,   383,   204,   460,
     219,   187,   229,   190,   189,   190,   189,    99,   156,   108,
     109,   156,   210,   211,   212,   213,   214,   210,   204,   337,
     285,   382,    81,     9,   188,   188,   188,   188,   188,   188,
     188,   189,    48,    49,   453,   454,   129,   261,   187,     9,
     188,   188,    81,    82,   204,   437,   204,    68,   191,   191,
     200,   202,    30,   130,   260,   172,    52,   157,   172,   367,
     338,   133,     9,   386,   188,   152,   461,   461,    14,   349,
     280,   219,   185,     9,   387,   461,   462,   405,   410,   405,
     191,     9,   386,   174,   415,   337,   188,     9,   387,    14,
     341,   239,   129,   259,   187,   446,   337,    30,   194,   194,
     133,   191,     9,   386,   337,   447,   187,   249,   244,   254,
      14,   441,   247,   236,    70,   415,   337,   447,   194,   191,
     188,   188,   194,   191,   188,    48,    49,    68,    76,    77,
      78,    88,   135,   148,   177,   204,   389,   391,   392,   395,
     398,   204,   415,   415,   133,   259,   405,   410,   188,   337,
     275,    73,    74,   276,   219,   328,   219,   330,    99,    36,
     134,   265,   415,   382,   204,    30,   221,   269,   189,   272,
     189,   272,     9,   174,    88,   133,   152,     9,   386,   188,
     166,   448,   449,   450,   448,   382,   382,   382,   382,   382,
     385,   388,   187,   152,   187,   382,   152,   190,    10,    11,
      12,    29,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    67,   152,   447,   191,   373,   190,
     233,   209,   209,   204,   210,   210,   214,     9,   387,   191,
     191,    14,   415,   189,     9,   174,   204,   262,   373,   190,
     428,   134,   415,    14,   194,   337,   191,   200,   461,   262,
     190,   366,    14,   188,   337,   350,   424,   189,   461,   185,
     191,    30,   451,   404,    36,    81,   166,   406,   407,   409,
     406,   407,   461,    36,   166,   337,   382,   280,   187,   373,
     260,   342,   240,   337,   337,   337,   191,   187,   282,   261,
      30,   260,   461,    14,   259,   446,   377,   191,   187,    14,
      76,    77,    78,   204,   390,   390,   392,   393,   394,    50,
     187,    87,   149,   187,     9,   386,   188,   399,    36,   337,
     260,   191,    73,    74,   277,   328,   221,   191,   189,    92,
     189,   265,   415,   187,   133,   264,    14,   219,   272,   102,
     103,   104,   272,   191,   461,   133,   461,   204,   455,     9,
     188,   386,   133,   194,     9,   386,   385,   205,   350,   352,
     354,   188,   127,   205,   382,   433,   434,   382,   382,   382,
      30,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   460,    81,   234,   191,   191,
     213,   189,   382,   454,    99,   100,   452,     9,   290,   188,
     187,   331,   334,   337,   194,   191,   441,   290,   158,   171,
     190,   362,   369,   158,   190,   368,   133,   189,   451,   461,
     349,   462,    81,   166,    14,    81,   447,   415,   337,   188,
     280,   190,   280,   187,   133,   187,   282,   188,   190,   461,
     190,   189,   461,   260,   241,   380,   282,   133,   194,     9,
     386,   391,   393,   149,   350,   396,   397,   392,   415,   190,
     328,    30,    75,   221,   189,   330,   264,   428,   265,   188,
     382,    98,   102,   189,   337,    30,   189,   273,   191,   174,
     461,   133,   166,    30,   188,   382,   382,   188,   133,     9,
     386,   188,   133,   191,     9,   386,   382,    30,   188,   219,
     189,   189,   204,   461,   461,   373,     4,   109,   114,   120,
     122,   159,   160,   162,   191,   291,   316,   317,   318,   323,
     324,   325,   326,   403,   428,   191,   190,   191,    52,   337,
     337,   337,   349,    36,    81,   166,    14,    81,   337,   187,
     451,   188,   290,   188,   280,   337,   282,   188,   290,   441,
     290,   189,   190,   187,   188,   392,   392,   188,   133,   188,
       9,   386,   290,    30,   219,   189,   188,   188,   188,   226,
     189,   189,   273,   219,   461,   461,   133,   382,   350,   382,
     382,   382,   190,   191,   452,   129,   130,   178,   205,   444,
     461,   263,   373,   109,   326,    29,   122,   135,   136,   157,
     163,   300,   301,   302,   303,   373,   161,   308,   309,   125,
     187,   204,   310,   311,   292,   237,   461,     9,   189,     9,
     189,   189,   441,   317,   188,   287,   157,   364,   191,   191,
      81,   166,    14,    81,   337,   282,   114,   339,   451,   191,
     451,   188,   188,   191,   190,   191,   290,   280,   133,   392,
     350,   191,   219,   224,   227,    30,   221,   267,   219,   188,
     382,   133,   133,   219,   373,   373,   446,    14,   205,     9,
     189,   190,   444,   441,   303,   173,   190,     9,   189,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    27,    28,
      55,    69,    70,    71,    72,    73,    74,    75,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   134,
     135,   137,   138,   139,   140,   141,   153,   154,   155,   165,
     167,   168,   170,   177,   178,   180,   182,   183,   204,   370,
     371,     9,   189,   157,   161,   204,   311,   312,   313,   189,
      81,   322,   236,   293,   444,   444,    14,   237,   191,   288,
     289,   444,    14,    81,   337,   188,   187,   190,   189,   190,
     314,   339,   451,   287,   191,   188,   392,   133,    30,   221,
     266,   267,   219,   382,   382,   191,   189,   189,   382,   373,
     296,   461,   304,   305,   381,   301,    14,    30,    49,   306,
     309,     9,    34,   188,    29,    48,    51,    14,     9,   189,
     206,   445,   322,    14,   461,   236,   189,    14,   337,    36,
      81,   361,   219,   219,   190,   314,   191,   451,   392,   219,
      96,   232,   191,   204,   217,   297,   298,   299,     9,   174,
       9,   386,   191,   382,   371,   371,    57,   307,   312,   312,
      29,    48,    51,   382,    81,   173,   187,   189,   382,   446,
     382,    81,     9,   387,   191,   191,   219,   314,    94,   189,
     112,   228,   152,    99,   461,   381,   164,    14,   453,   294,
     187,    36,    81,   188,   191,   189,   187,   170,   235,   204,
     317,   318,   174,   382,   174,   278,   279,   404,   295,    81,
     373,   233,   167,   204,   189,   188,     9,   387,   116,   117,
     118,   320,   321,   278,    81,   263,   189,   451,   404,   462,
     188,   188,   189,   189,   190,   315,   320,    36,    81,   166,
     451,   190,   219,   462,    81,   166,    14,    81,   315,   219,
     191,    36,    81,   166,    14,    81,   337,   191,    81,   166,
      14,    81,   337,    14,    81,   337,   337
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
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->onUse((yyvsp[(3) - (4)]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->onGroupUse((yyvsp[(2) - (6)]).text(), (yyvsp[(4) - (6)]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { _p->onGroupUse((yyvsp[(3) - (7)]).text(), (yyvsp[(5) - (7)]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();;}
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
    { _p->onDeclare((yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                         (yyval) = (yyvsp[(3) - (5)]);
                                         (yyval) = T_DECLARE;;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1033 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1048 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { (yyval).reset();;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1074 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1082 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1083 "hphp.y"
    { (yyval).reset();;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1087 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
    { (yyval).reset();;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1092 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1098 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1104 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1111 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1124 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1130 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1142 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1150 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1156 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
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
#line 1174 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
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
#line 1191 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1199 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1215 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval).reset();;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { (yyval).reset();;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { (yyval).reset();;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { (yyval).reset();;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (4)]));;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    {_p->onDeclareList((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    {_p->onDeclareList((yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));
                                           (yyval) = (yyvsp[(1) - (5)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { (yyval).reset();;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1333 "hphp.y"
    { (yyval).reset();;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { (yyval).reset();;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    { (yyval).reset();;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1362 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1378 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1382 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1387 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1395 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1401 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1425 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1431 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1437 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { (yyval).reset();;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1481 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { (yyval).reset();;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { (yyval).reset();;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { (yyval).reset();;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval).reset();;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { (yyval).reset();;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval).reset();;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { (yyval).reset();;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval).reset();;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval).reset();;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { (yyval).reset();;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
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
#line 2018 "hphp.y"
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
#line 2028 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
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
#line 2046 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2052 "hphp.y"
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
#line 2063 "hphp.y"
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
#line 2071 "hphp.y"
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
#line 2078 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
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
#line 2096 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
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
#line 2204 "hphp.y"
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
#line 2215 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval).reset();;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval).reset();;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
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
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval).reset();;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { (yyval).reset();;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval).reset();;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval).reset();;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { (yyval).reset();;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { (yyval).reset();;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { (yyval).reset();;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { (yyval).reset();;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval).reset();;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval).reset();;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { (yyval).reset();;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2654 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
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
#line 2716 "hphp.y"
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
#line 2731 "hphp.y"
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
#line 2743 "hphp.y"
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
#line 2755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
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
#line 2779 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
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
#line 2808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2828 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2830 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { (yyval).reset();;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2908 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

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

  case 863:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
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

  case 869:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { (yyval).reset();;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
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

  case 905:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3036 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3056 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3113 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3157 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3173 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3176 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    {;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    {;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    {;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3188 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3204 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3210 "hphp.y"
    { ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3215 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3236 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3243 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3253 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3259 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3263 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3266 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3275 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3281 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3290 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13747 "hphp.7.tab.cpp"
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
#line 3293 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

