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
#define YYLAST   16823

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  197
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  265
/* YYNRULES -- Number of rules.  */
#define YYNRULES  984
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1795

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
     200,   202,   204,   206,   210,   212,   216,   218,   222,   224,
     226,   229,   233,   238,   240,   243,   247,   252,   254,   257,
     261,   266,   268,   272,   274,   278,   281,   284,   287,   293,
     298,   301,   302,   304,   306,   308,   310,   314,   320,   329,
     330,   335,   336,   343,   344,   355,   356,   361,   364,   368,
     371,   375,   378,   382,   386,   390,   394,   398,   402,   408,
     410,   412,   414,   415,   425,   426,   437,   443,   444,   458,
     459,   465,   469,   473,   476,   479,   482,   485,   488,   491,
     495,   498,   501,   502,   507,   517,   518,   519,   524,   527,
     528,   530,   531,   533,   534,   544,   545,   556,   557,   569,
     570,   580,   581,   592,   593,   602,   603,   613,   614,   622,
     623,   632,   633,   642,   643,   651,   652,   661,   663,   665,
     667,   669,   671,   674,   678,   682,   685,   688,   689,   692,
     693,   696,   697,   699,   703,   705,   709,   712,   713,   715,
     718,   723,   725,   730,   732,   737,   739,   744,   746,   751,
     755,   761,   765,   770,   775,   781,   787,   792,   793,   795,
     797,   802,   803,   809,   810,   813,   814,   818,   819,   827,
     836,   843,   846,   852,   859,   864,   865,   870,   876,   884,
     891,   898,   906,   916,   925,   932,   940,   946,   949,   954,
     960,   964,   965,   969,   974,   981,   987,   993,  1000,  1009,
    1017,  1020,  1021,  1023,  1026,  1029,  1033,  1038,  1043,  1047,
    1049,  1051,  1054,  1059,  1063,  1069,  1071,  1075,  1078,  1079,
    1082,  1086,  1089,  1090,  1091,  1096,  1097,  1103,  1106,  1109,
    1112,  1113,  1124,  1125,  1137,  1141,  1145,  1149,  1154,  1159,
    1163,  1169,  1172,  1175,  1176,  1183,  1189,  1194,  1198,  1200,
    1202,  1206,  1211,  1213,  1216,  1218,  1220,  1225,  1232,  1234,
    1236,  1241,  1243,  1245,  1249,  1252,  1255,  1256,  1259,  1260,
    1262,  1266,  1268,  1270,  1272,  1274,  1278,  1283,  1288,  1293,
    1295,  1297,  1300,  1303,  1306,  1310,  1314,  1316,  1318,  1320,
    1322,  1326,  1328,  1332,  1334,  1336,  1338,  1339,  1341,  1344,
    1346,  1348,  1350,  1352,  1354,  1356,  1358,  1360,  1361,  1363,
    1365,  1367,  1371,  1377,  1379,  1383,  1389,  1394,  1398,  1402,
    1406,  1411,  1415,  1419,  1423,  1426,  1429,  1431,  1433,  1437,
    1441,  1443,  1445,  1446,  1448,  1451,  1456,  1460,  1464,  1471,
    1474,  1478,  1485,  1487,  1489,  1491,  1493,  1495,  1502,  1506,
    1511,  1518,  1522,  1526,  1530,  1534,  1538,  1542,  1546,  1550,
    1554,  1558,  1562,  1566,  1569,  1572,  1575,  1578,  1582,  1586,
    1590,  1594,  1598,  1602,  1606,  1610,  1614,  1618,  1622,  1626,
    1630,  1634,  1638,  1642,  1646,  1649,  1652,  1655,  1658,  1662,
    1666,  1670,  1674,  1678,  1682,  1686,  1690,  1694,  1698,  1702,
    1708,  1713,  1717,  1719,  1722,  1725,  1728,  1731,  1734,  1737,
    1740,  1743,  1746,  1748,  1750,  1752,  1756,  1759,  1761,  1767,
    1768,  1769,  1781,  1782,  1795,  1796,  1801,  1802,  1810,  1811,
    1817,  1818,  1822,  1823,  1830,  1833,  1836,  1841,  1843,  1845,
    1851,  1855,  1861,  1865,  1868,  1869,  1872,  1873,  1878,  1883,
    1887,  1892,  1897,  1902,  1907,  1909,  1911,  1913,  1915,  1919,
    1923,  1928,  1930,  1933,  1938,  1941,  1948,  1949,  1951,  1956,
    1957,  1960,  1961,  1963,  1965,  1969,  1971,  1975,  1977,  1979,
    1983,  1987,  1989,  1991,  1993,  1995,  1997,  1999,  2001,  2003,
    2005,  2007,  2009,  2011,  2013,  2015,  2017,  2019,  2021,  2023,
    2025,  2027,  2029,  2031,  2033,  2035,  2037,  2039,  2041,  2043,
    2045,  2047,  2049,  2051,  2053,  2055,  2057,  2059,  2061,  2063,
    2065,  2067,  2069,  2071,  2073,  2075,  2077,  2079,  2081,  2083,
    2085,  2087,  2089,  2091,  2093,  2095,  2097,  2099,  2101,  2103,
    2105,  2107,  2109,  2111,  2113,  2115,  2117,  2119,  2121,  2123,
    2125,  2127,  2129,  2131,  2133,  2135,  2137,  2139,  2141,  2143,
    2145,  2147,  2152,  2154,  2156,  2158,  2160,  2162,  2164,  2168,
    2170,  2174,  2176,  2178,  2182,  2184,  2186,  2188,  2191,  2193,
    2194,  2195,  2197,  2199,  2203,  2204,  2206,  2208,  2210,  2212,
    2214,  2216,  2218,  2220,  2222,  2224,  2226,  2228,  2230,  2234,
    2237,  2239,  2241,  2246,  2250,  2255,  2257,  2259,  2263,  2267,
    2271,  2275,  2279,  2283,  2287,  2291,  2295,  2299,  2303,  2307,
    2311,  2315,  2319,  2323,  2327,  2331,  2334,  2337,  2340,  2343,
    2347,  2351,  2355,  2359,  2363,  2367,  2371,  2375,  2379,  2385,
    2390,  2394,  2398,  2402,  2404,  2406,  2408,  2410,  2414,  2418,
    2422,  2425,  2426,  2428,  2429,  2431,  2432,  2438,  2442,  2446,
    2448,  2450,  2452,  2454,  2458,  2461,  2463,  2465,  2467,  2469,
    2471,  2475,  2477,  2479,  2481,  2484,  2487,  2492,  2496,  2501,
    2504,  2505,  2511,  2515,  2519,  2521,  2525,  2527,  2530,  2531,
    2537,  2541,  2544,  2545,  2549,  2550,  2555,  2558,  2559,  2563,
    2567,  2569,  2570,  2572,  2574,  2576,  2578,  2582,  2584,  2586,
    2588,  2592,  2594,  2596,  2600,  2604,  2607,  2612,  2615,  2620,
    2626,  2632,  2638,  2644,  2646,  2648,  2650,  2652,  2654,  2656,
    2660,  2664,  2669,  2674,  2678,  2680,  2682,  2684,  2686,  2690,
    2692,  2697,  2701,  2705,  2707,  2709,  2711,  2713,  2715,  2719,
    2723,  2728,  2733,  2737,  2739,  2741,  2749,  2759,  2767,  2774,
    2783,  2785,  2790,  2795,  2797,  2799,  2804,  2807,  2809,  2810,
    2812,  2814,  2816,  2820,  2824,  2828,  2829,  2831,  2833,  2837,
    2841,  2844,  2848,  2855,  2856,  2858,  2863,  2866,  2867,  2873,
    2877,  2881,  2883,  2890,  2895,  2900,  2903,  2906,  2907,  2913,
    2917,  2921,  2923,  2926,  2927,  2933,  2937,  2941,  2943,  2946,
    2949,  2951,  2954,  2956,  2961,  2965,  2969,  2976,  2980,  2982,
    2984,  2986,  2991,  2996,  3001,  3006,  3011,  3016,  3019,  3022,
    3027,  3030,  3033,  3035,  3039,  3043,  3047,  3048,  3051,  3057,
    3064,  3071,  3079,  3081,  3084,  3086,  3089,  3091,  3096,  3098,
    3103,  3107,  3108,  3110,  3114,  3117,  3121,  3123,  3125,  3126,
    3127,  3130,  3133,  3136,  3141,  3144,  3150,  3154,  3156,  3158,
    3159,  3163,  3168,  3174,  3178,  3180,  3183,  3184,  3189,  3191,
    3195,  3198,  3201,  3204,  3206,  3208,  3210,  3212,  3216,  3221,
    3228,  3230,  3239,  3246,  3248
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     198,     0,    -1,    -1,   199,   200,    -1,   200,   201,    -1,
      -1,   220,    -1,   237,    -1,   244,    -1,   241,    -1,   251,
      -1,   441,    -1,   126,   187,   188,   189,    -1,   153,   213,
     189,    -1,    -1,   153,   213,   190,   202,   200,   191,    -1,
      -1,   153,   190,   203,   200,   191,    -1,   114,   207,   189,
      -1,   114,   108,   208,   189,    -1,   114,   109,   209,   189,
      -1,   217,   189,    -1,    79,    -1,   100,    -1,   159,    -1,
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
     123,    -1,   205,    -1,   127,    -1,   207,     9,   210,    -1,
     210,    -1,   208,     9,   211,    -1,   211,    -1,   209,     9,
     212,    -1,   212,    -1,   213,    -1,   156,   213,    -1,   213,
      99,   204,    -1,   156,   213,    99,   204,    -1,   213,    -1,
     156,   213,    -1,   213,    99,   204,    -1,   156,   213,    99,
     204,    -1,   213,    -1,   156,   213,    -1,   213,    99,   204,
      -1,   156,   213,    99,   204,    -1,   204,    -1,   213,   156,
     204,    -1,   213,    -1,   153,   156,   213,    -1,   156,   213,
      -1,   214,   446,    -1,   214,   446,    -1,   217,     9,   442,
      14,   381,    -1,   109,   442,    14,   381,    -1,   218,   219,
      -1,    -1,   220,    -1,   237,    -1,   244,    -1,   251,    -1,
     190,   218,   191,    -1,    72,   327,   220,   273,   275,    -1,
      72,   327,    30,   218,   274,   276,    75,   189,    -1,    -1,
      91,   327,   221,   267,    -1,    -1,    90,   222,   220,    91,
     327,   189,    -1,    -1,    93,   187,   329,   189,   329,   189,
     329,   188,   223,   265,    -1,    -1,   101,   327,   224,   270,
      -1,   105,   189,    -1,   105,   336,   189,    -1,   107,   189,
      -1,   107,   336,   189,    -1,   110,   189,    -1,   110,   336,
     189,    -1,    27,   105,   189,    -1,   115,   283,   189,    -1,
     121,   285,   189,    -1,    89,   328,   189,    -1,   145,   328,
     189,    -1,   123,   187,   438,   188,   189,    -1,   189,    -1,
      83,    -1,    84,    -1,    -1,    95,   187,   336,    99,   264,
     263,   188,   225,   266,    -1,    -1,    95,   187,   336,    28,
      99,   264,   263,   188,   226,   266,    -1,    97,   187,   269,
     188,   268,    -1,    -1,   111,   229,   112,   187,   372,    81,
     188,   190,   218,   191,   231,   227,   234,    -1,    -1,   111,
     229,   170,   228,   232,    -1,   113,   336,   189,    -1,   106,
     204,   189,    -1,   336,   189,    -1,   330,   189,    -1,   331,
     189,    -1,   332,   189,    -1,   333,   189,    -1,   334,   189,
      -1,   110,   333,   189,    -1,   335,   189,    -1,   204,    30,
      -1,    -1,   190,   230,   218,   191,    -1,   231,   112,   187,
     372,    81,   188,   190,   218,   191,    -1,    -1,    -1,   190,
     233,   218,   191,    -1,   170,   232,    -1,    -1,    36,    -1,
      -1,   108,    -1,    -1,   236,   235,   445,   238,   187,   279,
     188,   450,   313,    -1,    -1,   317,   236,   235,   445,   239,
     187,   279,   188,   450,   313,    -1,    -1,   402,   316,   236,
     235,   445,   240,   187,   279,   188,   450,   313,    -1,    -1,
     163,   204,   242,    30,   460,   440,   190,   286,   191,    -1,
      -1,   402,   163,   204,   243,    30,   460,   440,   190,   286,
     191,    -1,    -1,   257,   254,   245,   258,   259,   190,   289,
     191,    -1,    -1,   402,   257,   254,   246,   258,   259,   190,
     289,   191,    -1,    -1,   128,   255,   247,   260,   190,   289,
     191,    -1,    -1,   402,   128,   255,   248,   260,   190,   289,
     191,    -1,    -1,   127,   250,   379,   258,   259,   190,   289,
     191,    -1,    -1,   165,   256,   252,   259,   190,   289,   191,
      -1,    -1,   402,   165,   256,   253,   259,   190,   289,   191,
      -1,   445,    -1,   157,    -1,   445,    -1,   445,    -1,   127,
      -1,   120,   127,    -1,   120,   119,   127,    -1,   119,   120,
     127,    -1,   119,   127,    -1,   129,   372,    -1,    -1,   130,
     261,    -1,    -1,   129,   261,    -1,    -1,   372,    -1,   261,
       9,   372,    -1,   372,    -1,   262,     9,   372,    -1,   133,
     264,    -1,    -1,   414,    -1,    36,   414,    -1,   134,   187,
     427,   188,    -1,   220,    -1,    30,   218,    94,   189,    -1,
     220,    -1,    30,   218,    96,   189,    -1,   220,    -1,    30,
     218,    92,   189,    -1,   220,    -1,    30,   218,    98,   189,
      -1,   204,    14,   381,    -1,   269,     9,   204,    14,   381,
      -1,   190,   271,   191,    -1,   190,   189,   271,   191,    -1,
      30,   271,   102,   189,    -1,    30,   189,   271,   102,   189,
      -1,   271,   103,   336,   272,   218,    -1,   271,   104,   272,
     218,    -1,    -1,    30,    -1,   189,    -1,   273,    73,   327,
     220,    -1,    -1,   274,    73,   327,    30,   218,    -1,    -1,
      74,   220,    -1,    -1,    74,    30,   218,    -1,    -1,   278,
       9,   403,   319,   461,   166,    81,    -1,   278,     9,   403,
     319,   461,    36,   166,    81,    -1,   278,     9,   403,   319,
     461,   166,    -1,   278,   386,    -1,   403,   319,   461,   166,
      81,    -1,   403,   319,   461,    36,   166,    81,    -1,   403,
     319,   461,   166,    -1,    -1,   403,   319,   461,    81,    -1,
     403,   319,   461,    36,    81,    -1,   403,   319,   461,    36,
      81,    14,   336,    -1,   403,   319,   461,    81,    14,   336,
      -1,   278,     9,   403,   319,   461,    81,    -1,   278,     9,
     403,   319,   461,    36,    81,    -1,   278,     9,   403,   319,
     461,    36,    81,    14,   336,    -1,   278,     9,   403,   319,
     461,    81,    14,   336,    -1,   280,     9,   403,   461,   166,
      81,    -1,   280,     9,   403,   461,    36,   166,    81,    -1,
     280,     9,   403,   461,   166,    -1,   280,   386,    -1,   403,
     461,   166,    81,    -1,   403,   461,    36,   166,    81,    -1,
     403,   461,   166,    -1,    -1,   403,   461,    81,    -1,   403,
     461,    36,    81,    -1,   403,   461,    36,    81,    14,   336,
      -1,   403,   461,    81,    14,   336,    -1,   280,     9,   403,
     461,    81,    -1,   280,     9,   403,   461,    36,    81,    -1,
     280,     9,   403,   461,    36,    81,    14,   336,    -1,   280,
       9,   403,   461,    81,    14,   336,    -1,   282,   386,    -1,
      -1,   336,    -1,    36,   414,    -1,   166,   336,    -1,   282,
       9,   336,    -1,   282,     9,   166,   336,    -1,   282,     9,
      36,   414,    -1,   283,     9,   284,    -1,   284,    -1,    81,
      -1,   192,   414,    -1,   192,   190,   336,   191,    -1,   285,
       9,    81,    -1,   285,     9,    81,    14,   381,    -1,    81,
      -1,    81,    14,   381,    -1,   286,   287,    -1,    -1,   288,
     189,    -1,   443,    14,   381,    -1,   289,   290,    -1,    -1,
      -1,   315,   291,   321,   189,    -1,    -1,   317,   460,   292,
     321,   189,    -1,   322,   189,    -1,   323,   189,    -1,   324,
     189,    -1,    -1,   316,   236,   235,   444,   187,   293,   277,
     188,   450,   314,    -1,    -1,   402,   316,   236,   235,   445,
     187,   294,   277,   188,   450,   314,    -1,   159,   299,   189,
      -1,   160,   307,   189,    -1,   162,   309,   189,    -1,     4,
     129,   372,   189,    -1,     4,   130,   372,   189,    -1,   114,
     262,   189,    -1,   114,   262,   190,   295,   191,    -1,   295,
     296,    -1,   295,   297,    -1,    -1,   216,   152,   204,   167,
     262,   189,    -1,   298,    99,   316,   204,   189,    -1,   298,
      99,   317,   189,    -1,   216,   152,   204,    -1,   204,    -1,
     300,    -1,   299,     9,   300,    -1,   301,   369,   305,   306,
      -1,   157,    -1,    29,   302,    -1,   302,    -1,   135,    -1,
     135,   173,   460,   174,    -1,   135,   173,   460,     9,   460,
     174,    -1,   372,    -1,   122,    -1,   163,   190,   304,   191,
      -1,   136,    -1,   380,    -1,   303,     9,   380,    -1,   303,
     385,    -1,    14,   381,    -1,    -1,    57,   164,    -1,    -1,
     308,    -1,   307,     9,   308,    -1,   161,    -1,   310,    -1,
     204,    -1,   125,    -1,   187,   311,   188,    -1,   187,   311,
     188,    51,    -1,   187,   311,   188,    29,    -1,   187,   311,
     188,    48,    -1,   310,    -1,   312,    -1,   312,    51,    -1,
     312,    29,    -1,   312,    48,    -1,   311,     9,   311,    -1,
     311,    34,   311,    -1,   204,    -1,   157,    -1,   161,    -1,
     189,    -1,   190,   218,   191,    -1,   189,    -1,   190,   218,
     191,    -1,   317,    -1,   122,    -1,   317,    -1,    -1,   318,
      -1,   317,   318,    -1,   116,    -1,   117,    -1,   118,    -1,
     121,    -1,   120,    -1,   119,    -1,   183,    -1,   320,    -1,
      -1,   116,    -1,   117,    -1,   118,    -1,   321,     9,    81,
      -1,   321,     9,    81,    14,   381,    -1,    81,    -1,    81,
      14,   381,    -1,   322,     9,   443,    14,   381,    -1,   109,
     443,    14,   381,    -1,   323,     9,   443,    -1,   120,   109,
     443,    -1,   120,   325,   440,    -1,   325,   440,    14,   460,
      -1,   109,   178,   445,    -1,   187,   326,   188,    -1,    70,
     376,   379,    -1,    70,   249,    -1,    69,   336,    -1,   361,
      -1,   356,    -1,   187,   336,   188,    -1,   328,     9,   336,
      -1,   336,    -1,   328,    -1,    -1,    27,    -1,    27,   336,
      -1,    27,   336,   133,   336,    -1,   187,   330,   188,    -1,
     414,    14,   330,    -1,   134,   187,   427,   188,    14,   330,
      -1,    28,   336,    -1,   414,    14,   333,    -1,   134,   187,
     427,   188,    14,   333,    -1,   337,    -1,   414,    -1,   326,
      -1,   418,    -1,   417,    -1,   134,   187,   427,   188,    14,
     336,    -1,   414,    14,   336,    -1,   414,    14,    36,   414,
      -1,   414,    14,    36,    70,   376,   379,    -1,   414,    26,
     336,    -1,   414,    25,   336,    -1,   414,    24,   336,    -1,
     414,    23,   336,    -1,   414,    22,   336,    -1,   414,    21,
     336,    -1,   414,    20,   336,    -1,   414,    19,   336,    -1,
     414,    18,   336,    -1,   414,    17,   336,    -1,   414,    16,
     336,    -1,   414,    15,   336,    -1,   414,    66,    -1,    66,
     414,    -1,   414,    65,    -1,    65,   414,    -1,   336,    32,
     336,    -1,   336,    33,   336,    -1,   336,    10,   336,    -1,
     336,    12,   336,    -1,   336,    11,   336,    -1,   336,    34,
     336,    -1,   336,    36,   336,    -1,   336,    35,   336,    -1,
     336,    50,   336,    -1,   336,    48,   336,    -1,   336,    49,
     336,    -1,   336,    51,   336,    -1,   336,    52,   336,    -1,
     336,    67,   336,    -1,   336,    53,   336,    -1,   336,    47,
     336,    -1,   336,    46,   336,    -1,    48,   336,    -1,    49,
     336,    -1,    54,   336,    -1,    56,   336,    -1,   336,    38,
     336,    -1,   336,    37,   336,    -1,   336,    40,   336,    -1,
     336,    39,   336,    -1,   336,    41,   336,    -1,   336,    45,
     336,    -1,   336,    42,   336,    -1,   336,    44,   336,    -1,
     336,    43,   336,    -1,   336,    55,   376,    -1,   187,   337,
     188,    -1,   336,    29,   336,    30,   336,    -1,   336,    29,
      30,   336,    -1,   336,    31,   336,    -1,   437,    -1,    64,
     336,    -1,    63,   336,    -1,    62,   336,    -1,    61,   336,
      -1,    60,   336,    -1,    59,   336,    -1,    58,   336,    -1,
      71,   377,    -1,    57,   336,    -1,   383,    -1,   355,    -1,
     354,    -1,   193,   378,   193,    -1,    13,   336,    -1,   358,
      -1,   114,   187,   360,   386,   188,    -1,    -1,    -1,   236,
     235,   187,   340,   279,   188,   450,   338,   190,   218,   191,
      -1,    -1,   317,   236,   235,   187,   341,   279,   188,   450,
     338,   190,   218,   191,    -1,    -1,   183,    81,   343,   348,
      -1,    -1,   183,   184,   344,   279,   185,   450,   348,    -1,
      -1,   183,   190,   345,   218,   191,    -1,    -1,    81,   346,
     348,    -1,    -1,   184,   347,   279,   185,   450,   348,    -1,
       8,   336,    -1,     8,   333,    -1,     8,   190,   218,   191,
      -1,    88,    -1,   439,    -1,   350,     9,   349,   133,   336,
      -1,   349,   133,   336,    -1,   351,     9,   349,   133,   381,
      -1,   349,   133,   381,    -1,   350,   385,    -1,    -1,   351,
     385,    -1,    -1,   177,   187,   352,   188,    -1,   135,   187,
     428,   188,    -1,    68,   428,   194,    -1,   372,   190,   430,
     191,    -1,   372,   190,   432,   191,    -1,   358,    68,   424,
     194,    -1,   359,    68,   424,   194,    -1,   355,    -1,   439,
      -1,   417,    -1,    88,    -1,   187,   337,   188,    -1,   360,
       9,    81,    -1,   360,     9,    36,    81,    -1,    81,    -1,
      36,    81,    -1,   171,   157,   362,   172,    -1,   364,    52,
      -1,   364,   172,   365,   171,    52,   363,    -1,    -1,   157,
      -1,   364,   366,    14,   367,    -1,    -1,   365,   368,    -1,
      -1,   157,    -1,   158,    -1,   190,   336,   191,    -1,   158,
      -1,   190,   336,   191,    -1,   361,    -1,   370,    -1,   369,
      30,   370,    -1,   369,    49,   370,    -1,   204,    -1,    71,
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
      -1,   178,    -1,   180,    -1,   177,    -1,   215,   187,   281,
     188,    -1,   216,    -1,   157,    -1,   372,    -1,   380,    -1,
     121,    -1,   422,    -1,   187,   337,   188,    -1,   373,    -1,
     374,   152,   423,    -1,   373,    -1,   420,    -1,   375,   152,
     423,    -1,   372,    -1,   121,    -1,   425,    -1,   187,   188,
      -1,   327,    -1,    -1,    -1,    87,    -1,   434,    -1,   187,
     281,   188,    -1,    -1,    76,    -1,    77,    -1,    78,    -1,
      88,    -1,   140,    -1,   141,    -1,   155,    -1,   137,    -1,
     168,    -1,   138,    -1,   139,    -1,   154,    -1,   182,    -1,
     148,    87,   149,    -1,   148,   149,    -1,   380,    -1,   214,
      -1,   135,   187,   384,   188,    -1,    68,   384,   194,    -1,
     177,   187,   353,   188,    -1,   382,    -1,   357,    -1,   187,
     381,   188,    -1,   381,    32,   381,    -1,   381,    33,   381,
      -1,   381,    10,   381,    -1,   381,    12,   381,    -1,   381,
      11,   381,    -1,   381,    34,   381,    -1,   381,    36,   381,
      -1,   381,    35,   381,    -1,   381,    50,   381,    -1,   381,
      48,   381,    -1,   381,    49,   381,    -1,   381,    51,   381,
      -1,   381,    52,   381,    -1,   381,    53,   381,    -1,   381,
      47,   381,    -1,   381,    46,   381,    -1,   381,    67,   381,
      -1,    54,   381,    -1,    56,   381,    -1,    48,   381,    -1,
      49,   381,    -1,   381,    38,   381,    -1,   381,    37,   381,
      -1,   381,    40,   381,    -1,   381,    39,   381,    -1,   381,
      41,   381,    -1,   381,    45,   381,    -1,   381,    42,   381,
      -1,   381,    44,   381,    -1,   381,    43,   381,    -1,   381,
      29,   381,    30,   381,    -1,   381,    29,    30,   381,    -1,
     216,   152,   205,    -1,   157,   152,   205,    -1,   216,   152,
     127,    -1,   214,    -1,    80,    -1,   439,    -1,   380,    -1,
     195,   434,   195,    -1,   196,   434,   196,    -1,   148,   434,
     149,    -1,   387,   385,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   387,     9,   381,   133,   381,    -1,   387,     9,
     381,    -1,   381,   133,   381,    -1,   381,    -1,    76,    -1,
      77,    -1,    78,    -1,   148,    87,   149,    -1,   148,   149,
      -1,    76,    -1,    77,    -1,    78,    -1,   204,    -1,    88,
      -1,    88,    50,   390,    -1,   388,    -1,   390,    -1,   204,
      -1,    48,   389,    -1,    49,   389,    -1,   135,   187,   392,
     188,    -1,    68,   392,   194,    -1,   177,   187,   395,   188,
      -1,   393,   385,    -1,    -1,   393,     9,   391,   133,   391,
      -1,   393,     9,   391,    -1,   391,   133,   391,    -1,   391,
      -1,   394,     9,   391,    -1,   391,    -1,   396,   385,    -1,
      -1,   396,     9,   349,   133,   391,    -1,   349,   133,   391,
      -1,   394,   385,    -1,    -1,   187,   397,   188,    -1,    -1,
     399,     9,   204,   398,    -1,   204,   398,    -1,    -1,   401,
     399,   385,    -1,    47,   400,    46,    -1,   402,    -1,    -1,
     131,    -1,   132,    -1,   204,    -1,   157,    -1,   190,   336,
     191,    -1,   405,    -1,   423,    -1,   204,    -1,   190,   336,
     191,    -1,   407,    -1,   423,    -1,    68,   424,   194,    -1,
     190,   336,   191,    -1,   415,   409,    -1,   187,   326,   188,
     409,    -1,   426,   409,    -1,   187,   326,   188,   409,    -1,
     187,   326,   188,   404,   406,    -1,   187,   337,   188,   404,
     406,    -1,   187,   326,   188,   404,   405,    -1,   187,   337,
     188,   404,   405,    -1,   421,    -1,   371,    -1,   419,    -1,
     420,    -1,   410,    -1,   412,    -1,   414,   404,   406,    -1,
     375,   152,   423,    -1,   416,   187,   281,   188,    -1,   417,
     187,   281,   188,    -1,   187,   414,   188,    -1,   371,    -1,
     419,    -1,   420,    -1,   410,    -1,   414,   404,   406,    -1,
     413,    -1,   416,   187,   281,   188,    -1,   187,   414,   188,
      -1,   375,   152,   423,    -1,   421,    -1,   410,    -1,   371,
      -1,   355,    -1,   380,    -1,   187,   414,   188,    -1,   187,
     337,   188,    -1,   417,   187,   281,   188,    -1,   416,   187,
     281,   188,    -1,   187,   418,   188,    -1,   339,    -1,   342,
      -1,   414,   404,   408,   446,   187,   281,   188,    -1,   187,
     326,   188,   404,   408,   446,   187,   281,   188,    -1,   375,
     152,   206,   446,   187,   281,   188,    -1,   375,   152,   423,
     187,   281,   188,    -1,   375,   152,   190,   336,   191,   187,
     281,   188,    -1,   422,    -1,   422,    68,   424,   194,    -1,
     422,   190,   336,   191,    -1,   423,    -1,    81,    -1,   192,
     190,   336,   191,    -1,   192,   423,    -1,   336,    -1,    -1,
     421,    -1,   411,    -1,   412,    -1,   425,   404,   406,    -1,
     374,   152,   421,    -1,   187,   414,   188,    -1,    -1,   411,
      -1,   413,    -1,   425,   404,   405,    -1,   187,   414,   188,
      -1,   427,     9,    -1,   427,     9,   414,    -1,   427,     9,
     134,   187,   427,   188,    -1,    -1,   414,    -1,   134,   187,
     427,   188,    -1,   429,   385,    -1,    -1,   429,     9,   336,
     133,   336,    -1,   429,     9,   336,    -1,   336,   133,   336,
      -1,   336,    -1,   429,     9,   336,   133,    36,   414,    -1,
     429,     9,    36,   414,    -1,   336,   133,    36,   414,    -1,
      36,   414,    -1,   431,   385,    -1,    -1,   431,     9,   336,
     133,   336,    -1,   431,     9,   336,    -1,   336,   133,   336,
      -1,   336,    -1,   433,   385,    -1,    -1,   433,     9,   381,
     133,   381,    -1,   433,     9,   381,    -1,   381,   133,   381,
      -1,   381,    -1,   434,   435,    -1,   434,    87,    -1,   435,
      -1,    87,   435,    -1,    81,    -1,    81,    68,   436,   194,
      -1,    81,   404,   204,    -1,   150,   336,   191,    -1,   150,
      80,    68,   336,   194,   191,    -1,   151,   414,   191,    -1,
     204,    -1,    82,    -1,    81,    -1,   124,   187,   328,   188,
      -1,   125,   187,   414,   188,    -1,   125,   187,   337,   188,
      -1,   125,   187,   418,   188,    -1,   125,   187,   417,   188,
      -1,   125,   187,   326,   188,    -1,     7,   336,    -1,     6,
     336,    -1,     5,   187,   336,   188,    -1,     4,   336,    -1,
       3,   336,    -1,   414,    -1,   438,     9,   414,    -1,   375,
     152,   205,    -1,   375,   152,   127,    -1,    -1,    99,   460,
      -1,   178,   445,    14,   460,   189,    -1,   402,   178,   445,
      14,   460,   189,    -1,   180,   445,   440,    14,   460,   189,
      -1,   402,   180,   445,   440,    14,   460,   189,    -1,   206,
      -1,   460,   206,    -1,   205,    -1,   460,   205,    -1,   206,
      -1,   206,   173,   452,   174,    -1,   204,    -1,   204,   173,
     452,   174,    -1,   173,   448,   174,    -1,    -1,   460,    -1,
     447,     9,   460,    -1,   447,   385,    -1,   447,     9,   166,
      -1,   448,    -1,   166,    -1,    -1,    -1,    30,   460,    -1,
      99,   460,    -1,   100,   460,    -1,   452,     9,   453,   204,
      -1,   453,   204,    -1,   452,     9,   453,   204,   451,    -1,
     453,   204,   451,    -1,    48,    -1,    49,    -1,    -1,    88,
     133,   460,    -1,    29,    88,   133,   460,    -1,   216,   152,
     204,   133,   460,    -1,   455,     9,   454,    -1,   454,    -1,
     455,   385,    -1,    -1,   177,   187,   456,   188,    -1,   216,
      -1,   204,   152,   459,    -1,   204,   446,    -1,    29,   460,
      -1,    57,   460,    -1,   216,    -1,   135,    -1,   136,    -1,
     457,    -1,   458,   152,   459,    -1,   135,   173,   460,   174,
      -1,   135,   173,   460,     9,   460,   174,    -1,   157,    -1,
     187,   108,   187,   449,   188,    30,   460,   188,    -1,   187,
     460,     9,   447,   385,   188,    -1,   460,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   727,   727,   727,   736,   738,   741,   742,   743,   744,
     745,   746,   747,   750,   752,   752,   754,   754,   756,   757,
     759,   761,   766,   767,   768,   769,   770,   771,   772,   776,
     777,   778,   779,   780,   781,   782,   783,   784,   785,   786,
     787,   788,   789,   790,   791,   792,   793,   794,   795,   796,
     797,   798,   799,   800,   801,   802,   803,   804,   805,   806,
     807,   808,   809,   810,   811,   812,   813,   814,   815,   816,
     817,   818,   819,   820,   821,   822,   823,   824,   825,   826,
     827,   828,   829,   830,   831,   832,   833,   834,   835,   836,
     840,   844,   845,   849,   851,   855,   857,   861,   863,   867,
     868,   869,   870,   875,   876,   877,   878,   883,   884,   885,
     886,   891,   892,   896,   897,   899,   903,   910,   917,   921,
     927,   929,   932,   933,   934,   935,   938,   939,   943,   948,
     948,   954,   954,   961,   960,   966,   966,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,   982,   983,
     984,   985,   989,   987,   996,   994,  1001,  1009,  1003,  1013,
    1011,  1015,  1016,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1036,  1036,  1041,  1047,  1051,  1051,  1059,  1060,
    1064,  1065,  1069,  1075,  1073,  1088,  1085,  1101,  1098,  1115,
    1114,  1123,  1121,  1133,  1132,  1151,  1149,  1168,  1167,  1176,
    1174,  1185,  1185,  1192,  1191,  1203,  1201,  1214,  1215,  1219,
    1222,  1225,  1226,  1227,  1230,  1231,  1234,  1236,  1239,  1240,
    1243,  1244,  1247,  1248,  1252,  1253,  1258,  1259,  1262,  1263,
    1264,  1268,  1269,  1273,  1274,  1278,  1279,  1283,  1284,  1289,
    1290,  1295,  1296,  1297,  1298,  1301,  1304,  1306,  1309,  1310,
    1314,  1316,  1319,  1322,  1325,  1326,  1329,  1330,  1334,  1340,
    1346,  1353,  1355,  1360,  1365,  1371,  1375,  1379,  1383,  1388,
    1393,  1398,  1403,  1409,  1418,  1423,  1428,  1434,  1436,  1440,
    1444,  1449,  1453,  1456,  1459,  1463,  1467,  1471,  1475,  1480,
    1488,  1490,  1493,  1494,  1495,  1496,  1498,  1500,  1505,  1506,
    1509,  1510,  1511,  1515,  1516,  1518,  1519,  1523,  1525,  1528,
    1532,  1538,  1540,  1543,  1543,  1547,  1546,  1550,  1552,  1555,
    1558,  1556,  1572,  1568,  1582,  1584,  1586,  1588,  1590,  1592,
    1594,  1598,  1599,  1600,  1603,  1609,  1613,  1619,  1622,  1627,
    1629,  1634,  1639,  1643,  1644,  1648,  1649,  1651,  1653,  1659,
    1660,  1662,  1666,  1667,  1672,  1676,  1677,  1681,  1682,  1686,
    1688,  1694,  1699,  1700,  1702,  1706,  1707,  1708,  1709,  1713,
    1714,  1715,  1716,  1717,  1718,  1720,  1725,  1728,  1729,  1733,
    1734,  1738,  1739,  1742,  1743,  1746,  1747,  1750,  1751,  1755,
    1756,  1757,  1758,  1759,  1760,  1761,  1765,  1766,  1769,  1770,
    1771,  1774,  1776,  1778,  1779,  1782,  1784,  1788,  1790,  1794,
    1798,  1802,  1807,  1808,  1810,  1811,  1812,  1813,  1816,  1820,
    1821,  1825,  1826,  1830,  1831,  1832,  1833,  1837,  1841,  1846,
    1850,  1854,  1859,  1860,  1861,  1862,  1863,  1867,  1869,  1870,
    1871,  1874,  1875,  1876,  1877,  1878,  1879,  1880,  1881,  1882,
    1883,  1884,  1885,  1886,  1887,  1888,  1889,  1890,  1891,  1892,
    1893,  1894,  1895,  1896,  1897,  1898,  1899,  1900,  1901,  1902,
    1903,  1904,  1905,  1906,  1907,  1908,  1909,  1910,  1911,  1912,
    1913,  1914,  1915,  1916,  1918,  1919,  1921,  1922,  1924,  1925,
    1926,  1927,  1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,
    1936,  1937,  1938,  1939,  1940,  1941,  1942,  1943,  1947,  1951,
    1956,  1955,  1970,  1968,  1986,  1985,  2004,  2003,  2022,  2021,
    2039,  2039,  2054,  2054,  2072,  2073,  2074,  2079,  2081,  2085,
    2089,  2095,  2099,  2105,  2107,  2111,  2113,  2117,  2121,  2122,
    2126,  2133,  2140,  2142,  2147,  2148,  2149,  2150,  2152,  2156,
    2157,  2158,  2159,  2163,  2169,  2178,  2191,  2192,  2195,  2198,
    2201,  2202,  2205,  2209,  2212,  2215,  2222,  2223,  2227,  2228,
    2230,  2234,  2235,  2236,  2237,  2238,  2239,  2240,  2241,  2242,
    2243,  2244,  2245,  2246,  2247,  2248,  2249,  2250,  2251,  2252,
    2253,  2254,  2255,  2256,  2257,  2258,  2259,  2260,  2261,  2262,
    2263,  2264,  2265,  2266,  2267,  2268,  2269,  2270,  2271,  2272,
    2273,  2274,  2275,  2276,  2277,  2278,  2279,  2280,  2281,  2282,
    2283,  2284,  2285,  2286,  2287,  2288,  2289,  2290,  2291,  2292,
    2293,  2294,  2295,  2296,  2297,  2298,  2299,  2300,  2301,  2302,
    2303,  2304,  2305,  2306,  2307,  2308,  2309,  2310,  2311,  2312,
    2313,  2317,  2322,  2323,  2327,  2328,  2329,  2330,  2332,  2336,
    2337,  2348,  2349,  2351,  2363,  2364,  2365,  2369,  2370,  2371,
    2375,  2376,  2377,  2380,  2382,  2386,  2387,  2388,  2389,  2391,
    2392,  2393,  2394,  2395,  2396,  2397,  2398,  2399,  2400,  2403,
    2408,  2409,  2410,  2412,  2413,  2415,  2416,  2417,  2418,  2420,
    2422,  2424,  2426,  2428,  2429,  2430,  2431,  2432,  2433,  2434,
    2435,  2436,  2437,  2438,  2439,  2440,  2441,  2442,  2443,  2444,
    2446,  2448,  2450,  2452,  2453,  2456,  2457,  2461,  2465,  2467,
    2471,  2474,  2477,  2483,  2484,  2485,  2486,  2487,  2488,  2489,
    2494,  2496,  2500,  2501,  2504,  2505,  2509,  2512,  2514,  2516,
    2520,  2521,  2522,  2523,  2526,  2530,  2531,  2532,  2533,  2537,
    2539,  2546,  2547,  2548,  2549,  2550,  2551,  2553,  2554,  2559,
    2561,  2564,  2567,  2569,  2571,  2574,  2576,  2580,  2582,  2585,
    2588,  2594,  2596,  2599,  2600,  2605,  2608,  2612,  2612,  2617,
    2620,  2621,  2625,  2626,  2630,  2631,  2632,  2636,  2641,  2646,
    2647,  2651,  2656,  2661,  2662,  2666,  2667,  2672,  2674,  2679,
    2690,  2704,  2716,  2731,  2732,  2733,  2734,  2735,  2736,  2737,
    2747,  2756,  2758,  2760,  2764,  2765,  2766,  2767,  2768,  2784,
    2785,  2787,  2789,  2796,  2797,  2798,  2799,  2800,  2801,  2802,
    2803,  2805,  2810,  2814,  2815,  2819,  2822,  2829,  2833,  2842,
    2849,  2857,  2859,  2860,  2864,  2865,  2867,  2872,  2873,  2884,
    2885,  2886,  2887,  2898,  2901,  2904,  2905,  2906,  2907,  2918,
    2922,  2923,  2924,  2926,  2927,  2928,  2932,  2934,  2937,  2939,
    2940,  2941,  2942,  2945,  2947,  2948,  2952,  2954,  2957,  2959,
    2960,  2961,  2965,  2967,  2970,  2973,  2975,  2977,  2981,  2982,
    2984,  2985,  2991,  2992,  2994,  3004,  3006,  3008,  3011,  3012,
    3013,  3017,  3018,  3019,  3020,  3021,  3022,  3023,  3024,  3025,
    3026,  3027,  3031,  3032,  3036,  3038,  3046,  3048,  3052,  3056,
    3061,  3065,  3073,  3074,  3078,  3079,  3085,  3086,  3095,  3096,
    3104,  3107,  3111,  3114,  3119,  3124,  3126,  3127,  3128,  3132,
    3133,  3137,  3138,  3141,  3144,  3146,  3150,  3156,  3157,  3158,
    3162,  3166,  3176,  3184,  3186,  3190,  3192,  3197,  3203,  3206,
    3211,  3219,  3222,  3225,  3226,  3229,  3232,  3233,  3238,  3241,
    3245,  3249,  3255,  3265,  3266
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
  "ident_for_class_const", "ident", "use_declarations",
  "use_fn_declarations", "use_const_declarations", "use_declaration",
  "use_fn_declaration", "use_const_declaration", "namespace_name",
  "namespace_string", "namespace_string_typeargs",
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
     205,   206,   206,   207,   207,   208,   208,   209,   209,   210,
     210,   210,   210,   211,   211,   211,   211,   212,   212,   212,
     212,   213,   213,   214,   214,   214,   215,   216,   217,   217,
     218,   218,   219,   219,   219,   219,   220,   220,   220,   221,
     220,   222,   220,   223,   220,   224,   220,   220,   220,   220,
     220,   220,   220,   220,   220,   220,   220,   220,   220,   220,
     220,   220,   225,   220,   226,   220,   220,   227,   220,   228,
     220,   220,   220,   220,   220,   220,   220,   220,   220,   220,
     220,   220,   230,   229,   231,   231,   233,   232,   234,   234,
     235,   235,   236,   238,   237,   239,   237,   240,   237,   242,
     241,   243,   241,   245,   244,   246,   244,   247,   244,   248,
     244,   250,   249,   252,   251,   253,   251,   254,   254,   255,
     256,   257,   257,   257,   257,   257,   258,   258,   259,   259,
     260,   260,   261,   261,   262,   262,   263,   263,   264,   264,
     264,   265,   265,   266,   266,   267,   267,   268,   268,   269,
     269,   270,   270,   270,   270,   271,   271,   271,   272,   272,
     273,   273,   274,   274,   275,   275,   276,   276,   277,   277,
     277,   277,   277,   277,   277,   277,   278,   278,   278,   278,
     278,   278,   278,   278,   279,   279,   279,   279,   279,   279,
     279,   279,   280,   280,   280,   280,   280,   280,   280,   280,
     281,   281,   282,   282,   282,   282,   282,   282,   283,   283,
     284,   284,   284,   285,   285,   285,   285,   286,   286,   287,
     288,   289,   289,   291,   290,   292,   290,   290,   290,   290,
     293,   290,   294,   290,   290,   290,   290,   290,   290,   290,
     290,   295,   295,   295,   296,   297,   297,   298,   298,   299,
     299,   300,   300,   301,   301,   302,   302,   302,   302,   302,
     302,   302,   303,   303,   304,   305,   305,   306,   306,   307,
     307,   308,   309,   309,   309,   310,   310,   310,   310,   311,
     311,   311,   311,   311,   311,   311,   312,   312,   312,   313,
     313,   314,   314,   315,   315,   316,   316,   317,   317,   318,
     318,   318,   318,   318,   318,   318,   319,   319,   320,   320,
     320,   321,   321,   321,   321,   322,   322,   323,   323,   324,
     324,   325,   326,   326,   326,   326,   326,   326,   327,   328,
     328,   329,   329,   330,   330,   330,   330,   331,   332,   333,
     334,   335,   336,   336,   336,   336,   336,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,   337,   337,   337,   337,   338,   338,
     340,   339,   341,   339,   343,   342,   344,   342,   345,   342,
     346,   342,   347,   342,   348,   348,   348,   349,   349,   350,
     350,   351,   351,   352,   352,   353,   353,   354,   355,   355,
     356,   357,   358,   358,   359,   359,   359,   359,   359,   360,
     360,   360,   360,   361,   362,   362,   363,   363,   364,   364,
     365,   365,   366,   367,   367,   368,   368,   368,   369,   369,
     369,   370,   370,   370,   370,   370,   370,   370,   370,   370,
     370,   370,   370,   370,   370,   370,   370,   370,   370,   370,
     370,   370,   370,   370,   370,   370,   370,   370,   370,   370,
     370,   370,   370,   370,   370,   370,   370,   370,   370,   370,
     370,   370,   370,   370,   370,   370,   370,   370,   370,   370,
     370,   370,   370,   370,   370,   370,   370,   370,   370,   370,
     370,   370,   370,   370,   370,   370,   370,   370,   370,   370,
     370,   370,   370,   370,   370,   370,   370,   370,   370,   370,
     370,   371,   372,   372,   373,   373,   373,   373,   373,   374,
     374,   375,   375,   375,   376,   376,   376,   377,   377,   377,
     378,   378,   378,   379,   379,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     381,   381,   381,   381,   381,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   381,   381,   381,   381,   381,
     382,   382,   382,   383,   383,   383,   383,   383,   383,   383,
     384,   384,   385,   385,   386,   386,   387,   387,   387,   387,
     388,   388,   388,   388,   388,   389,   389,   389,   389,   390,
     390,   391,   391,   391,   391,   391,   391,   391,   391,   392,
     392,   393,   393,   393,   393,   394,   394,   395,   395,   396,
     396,   397,   397,   398,   398,   399,   399,   401,   400,   402,
     403,   403,   404,   404,   405,   405,   405,   406,   406,   407,
     407,   408,   408,   409,   409,   410,   410,   411,   411,   412,
     412,   413,   413,   414,   414,   414,   414,   414,   414,   414,
     414,   414,   414,   414,   415,   415,   415,   415,   415,   415,
     415,   415,   415,   416,   416,   416,   416,   416,   416,   416,
     416,   416,   417,   418,   418,   419,   419,   420,   420,   420,
     421,   422,   422,   422,   423,   423,   423,   424,   424,   425,
     425,   425,   425,   425,   425,   426,   426,   426,   426,   426,
     427,   427,   427,   427,   427,   427,   428,   428,   429,   429,
     429,   429,   429,   429,   429,   429,   430,   430,   431,   431,
     431,   431,   432,   432,   433,   433,   433,   433,   434,   434,
     434,   434,   435,   435,   435,   435,   435,   435,   436,   436,
     436,   437,   437,   437,   437,   437,   437,   437,   437,   437,
     437,   437,   438,   438,   439,   439,   440,   440,   441,   441,
     441,   441,   442,   442,   443,   443,   444,   444,   445,   445,
     446,   446,   447,   447,   448,   449,   449,   449,   449,   450,
     450,   451,   451,   452,   452,   452,   452,   453,   453,   453,
     454,   454,   454,   455,   455,   456,   456,   457,   458,   459,
     459,   460,   460,   460,   460,   460,   460,   460,   460,   460,
     460,   460,   460,   461,   461
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
       1,     1,     1,     3,     1,     3,     1,     3,     1,     1,
       2,     3,     4,     1,     2,     3,     4,     1,     2,     3,
       4,     1,     3,     1,     3,     2,     2,     2,     5,     4,
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
     865,   669,     0,   675,   676,   677,    22,   734,   854,   150,
     151,   678,     0,   131,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   182,     0,     0,     0,     0,     0,     0,
     389,   390,   391,   394,   393,   392,     0,     0,     0,     0,
     211,     0,     0,     0,   682,   684,   685,   679,   680,     0,
       0,     0,   686,   681,     0,   653,    24,    25,    26,    28,
      27,     0,   683,     0,     0,     0,     0,   687,   395,   522,
       0,   149,   121,     0,   670,     0,     0,     4,   111,   113,
     733,     0,   652,     0,     6,   181,     7,     9,     8,    10,
       0,     0,   387,   434,     0,     0,     0,     0,     0,     0,
       0,   432,   843,   844,   504,   503,   417,   507,     0,   416,
     814,   654,   661,     0,   736,   502,   386,   817,   818,   829,
     433,     0,     0,   436,   435,   815,   816,   813,   850,   853,
     492,   735,    11,   394,   393,   392,     0,     0,    28,     0,
     111,   181,     0,   921,   433,   920,     0,   918,   917,   506,
       0,   424,   429,     0,     0,   474,   475,   476,   477,   501,
     499,   498,   497,   496,   495,   494,   493,   854,   678,   656,
       0,     0,   941,   836,   654,     0,   655,   456,     0,   454,
       0,   881,     0,   743,   415,   665,   201,     0,   941,   414,
     664,   659,     0,   674,   655,   860,   861,   867,   859,   666,
       0,     0,   668,   500,     0,     0,     0,     0,   420,     0,
     129,   422,     0,     0,   135,   137,     0,     0,   139,     0,
      69,    68,    63,    62,    54,    55,    46,    66,    77,     0,
      49,     0,    61,    53,    59,    79,    72,    71,    44,    67,
      86,    87,    45,    82,    42,    83,    43,    84,    41,    88,
      76,    80,    85,    73,    74,    48,    75,    78,    40,    70,
      56,    89,    64,    57,    47,    39,    38,    37,    36,    35,
      34,    58,    90,    92,    51,    32,    33,    60,   974,   975,
      52,   980,    31,    50,    81,     0,     0,   111,    91,   932,
     973,     0,   976,     0,     0,   141,     0,     0,   172,     0,
       0,     0,     0,     0,     0,    94,    99,   300,     0,     0,
     299,     0,   215,     0,   212,   305,     0,     0,     0,     0,
       0,   938,   197,   209,   873,   877,     0,   902,     0,   689,
       0,     0,     0,   900,     0,    16,     0,   115,   189,   203,
     210,   559,   534,     0,   926,   514,   516,   518,   791,   423,
     434,     0,     0,   432,   433,   435,     0,     0,   856,   671,
       0,   672,     0,     0,     0,   171,     0,     0,   117,   291,
       0,    21,   180,     0,   208,   193,   207,   392,   395,   181,
     388,   164,   165,   166,   167,   168,   170,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   865,     0,   163,   858,   858,   887,
       0,     0,     0,     0,     0,     0,     0,     0,   385,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   455,   453,   792,   793,     0,   858,     0,   805,
     291,   291,   858,     0,   873,     0,   181,     0,     0,   143,
       0,   789,   784,   743,     0,   434,   432,     0,   885,     0,
     539,   742,   876,   674,   434,   432,   433,   117,     0,   291,
     413,     0,   807,   667,     0,   121,   251,     0,   521,     0,
     146,     0,     0,   421,     0,     0,     0,     0,     0,   138,
     162,   140,   974,   975,   971,   972,     0,   966,     0,     0,
       0,     0,    65,    30,    52,    29,   933,   169,   142,   121,
       0,   159,   161,     0,     0,    96,   103,     0,     0,    98,
     107,   100,     0,    18,     0,     0,   301,     0,   144,   214,
     213,     0,     0,   145,   922,     0,     0,   434,   432,   433,
     436,   435,     0,   959,   221,     0,   874,     0,     0,   147,
       0,     0,   688,   901,   734,     0,     0,   899,   739,   898,
     114,     5,    13,    14,     0,   219,     0,     0,   527,     0,
       0,   743,     0,     0,   662,   657,   528,     0,     0,     0,
       0,   791,   121,     0,   745,   790,   984,   412,   426,   488,
     823,   842,   126,   120,   122,   123,   124,   125,   386,     0,
     505,   737,   738,   112,   743,     0,   942,     0,     0,     0,
     745,   292,     0,   510,   183,   217,     0,   459,   461,   460,
       0,     0,   491,   457,   458,   462,   464,   463,   479,   478,
     481,   480,   482,   484,   486,   485,   483,   473,   472,   466,
     467,   465,   468,   469,   471,   487,   470,   857,     0,     0,
     891,     0,   743,   925,     0,   924,   941,   820,   199,   191,
     205,     0,   926,   195,   181,     0,   427,   430,   438,   452,
     451,   450,   449,   448,   447,   446,   445,   444,   443,   442,
     441,   795,     0,   794,   797,   819,   801,   941,   798,     0,
       0,     0,     0,     0,     0,     0,     0,   919,   425,   782,
     786,   742,   788,     0,   658,     0,   880,     0,   879,   217,
       0,   658,   864,   863,   850,   853,     0,     0,   794,   797,
     862,   798,   418,   253,   255,   121,   525,   524,   419,     0,
     121,   235,   130,   422,     0,     0,     0,     0,     0,   247,
     247,   136,     0,     0,     0,     0,   964,   743,     0,   948,
       0,     0,     0,     0,     0,   741,     0,   653,     0,     0,
     691,   652,   696,     0,   690,   119,   695,   941,   977,     0,
       0,     0,   104,     0,    19,     0,   108,     0,    20,     0,
       0,    93,   101,     0,   298,   306,   303,     0,     0,   911,
     916,   913,   912,   915,   914,    12,   957,   958,     0,     0,
       0,     0,   873,   870,     0,   538,   910,   909,   908,     0,
     904,     0,   905,   907,     0,     5,     0,     0,     0,   553,
     554,   562,   561,     0,   432,     0,   742,   533,   537,     0,
       0,   927,     0,   515,     0,     0,   949,   791,   277,   983,
       0,     0,   806,     0,   855,   742,   944,   940,   293,   294,
     651,   744,   290,     0,   791,     0,     0,   219,   512,   185,
     490,     0,   542,   543,     0,   540,   742,   886,     0,     0,
     291,   221,     0,   219,     0,     0,   217,     0,   865,   439,
       0,     0,   803,   804,   821,   822,   851,   852,     0,     0,
       0,   770,   750,   751,   752,   759,     0,     0,     0,   763,
     761,   762,   776,   743,     0,   784,   884,   883,     0,   219,
       0,   808,   673,     0,   257,     0,     0,   127,     0,     0,
       0,     0,     0,     0,     0,   227,   228,   239,     0,   121,
     237,   156,   247,     0,   247,     0,     0,   978,     0,     0,
       0,   742,   965,   967,   947,   743,   946,     0,   743,   717,
     718,   715,   716,   749,     0,   743,   741,     0,   536,     0,
       0,   893,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     970,   173,     0,   176,   160,     0,    95,   105,     0,    97,
     109,   102,   302,     0,   923,   148,   959,   939,   954,   220,
     222,   312,     0,     0,   871,     0,   903,     0,    17,     0,
     926,   218,   312,     0,     0,   658,   530,     0,   663,   928,
       0,   949,   519,     0,     0,   984,     0,   282,   280,   797,
     809,   941,   797,   810,   943,     0,     0,   295,   118,     0,
     791,   216,     0,   791,     0,   489,   890,   889,     0,   291,
       0,     0,     0,     0,     0,     0,   219,   187,   674,   796,
     291,     0,   755,   756,   757,   758,   764,   765,   774,     0,
     743,     0,   770,     0,   754,   778,   742,   781,   783,   785,
       0,   878,     0,   796,     0,     0,     0,     0,   254,   526,
     132,     0,   422,   227,   229,   873,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   241,     0,     0,   960,     0,
     963,   742,     0,     0,     0,   693,   742,   740,     0,   731,
       0,   743,     0,   697,   732,   730,   897,     0,   743,   700,
     702,   701,     0,     0,   698,   699,   703,   705,   704,   720,
     719,   722,   721,   723,   725,   727,   726,   724,   713,   712,
     707,   708,   706,   709,   710,   711,   714,   969,     0,   121,
     106,   110,   304,     0,     0,     0,   956,     0,   386,   875,
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
     953,   951,   952,   223,     0,     0,     0,   393,   384,     0,
       0,     0,   198,   311,   313,     0,   383,     0,     0,     0,
     926,   386,     0,   906,   308,   204,   556,     0,     0,   529,
     517,     0,   286,   276,     0,   279,   285,   291,   509,   949,
     386,   949,     0,   888,     0,   847,   386,     0,   386,   931,
     312,   791,   845,   773,   772,   766,     0,   768,   742,   777,
     386,   121,   256,   128,   133,   154,   230,     0,   238,   244,
     121,   246,   962,     0,     0,   532,     0,   896,   895,   728,
     121,   177,   955,     0,     0,     0,   934,     0,     0,     0,
     224,     0,   926,     0,   349,   345,   351,   653,    28,     0,
     339,     0,   344,   348,   361,     0,   359,   364,     0,   363,
       0,   362,     0,   181,   315,     0,   317,     0,   318,   319,
       0,     0,   872,     0,   557,   555,   566,   564,   287,     0,
       0,   274,   284,     0,     0,     0,     0,   194,   509,   949,
     849,   200,   308,   206,   386,     0,     0,   780,     0,   202,
     252,     0,     0,   121,   233,   153,   245,   981,   746,     0,
       0,     0,     0,     0,   411,     0,   935,     0,   329,   333,
     408,   409,   343,     0,     0,     0,   324,   617,   616,   613,
     615,   614,   634,   636,   635,   605,   576,   577,   595,   611,
     610,   572,   582,   583,   585,   584,   604,   588,   586,   587,
     589,   590,   591,   592,   593,   594,   596,   597,   598,   599,
     600,   601,   603,   602,   573,   574,   575,   578,   579,   581,
     619,   620,   629,   628,   627,   626,   625,   624,   612,   631,
     621,   622,   623,   606,   607,   608,   609,   632,   633,   637,
     639,   638,   640,   641,   618,   643,   642,   645,   647,   646,
     580,   650,   648,   649,   644,   630,   571,   356,   568,     0,
     325,   377,   378,   376,   369,     0,   370,   326,   403,     0,
       0,     0,     0,   407,     0,   181,   190,   307,     0,     0,
       0,   275,   289,   846,     0,   121,   379,   121,   184,     0,
       0,     0,   196,   949,   771,     0,   121,   231,   134,   155,
       0,   531,   894,   175,   327,   328,   406,   225,     0,     0,
     743,     0,   352,   340,     0,     0,     0,   358,   360,     0,
       0,   365,   372,   373,   371,     0,     0,   314,   936,     0,
       0,     0,   410,     0,   309,     0,   288,     0,   551,   745,
       0,     0,   121,   186,   192,     0,   779,     0,     0,   157,
     330,   111,     0,   331,   332,     0,     0,   346,   742,   354,
     350,   355,   569,   570,     0,   341,   374,   375,   367,   368,
     366,   404,   401,   959,   320,   316,   405,     0,   310,   552,
     744,     0,   511,   380,     0,   188,     0,   234,     0,   179,
       0,   386,     0,   353,   357,     0,     0,   791,   322,     0,
     549,   508,   513,   232,     0,     0,   158,   337,     0,   385,
     347,   402,   937,     0,   745,   397,   791,   550,     0,   178,
       0,     0,   336,   949,   791,   261,   398,   399,   400,   984,
     396,     0,     0,     0,   335,     0,   397,     0,   949,     0,
     334,   381,   121,   321,   984,     0,   266,   264,     0,   121,
       0,     0,   267,     0,     0,   262,   323,     0,   382,     0,
     270,   260,     0,   263,   269,   174,   271,     0,     0,   258,
     268,     0,   259,   273,   272
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   855,   601,   170,  1406,   696,
     334,   554,   558,   335,   555,   559,   109,   110,   111,   112,
     113,   386,   633,   634,   522,   239,  1471,   528,  1387,  1472,
    1709,   811,   329,   549,  1669,  1034,  1209,  1726,   403,   171,
     635,   895,  1094,  1264,   117,   604,   912,   636,   655,   916,
     584,   911,   219,   503,   637,   605,   913,   405,   352,   369,
     120,   897,   858,   841,  1049,  1409,  1147,   965,  1618,  1475,
     772,   971,   527,   781,   973,  1297,   764,   954,   957,  1136,
    1733,  1734,   623,   624,   649,   650,   339,   340,   346,  1443,
    1597,  1598,  1218,  1333,  1432,  1591,  1717,  1736,  1628,  1673,
    1674,  1675,  1419,  1420,  1421,  1422,  1630,  1631,  1637,  1685,
    1425,  1426,  1430,  1584,  1585,  1586,  1608,  1763,  1334,  1335,
     172,   122,  1749,  1750,  1589,  1337,  1338,  1339,  1340,   123,
     232,   523,   524,   124,   125,   126,   127,   128,   129,   130,
     131,  1455,   132,   894,  1093,   133,   620,   621,   622,   236,
     378,   518,   610,   611,  1171,   612,  1172,   134,   135,   136,
     802,   137,   138,  1659,   139,   606,  1445,   607,  1063,   863,
    1235,  1232,  1577,  1578,   140,   141,   142,   222,   143,   223,
     233,   390,   510,   144,   993,   806,   145,   994,   886,   878,
     995,   940,  1116,   941,  1118,  1119,  1120,   943,  1275,  1276,
     944,   740,   493,   183,   184,   638,   626,   476,  1079,  1080,
     726,   727,   882,   147,   225,   148,   149,   174,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   688,   229,   230,
     587,   212,   213,   691,   692,  1177,  1178,   362,   363,   849,
     160,   575,   161,   619,   162,   321,  1599,  1649,   353,   398,
     644,   645,   987,  1074,  1216,   838,   839,   786,   787,   788,
     322,   323,   808,  1408,   880
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1463
static const yytype_int16 yypact[] =
{
   -1463,   151, -1463, -1463,  4892, 12652, 12652,   -30, 12652, 12652,
   12652, 10518, 12652, -1463, 12652, 12652, 12652, 12652, 12652, 12652,
   12652, 12652, 12652, 12652, 12652, 12652, 15483, 15483, 10712, 12652,
   15938,   -24,     3, -1463, -1463, -1463, -1463, -1463,   225, -1463,
   -1463,   160, 12652, -1463,     3,   142,   145,   167, -1463,     3,
   10906,  1660, 11100, -1463, 13402,  9548,   171, 12652,  1223,   214,
   -1463, -1463, -1463,    60,    43,    78,   177,   196,   205,   213,
   -1463,  1660,   224,   254, -1463, -1463, -1463, -1463, -1463, 12652,
     607,   990, -1463, -1463,  1660, -1463, -1463, -1463, -1463,  1660,
   -1463,  1660, -1463,   238,   346,  1660,  1660, -1463,   318, -1463,
   11294, -1463, -1463,    62,   515,   524,   524, -1463,   509,   401,
       2,   374, -1463,    77, -1463,   558, -1463, -1463, -1463, -1463,
    1259,   455, -1463, -1463,   395,   417,   421,   452,   460,   470,
   14055, -1463, -1463, -1463, -1463,   147, -1463,   593,   604, -1463,
      87,   486, -1463,   528,     4, -1463,  1972,   146, -1463, -1463,
    2358,   150,   500,   263, -1463,   155,   194,   513,   399, -1463,
   -1463,   638, -1463, -1463, -1463,   566,   544,   580, -1463, 12652,
   -1463,   558,   455, 16403,  3347, 16403, 12652, 16403, 16403, 13849,
     554, 14879, 13849,   700,  1660,   695,   695,   126,   695,   695,
     695,   695,   695,   695,   695,   695,   695, -1463, -1463, -1463,
      49, 12652,   597, -1463, -1463,   629,   598,   226,   601,   226,
   15483, 15089,   600,   765, -1463,   566, -1463, 12652,   597, -1463,
     644, -1463,   646,   616, -1463,   157, -1463, -1463, -1463,   226,
     150, 11488, -1463, -1463, 12652,  8190,   793,    84, 16403,  9160,
   -1463, 12652, 12652,  1660, -1463, -1463, 14101,   631, -1463, 14168,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,  2514,
   -1463,  2514, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,    67,    69,
     580, -1463, -1463, -1463, -1463,   642,  2111,    76, -1463, -1463,
     666,   817, -1463,   684, 14435, -1463,   649, 14214, -1463,    41,
   14260,  1475,  1490,  1660,    89, -1463,    61, -1463, 15107,    91,
   -1463,   718, -1463,   737, -1463,   844,    93, 15483, 12652, 12652,
     678,   698, -1463, -1463, 15200, 10712,    96,   255,   614, -1463,
   12846, 15483,   622, -1463,  1660, -1463,   494,   401, -1463, -1463,
   -1463, -1463, 16031,   860,   777, -1463, -1463, -1463,   139, 12652,
     691,   696, 16403,   697,  1001,   699,  5086, 12652, -1463,   484,
     702,   565,   484,   429,   462, -1463,  1660,  2514,   706,  9742,
   13402, -1463, -1463,  1099, -1463, -1463, -1463, -1463, -1463,   558,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, 12652, 12652, 12652,
   11682, 12652, 12652, 12652, 12652, 12652, 12652, 12652, 12652, 12652,
   12652, 12652, 12652, 12652, 12652, 12652, 12652, 12652, 12652, 12652,
   12652, 12652, 12652, 12652, 16124, 12652, -1463, 12652, 12652, 12652,
    4233,  1660,  1660,  1660,  1660,  1660,  1259,   789,   891,  9354,
   12652, 12652, 12652, 12652, 12652, 12652, 12652, 12652, 12652, 12652,
   12652, 12652, -1463, -1463, -1463, -1463,   563, 12652, 12652, -1463,
    9742,  9742, 12652, 12652, 15200,   713,   558, 11876, 14327, -1463,
   12652, -1463,   715,   897,   760,   726,   727, 12985,   226, 12070,
   -1463, 12264, -1463,   616,   729,   741,  1562, -1463,   309,  9742,
   -1463,   680, -1463, -1463, 14373, -1463, -1463,  9936, -1463, 12652,
   -1463,   840,  8384,   923,   744, 16286,   920,   112,   122, -1463,
   -1463, -1463,   762, -1463, -1463, -1463,  2514,   585,   749,   930,
   15014,  1660, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
     753, -1463, -1463,  1660,    97, -1463,   206,  1660,    98, -1463,
     217,   439,  1635, -1463,  1660, 12652,   226,   214, -1463, -1463,
   -1463, 15014,   861, -1463,   226,   115,   116,   759,   761,  2162,
     201,   763,   767,   354,   825,   771,   226,   119,   774, -1463,
    1346,  1660, -1463, -1463,   896,  2676,   446, -1463, -1463, -1463,
     401, -1463, -1463, -1463,   929,   835,   794,   352,   815, 12652,
     839,   961,   786,   827, -1463,   159, -1463,  2514,  2514,   968,
     793,   139, -1463,   799,   976, -1463,  2514,    52, -1463,   438,
     148, -1463, -1463, -1463, -1463, -1463, -1463, -1463,  2043,  3240,
   -1463, -1463, -1463, -1463,   980,   818, -1463, 15483, 12652,   806,
     986, 16403,   982, -1463, -1463,   871,  1153, 11085, 16535, 13849,
   12652, 16357, 16650, 16721,  3964, 10109, 11854, 12241, 12435, 12435,
   12435, 12435,  2986,  2986,  2986,  2986,  2986,  1047,  1047,   837,
     837,   837,   126,   126,   126, -1463,   695, 16403,   807,   808,
   15645,   812,   995,   358, 12652,   369,   597,   185, -1463, -1463,
   -1463,   991,   777, -1463,   558, 15297, -1463, -1463, 13849, 13849,
   13849, 13849, 13849, 13849, 13849, 13849, 13849, 13849, 13849, 13849,
   13849, -1463, 12652,   414, -1463,   163, -1463,   597,   505,   836,
    4050,   826,   841,   838,  4395,   120,   849, -1463, 16403,  2735,
   -1463,  1660, -1463,    52,    34, 15483, 16403, 15483, 15691,   871,
      52,   226,   166, -1463,   159,   879,   850, 12652, -1463,   173,
   -1463, -1463, -1463,  7996,   556, -1463, -1463, 16403, 16403,     3,
   -1463, -1463, -1463, 12652,   940, 14897, 15014,  1660,  8578,   852,
     855, -1463,    75,   957,   913,   895, -1463,  1039,   863,  3059,
    2514, 15014, 15014, 15014, 15014, 15014,   865,   902,   868, 15014,
     466,   905, -1463,   870, -1463, 16493, -1463,   208, -1463,  5280,
    1542,   874,   453,  1475, -1463,  1660,   463,  1490, -1463,  1660,
    1660, -1463, -1463,  4445, -1463, 16493,  1051, 15483,   881, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,   102,  1660,
    1542,   878, 15200, 15390,  1057, -1463, -1463, -1463, -1463,   882,
   -1463, 12652, -1463, -1463,  4504, -1463,  2514,  1542,   883, -1463,
   -1463, -1463, -1463,  1058,   887, 12652, 16031, -1463, -1463,  4233,
     888, -1463,  2514, -1463,   893,  5474,  1049,    50, -1463, -1463,
     132,   563, -1463,   680, -1463,  2514, -1463, -1463,   226, 16403,
   -1463, 10130, -1463, 15014,    44,   906,  1542,   835, -1463, -1463,
   16650, 12652, -1463, -1463, 12652, -1463, 12652, -1463, 10503,   914,
    9742,   825,  1050,   835,  2514,  1070,   871,  1660, 16124,   226,
   10891,   916, -1463, -1463,   152,   917, -1463, -1463,  1078,   899,
     899,  2735, -1463, -1463, -1463,  1060,   921,    51,   925, -1463,
   -1463, -1463, -1463,  1098,   927,   715,   226,   226, 12458,   835,
     680, -1463, -1463, 11473,   596,     3,  9160, -1463,  5668,   928,
    5862,   931, 14897, 15483,   934,   983,   226, 16493,  1104, -1463,
   -1463, -1463, -1463,   752, -1463,   416,  2514, -1463,   989,  2514,
    1660,   585, -1463, -1463, -1463,  1114, -1463,   936,   980,   754,
     754,  1062,  1062, 15796,   933,  1116, 15014, 14703, 16031,  2929,
   14569, 15014, 15014, 15014, 15014, 14804, 15014, 15014, 15014, 15014,
   15014, 15014, 15014, 15014, 15014, 15014, 15014, 15014, 15014, 15014,
   15014, 15014, 15014, 15014, 15014, 15014, 15014, 15014, 15014,  1660,
   -1463, -1463,  1054, -1463, -1463,  1660, -1463, -1463,  1660, -1463,
   -1463, -1463, -1463, 15014,   226, -1463,   354, -1463,   634,  1121,
   -1463, -1463,   123,   953,   226, 10324, -1463,  2468, -1463,  4698,
     777,  1121, -1463,   496,   -17, -1463, 16403,  1009,   956, -1463,
     959,  1049, -1463,  2514,   793,  2514,    53,  1131,  1074,   175,
   -1463,   597,   308, -1463, -1463, 15483, 12652, 16403, 16493,   969,
      44, -1463,   972,    44,   971, 16650, 16403, 15750,   977,  9742,
     978,   979,  2514,   987,   981,  2514,   835, -1463,   616,   567,
    9742, 12652, -1463, -1463, -1463, -1463, -1463, -1463,  1035,   985,
    1164,  1093,  2735,  1043, -1463, 16031,  2735, -1463, -1463, -1463,
   15483, 16403,   993, -1463,     3,  1158,  1125,  9160, -1463, -1463,
   -1463,  1012, 12652,   983,   226, 15200, 14897,  1014, 15014,  6056,
     809,  1015, 12652,    71,   437, -1463,  1031,  2514, -1463,  1077,
   -1463,  3566,  1177,  1023, 15014, -1463, 15014, -1463,  1025, -1463,
    1081,  1206,  1030, -1463, -1463, -1463, 15855,  1028,  1211, 14432,
   16577, 16613, 15014, 16449, 16756,  9722, 10691, 12048, 13119, 14570,
   14570, 14570, 14570,  3629,  3629,  3629,  3629,  3629,  1340,  1340,
     754,   754,   754,  1062,  1062,  1062,  1062, -1463,  1038, -1463,
   -1463, -1463, 16493,  1660,  2514,  2514, -1463,  1542,    88, -1463,
   15200, -1463, -1463, 13849,  1036, -1463,  1040,  1253, -1463,   474,
   12652, -1463, -1463, -1463, 12652, -1463, 12652, -1463,   793, -1463,
   -1463,   143,  1214,  1152, 12652, -1463,  1055,   226, 16403,  1049,
    1046, -1463,  1053,    44, 12652,  9742,  1064, -1463, -1463,   777,
   -1463, -1463,  1056,  1048,  1067, -1463,  1072,  2735, -1463,  2735,
   -1463, -1463,  1076, -1463,  1122,  1079,  1256, -1463,   226, -1463,
    1238, -1463,  1080, -1463, -1463,  1082,  1085,   130, -1463, -1463,
   16493,  1086,  1087, -1463, 14009, -1463, -1463, -1463, -1463, -1463,
   -1463,  2514, -1463,  2514, -1463, 16493, 15899, -1463, 15014, 16031,
   -1463, -1463, 15014, -1463, 15014, -1463, 16686, 15014,  1089,  6250,
     634, -1463, -1463, -1463,   621, 13541,  1542,  1165, -1463,  1380,
    1120,  1084, -1463, -1463, -1463,   789,  3350,    99,   101,  1088,
     777,   891,   131, -1463, -1463, -1463,  1126, 12637, 12831, 16403,
   -1463,   239,  1270,  1201, 12652, -1463, 16403,  9742,  1171,  1049,
    1558,  1049,  1101, 16403,  1102, -1463,  1786,  1097,  1808, -1463,
   -1463,    44, -1463, -1463,  1159, -1463,  2735, -1463, 16031, -1463,
    1843, -1463,  7996, -1463, -1463, -1463, -1463,  8772, -1463, -1463,
   -1463,  7996, -1463,  1103, 15014, 16493,  1160, 16493, 15957, 16686,
   -1463, -1463, -1463,  1542,  1542,  1660, -1463,  1282, 14703,    55,
   -1463, 13541,   777,  2466, -1463,  1128, -1463,   103,  1109,   104,
   -1463, 13848, -1463, -1463, -1463,   105, -1463, -1463,  1034, -1463,
    1115, -1463,  1222,   558, -1463, 13680, -1463, 13680, -1463, -1463,
    1291,   789, -1463, 13124, -1463, -1463, -1463, -1463,  1294,  1228,
   12652, -1463, 16403,  1123,  1133,  1134,   587, -1463,  1171,  1049,
   -1463, -1463, -1463, -1463,  1962,  1130,  2735, -1463,  1188, -1463,
    7996,  8966,  8772, -1463, -1463, -1463,  7996, -1463, 16493, 15014,
   15014,  6444,  1136,  1144, -1463, 15014, -1463,  1542, -1463, -1463,
   -1463, -1463, -1463,  2514,  2251,  1380, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463, -1463, -1463,   534, -1463,  1120,
   -1463, -1463, -1463, -1463, -1463,   113,   648, -1463,  1320,   107,
   14435,  1222,  1321, -1463,  2514,   558, -1463, -1463,  1147,  1332,
   12652, -1463, 16403, -1463,   285, -1463, -1463, -1463, -1463,  1161,
     587, 13263, -1463,  1049, -1463,  2735, -1463, -1463, -1463, -1463,
    6638, 16493, 16493, -1463, -1463, -1463, 16493, -1463,   548,   136,
    1339,  1163, -1463, -1463, 15014, 13848, 13848,  1293, -1463,  1034,
    1034,   731, -1463, -1463, -1463, 15014,  1279, -1463,  1190,  1174,
     108, 15014, -1463,  1660, -1463, 15014, 16403,  1283, -1463,  1356,
    6832,  7026, -1463, -1463, -1463,   587, -1463,  7220,  1179,  1254,
   -1463,  1277,  1226, -1463, -1463,  1281,  2514, -1463,  2251, -1463,
   -1463, 16493, -1463, -1463,  1217, -1463,  1350, -1463, -1463, -1463,
   -1463, 16493,  1381,   354, -1463, -1463, 16493,  1209, 16493, -1463,
     315,  1210, -1463, -1463,  7414, -1463,  1212, -1463,  1213,  1232,
    1660,   891,  1229, -1463, -1463, 15014,   140,    95, -1463,  1323,
   -1463, -1463, -1463, -1463,  1542,   874, -1463,  1239,  1660,   692,
   -1463, 16493, -1463,  1236,  1401,   804,    95, -1463,  1336, -1463,
    1542,  1231, -1463,  1049,   114, -1463, -1463, -1463, -1463,  2514,
   -1463,  1242,  1243,   110, -1463,   626,   804,   186,  1049,  1244,
   -1463, -1463, -1463, -1463,  2514,   422,  1419,  1357,   626, -1463,
    7608,   203,  1423,  1358, 12652, -1463, -1463,  7802, -1463,   434,
    1427,  1361, 12652, -1463, 16403, -1463,  1429,  1366, 12652, -1463,
   16403, 12652, -1463, 16403, 16403
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1463, -1463, -1463,  -542, -1463, -1463, -1463,   397,   -52,   -33,
   -1463, -1463, -1463,   886,   636,   637,    46,  1449, -1463,  2549,
   -1463,  -440, -1463,    24, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463, -1463, -1463,  -274, -1463, -1463,  -145,    23,
      26, -1463, -1463, -1463, -1463, -1463, -1463,    28, -1463, -1463,
   -1463, -1463, -1463, -1463,    29, -1463, -1463,   997,  1004,  1008,
     -91,  -670,  -831,   546,   605,  -273,   325,  -895, -1463,    -2,
   -1463, -1463, -1463, -1463,  -717,   179, -1463, -1463, -1463, -1463,
    -267, -1463,  -572, -1463,  -420, -1463, -1463,   904, -1463,    15,
   -1463, -1463,  -990, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463,   -13, -1463,    70, -1463, -1463, -1463, -1463,
   -1463,   -95, -1463,   154,  -806, -1463, -1462,  -281, -1463,  -138,
     181,   -99,  -268, -1463,  -100, -1463, -1463, -1463,   165,   -23,
      -3,    20,  -703,   -62, -1463, -1463,   -12, -1463, -1463,    -5,
     -43,    36, -1463, -1463, -1463, -1463, -1463, -1463, -1463, -1463,
   -1463,  -567,  -810, -1463, -1463, -1463, -1463, -1463,  1271, -1463,
   -1463, -1463, -1463, -1463,   435, -1463, -1463, -1463, -1463, -1463,
   -1463, -1463, -1463,  -774, -1463,  1997,     9, -1463,  1268,  -386,
   -1463, -1463,  -461,  2893,  2815, -1463, -1463,   501,  -165,  -625,
   -1463, -1463,   571,   386,  -652,   390, -1463, -1463, -1463, -1463,
   -1463,   568, -1463, -1463, -1463,    92,  -826,  -158,  -396,  -388,
   -1463,   633,   -97, -1463, -1463,    35,    39,   381, -1463, -1463,
     344,   -32, -1463,  -338,     5,  -341,    47,   220, -1463, -1463,
    -353,  1162, -1463, -1463, -1463, -1463, -1463,   820,   433, -1463,
   -1463, -1463,  -336,  -608, -1463,  1124,  -814, -1463,    81,  -174,
      79,   732, -1463,  -968,   200,  -168,   482,   550, -1463, -1463,
   -1463, -1463,   503,  2283, -1035
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -969
static const yytype_int16 yytable[] =
{
     173,   175,   318,   177,   178,   179,   181,   182,   457,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   319,   410,   211,   214,   892,   485,   115,   114,   235,
     116,   615,   118,   119,   614,   228,   616,   238,   381,   221,
    1241,   240,   749,   326,   507,   246,   244,   249,   502,   874,
     327,  1075,   330,   873,   479,   456,  1067,   383,   685,   854,
     731,   732,   237,   975,  1487,   226,  1092,  1143,   385,   227,
     961,   511,  1227,   410,   238,   763,   -65,   380,   -30,   949,
     724,   -65,  1103,   -30,   976,   -29,   400,   942,   725,   756,
     -29,    13,  1324,   519,   915,   382,   146,    13,   562,   356,
     567,  1295,   572,  1238,   336,   519,   813,   817,  1435,   809,
    1437,  1046,  -342,  1495,  1579,   759,  1646,  1646,  1132,  1487,
     477,   777,  1639,   760,   827,   519,   383,   366,   843,   843,
     367,   735,   843,   512,  1242,    13,   494,   385,  1123,   843,
     843,  1233,    13,   197,   409,  1676,   380,  1640,  1663,  1046,
     388,     3,   779,   550,  -941,  -824,  -655,   176,   496,   345,
     564,    13,   343,   231,   382,   474,   475,   754,  1076,   385,
     344,   488,   370,  1234,   505,   397,   373,   374,   495,  1351,
     341,   444,   875,   474,   475,   121,    13,   342,  1170,  -941,
     234,  -837,  -941,   445,   504,   486,   382,  1325,   359,   591,
    1124,   406,  1326,  1705,    60,    61,    62,   163,  1327,   407,
    1328,   551,   382,  1077,  -827,  -544,  -831,   396,   477,  1243,
    -830,  -839,  1765,  -825,  1352,  -866,   514,   482,  -547,   514,
    -656,  -828,  -281,  -520,  -869,  -744,   238,   525,  -744,  1779,
     536,  -868,   478,  -811,  1488,  1489,  1106,  1329,  1330,   977,
    1331,  1288,   387,  -832,   103,  1150,   -65,  1154,   -30,   516,
    1296,  1360,  -826,   521,   656,   -29,   401,  1766,  1366,  -546,
    1368,   408,   318,   520,  -835,  1263,  1047,  -824,   563,  1332,
     568,  1358,   573,  -265,  1780,   589,   814,   818,  1436,  1380,
    1438,   546,  -342,  1496,  1580,   337,  1647,  1695,  1078,  1760,
     778,  1641,  -744,   828,   829,   815,   578,   844,   928,  1353,
    1677,  1219,   780,  1059,  1732,  1274,   819,   581,  1386,  1442,
    1448,  1657,  1089,   590,  -281,   958,   577,   458,   742,   241,
     960,  -546,   242,  -834,  -836,  -838,  -827,  -663,  -831,  -841,
     478,   736,  -830,   238,   382,  -825,  -662,  -866,   318,   483,
     211,  1719,  1767,  -828,   243,   595,  -869,   474,   475,   410,
    1029,   328,   396,  -868,   347,  -811,  1658,   319,   576,  1781,
     208,   208,   910,   396,   181,  -832,  -812,   556,   560,   561,
    1464,   397,   639,   348,  -826,   150,   474,   475,   481,   833,
     197,  1456,   349,  1458,   651,   371,  1720,   706,   695,   375,
     350,   108,   836,   837,   860,  1449,   338,   207,   209,   115,
     600,   354,   657,   658,   659,   661,   662,   663,   664,   665,
     666,   667,   668,   669,   670,   671,   672,   673,   674,   675,
     676,   677,   678,   679,   680,   681,   682,   683,   684,  1285,
     686,   355,   687,   687,   690,   318,   867,   707,   247,   228,
     481,   317,  1226,   221,   708,   709,   710,   711,   712,   713,
     714,   715,   716,   717,   718,   719,   720,   482,   351,   881,
     625,   883,   687,   730,  1277,   651,   651,   687,   734,   226,
     704,   384,   708,   227,   654,   738,   368,  1082,   351,  1052,
    1100,  1610,   351,   351,   746,  1083,   748,   697,  -812,  1396,
     457,   103,   376,  1772,   651,   766,  -548,  1240,   377,   861,
     357,  1407,   767,   753,   768,  1786,   597,   351,  1250,  1152,
    1153,  1252,   909,   728,   862,   615,  1346,   907,   614,  1149,
     616,   -92,  1108,   372,   370,   701,   702,   406,   820,   395,
    1152,  1153,   -91,   357,   697,   -92,   771,   456,  1634,   597,
     384,  -657,  1035,   921,   208,   755,   -91,   396,   761,   917,
     823,   399,  1038,    53,  1635,   357,   864,   121,  1468,   474,
     475,    60,    61,    62,   163,   164,   407,   474,   475,   360,
     361,   492,   384,  1636,   411,   881,   883,  -799,  1773,   483,
    -658,   498,   950,   883,   402,   396,   357,  1490,   506,   812,
    1787,  -799,   389,   816,   382,   357,   412,  1155,   336,   396,
     413,   392,   360,   361,   783,  1373,   150,  1374,  -941,   396,
     150,  1592,   982,  1593,   641,  -839,   507,    36,  1298,   955,
     956,   371,   108,  1030,   360,   361,   108,   853,   408,   397,
     526,   414,    36,   889,   197,  1665,   357,  1265,    48,   415,
     396,  1367,   597,   951,  1228,   900,  -941,   615,   642,   416,
     614,   447,   616,    48,    36,   360,   361,  1229,   689,  1134,
    1135,  1350,   448,   784,   360,   361,   449,  1642,  -802,  1256,
     450,  1362,   208,   602,   603,    48,  1230,   480,   357,   908,
    1266,   208,  -802,   580,   358,   357,  1643,   729,   208,  1644,
    -833,   167,   733,   357,    84,   208,  -545,    86,    87,   597,
      88,   168,    90,   625,  1757,   360,   361,   920,  -656,   566,
     721,   545,    86,    87,  1467,    88,   168,    90,   574,  1771,
     579,   484,  1440,  1214,  1215,   586,   364,   899,   167,  1670,
    -800,    84,   596,   489,    86,    87,   491,    88,   168,    90,
    1403,  1404,   953,   722,  -800,   103,   359,   360,   361,    36,
    1688,   197,   445,   592,   360,   361,   959,   150,   238,  1319,
     397,   598,   360,   361,   501,  1755,  1606,  1607,  1127,  1689,
      48,   497,  1690,   108,   615,  -837,   115,   614,   481,   616,
    1768,   593,  1287,   643,   500,   599,  -654,   317,   508,  1465,
     351,   517,   970,   509,  1491,  1025,  1026,  1027,    60,    61,
      62,   163,   164,   407,  1614,  1761,  1762,   695,  -968,   458,
     530,  1028,   593,  1163,   599,   593,   599,   599,   208,   537,
    1167,   540,   115,  1686,  1687,  1364,   541,   721,   547,    86,
      87,  1382,    88,   168,    90,   569,  1057,   545,   351,   699,
     351,   351,   351,   351,  1151,  1152,  1153,  1391,   571,   556,
    1066,  1682,  1683,   560,   570,   586,   582,  1342,   985,   988,
     757,   583,   103,   723,   617,   408,   618,   115,   114,   627,
     116,  1742,   118,   119,   628,   629,  1087,   631,   441,   442,
     443,  1735,   444,  -116,   545,   640,  1095,    53,   115,  1096,
     653,  1097,   739,   150,   445,   651,   741,  1246,   758,   592,
    1735,  1292,  1152,  1153,   743,   744,  1068,   750,  1756,   108,
    1746,  1747,  1748,   228,   391,   393,   394,   221,   728,   751,
     761,   769,   519,   773,   776,   536,   789,  1453,   807,   790,
     810,  1470,   826,  1131,   121,  1169,   146,   830,  1175,   831,
    1476,   834,  1137,   226,   840,  1270,   835,   227,   842,   856,
    1481,   822,   845,  1666,   851,   857,   859,  -678,   615,   625,
     866,   614,   865,   616,   868,  1112,  1113,  1114,    36,   869,
    1138,   115,   872,   115,   876,   877,   625,   848,   850,   885,
     121,   208,   887,  1221,   890,   891,   893,   761,  1107,    48,
     896,   902,   903,   905,   906,   914,  1310,    60,    61,    62,
     163,   164,   407,  1315,   924,   487,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   888,   925,
     922,  -660,   926,  1620,  1701,   121,   898,   615,   952,   962,
     614,   972,   616,  1222,   974,   978,   979,   980,   981,   208,
    1223,   983,   996,   351,   997,   998,   121,  1000,    86,    87,
    1001,    88,   168,    90,  1033,  1043,   472,   473,  1051,    36,
    1045,  1055,  1064,  1062,   408,  1065,  1056,  1069,  1071,  1073,
    1102,  1248,   115,   114,  1105,   116,   919,   118,   119,   208,
      48,   208,  1111,  1090,   651,   438,   439,   440,   441,   442,
     443,  1099,   444,  1110,  -840,   651,  1223,  1126,  1122,  1745,
    1121,  1379,  1125,    36,   445,  1128,  1146,  1140,  1148,   208,
    1142,  1145,  1157,  1161,  1162,  1166,   946,  1165,   947,  1028,
    1217,  1280,   474,   475,    48,  1208,   939,   238,   945,   121,
    1220,   121,  1236,   910,   150,  1244,   364,  1294,  1237,    86,
      87,   146,    88,   168,    90,  1245,   966,  1249,  1253,   150,
     108,  1283,  1251,    36,  1255,  1660,  1257,  1661,  1267,  1258,
    1261,   208,   115,  1269,   968,   108,  1667,  1260,    36,  1268,
     365,   935,   625,  1279,    48,   625,   208,   208,  1281,   630,
     150,  1581,  1273,    86,    87,  1582,    88,   168,    90,    48,
    1282,  1284,  1289,  1441,  1293,  1299,   108,  1303,  1044,  1427,
    1301,  1304,  1037,  1307,  1308,  1309,  1040,  1041,  1311,  1313,
    1314,  1428,  1704,   586,  1054,  1347,  1318,  1343,  1354,  1348,
    1344,  1349,    36,  1355,  1359,   150,  1048,   410,  1370,  1356,
     121,  1361,  1357,    86,    87,  1369,    88,   168,    90,  1363,
     651,   108,  1365,    48,  1371,  1376,   150,  1324,    86,    87,
    1372,    88,   168,    90,  1375,  1378,   545,  1377,  1381,  1383,
    1384,  1428,   108,  1385,  1411,  1388,  1389,  1439,   723,  1400,
     758,  1424,  1451,  1444,  1450,  1454,   653,  1462,  1590,  1459,
    1460,  1477,  1466,  1479,   205,   205,  1485,   203,   203,  1494,
      13,  1493,    36,  1588,  1587,  1594,   208,   208,  1600,  1601,
    1341,  1603,    86,    87,   351,    88,   168,    90,  1613,  1341,
    1604,  1615,  1770,    48,  1605,  1624,  1115,  1115,   939,  1777,
     121,   331,   332,  1625,  1645,  1651,  1654,   150,    36,   150,
     898,   150,   115,   966,  1144,   625,  1655,   758,  1678,  1452,
    1684,  1662,   651,   108,  1680,   108,  1486,   108,  1433,    48,
    1692,  1694,  1325,  1693,  1699,  1700,  1708,  1326,  1707,    60,
      61,    62,   163,  1327,   407,  1328,  -338,  1159,  1710,   333,
    1711,  1714,    86,    87,  1640,    88,   168,    90,  1022,  1023,
    1024,  1025,  1026,  1027,   545,  1715,  1718,   545,  1721,  1336,
    1724,  1723,  1725,  1730,  1737,   115,  1740,  1028,  1336,  1413,
    1744,  1474,  1329,  1330,   115,  1331,   404,  1752,    86,    87,
    1754,    88,   168,    90,  1743,    36,   807,   846,   847,   208,
    1758,  1759,  1210,  1774,  1769,  1211,   408,  1782,  1775,  1783,
     150,  1788,  1789,  1791,  1345,  1602,    48,  1792,   821,  1036,
    1653,  1739,  1341,   703,  1039,   698,   108,  1101,  1341,    36,
    1341,   700,  1061,   625,  1595,  1679,  1247,  1753,  1286,  1751,
    1619,   824,  1341,  1390,   208,   202,   202,  1611,   205,   218,
      48,   203,  1633,  1492,  1638,  1431,  1484,  1776,  1764,   208,
     208,  1650,  1412,   115,  1609,  1617,  1474,  1168,  1231,   115,
     121,  1117,  1414,   218,   115,    86,    87,  1271,    88,   168,
      90,  1278,  1272,  1129,  1081,  1415,  1416,   588,   150,   939,
    1402,   986,   458,   939,   652,  1716,   586,   966,  1213,     0,
     150,  1160,  1207,   167,   108,     0,    84,  1417,   318,    86,
      87,  1336,    88,  1418,    90,     0,   108,  1336,     0,  1336,
       0,     0,     0,     0,    36,     0,  1341,  1648,     0,     0,
       0,  1336,  1324,   121,   208,     0,     0,     0,     0,    36,
       0,     0,   121,  1728,     0,    48,   487,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,     0,
      48,     0,     0,     0,     0,  1656,     0,     0,     0,     0,
       0,   586,     0,     0,     0,    13,   205,     0,     0,   203,
    1320,     0,     0,     0,     0,   205,     0,     0,   203,     0,
       0,    36,   205,     0,     0,   203,     0,   472,   473,   205,
     410,   553,   203,     0,    86,    87,     0,    88,   168,    90,
     613,     0,    48,   115,     0,  1336,   557,     0,     0,    86,
      87,   121,    88,   168,    90,     0,     0,   121,     0,   202,
       0,     0,   121,     0,   939,     0,   939,  1325,     0,     0,
       0,     0,  1326,     0,    60,    61,    62,   163,  1327,   407,
    1328,     0,     0,   115,   115,     0,     0,     0,     0,     0,
     115,     0,     0,   474,   475,   167,     0,     0,    84,    85,
     150,    86,    87,     0,    88,   168,    90,     0,   218,     0,
     218,     0,     0,     0,    36,     0,   108,  1329,  1330,     0,
    1331,     0,   317,     0,     0,     0,     0,   115,  1429,     0,
       0,     0,     0,     0,  1697,    48,     0,     0,     0,    36,
       0,   408,     0,     0,     0,     0,     0,     0,     0,  1457,
     752,     0,   205,     0,     0,   203,     0,     0,     0,     0,
      48,     0,     0,   150,     0,   218,     0,     0,   150,  1784,
       0,     0,   150,   939,     0,     0,     0,  1790,     0,   108,
       0,     0,     0,  1793,   108,     0,  1794,   202,   108,     0,
    1324,   333,     0,   115,    86,    87,   202,    88,   168,    90,
     115,   121,   351,   202,     0,   545,     0,     0,   317,   625,
     202,     0,  1324,     0,     0,     0,     0,     0,  1576,    86,
      87,   218,    88,   168,    90,  1583,     0,     0,   625,     0,
       0,     0,   317,    13,   317,     0,   625,     0,     0,     0,
     317,   121,   121,     0,     0,     0,   218,  1324,   121,   218,
       0,   150,   150,   150,     0,    13,     0,   150,     0,     0,
       0,     0,   150,   939,     0,     0,     0,   108,   108,   108,
       0,     0,     0,   108,     0,     0,     0,     0,   108,     0,
       0,     0,     0,     0,     0,   121,     0,     0,     0,     0,
      13,     0,  1729,   218,     0,  1325,     0,     0,     0,     0,
    1326,     0,    60,    61,    62,   163,  1327,   407,  1328,     0,
       0,     0,     0,     0,     0,   205,     0,  1325,   203,     0,
       0,     0,  1326,     0,    60,    61,    62,   163,  1327,   407,
    1328,     0,     0,   202,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1329,  1330,     0,  1331,     0,
       0,   121,  1325,     0,     0,     0,     0,  1326,   121,    60,
      61,    62,   163,  1327,   407,  1328,  1324,  1329,  1330,   408,
    1331,     0,     0,   205,     0,     0,   203,  1461,     0,     0,
       0,     0,     0,     0,     0,   218,   218,   545,     0,   800,
       0,   408,     0,     0,     0,     0,     0,     0,     0,  1463,
       0,   150,  1329,  1330,     0,  1331,     0,     0,   317,    13,
       0,     0,   939,   205,     0,   205,   203,   108,   203,     0,
     800,     0,     0,   204,   204,  1671,   408,   220,     0,     0,
       0,     0,  1576,  1576,  1469,     0,  1583,  1583,     0,     0,
       0,   150,   150,   205,     0,     0,   203,     0,   150,     0,
     351,     0,     0,     0,     0,     0,     0,   108,   108,     0,
       0,     0,     0,     0,   108,     0,   218,   218,     0,     0,
       0,  1325,     0,     0,     0,   218,  1326,     0,    60,    61,
      62,   163,  1327,   407,  1328,   150,     0,     0,    60,    61,
      62,    63,    64,   407,     0,   205,   202,     0,   203,    70,
     451,   108,     0,     0,     0,     0,     0,  1727,     0,     0,
     205,   205,     0,   203,   203,     0,     0,     0,     0,     0,
       0,  1329,  1330,     0,  1331,  1741,     0,     0,     0,     0,
       0,     0,     0,     0,   613,   452,     0,   453,     0,     0,
     259,     0,     0,     0,     0,   408,     0,     0,     0,     0,
     454,   150,   455,  1612,   202,   408,     0,     0,   150,    60,
      61,    62,    63,    64,   407,     0,     0,   108,   261,     0,
      70,   451,     0,     0,   108,     0,   487,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,     0,
      36,     0,     0,     0,   202,     0,   202,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   204,   453,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,   538,
       0,     0,     0,     0,   202,   800,   408,   472,   473,     0,
     205,   205,     0,   203,   203,     0,     0,     0,   218,   218,
     800,   800,   800,   800,   800,     0,   532,   533,   800,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
       0,     0,     0,     0,   167,     0,   613,    84,   311,     0,
      86,    87,     0,    88,   168,    90,   202,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   315,   218,
       0,   202,   202,   474,   475,     0,     0,     0,   316,     0,
       0,     0,     0,     0,     0,   218,   218,     0,     0,     0,
       0,     0,     0,     0,     0,   218,     0,     0,     0,     0,
       0,   218,     0,     0,     0,     0,     0,    33,    34,    35,
       0,     0,     0,     0,   218,   204,     0,   324,     0,   198,
       0,     0,   800,     0,   204,   218,     0,     0,     0,     0,
     832,   204,     0,   205,     0,     0,   203,     0,   204,     0,
       0,     0,     0,   218,     0,     0,     0,   218,     0,   204,
       0,     0,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,     0,     0,     0,    74,    75,
      76,    77,    78,   613,     0,     0,     0,     0,   205,   200,
       0,   203,     0,     0,     0,    82,    83,     0,     0,     0,
       0,   202,   202,   205,   205,     0,   203,   203,     0,    92,
       0,     0,     0,   472,   473,   218,     0,     0,   218,     0,
     218,     0,     0,    97,     0,     0,     0,     0,     0,     0,
       0,   220,     0,     0,     0,   800,     0,   218,     0,     0,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   417,   418,
     419,   204,     0,     0,     0,     0,     0,     0,   205,   474,
     475,   203,   800,     0,     0,     0,     0,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   218,   444,   218,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   202,   445,     0,   803,     0,     0,
       0,     0,   534,   259,   535,    36,     0,     0,     0,     0,
       0,   218,     0,     0,   218,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,   803,     0,
       0,   261,     0,     0,   218,     0,     0,   613,     0,   202,
       0,     0,     0,     0,     0,     0,     0,     0,  1414,     0,
       0,     0,     0,    36,   202,   202,     0,   800,     0,   539,
       0,  1415,  1416,   320,     0,     0,   218,     0,     0,     0,
     218,     0,     0,   800,    48,   800,     0,     0,     0,   167,
       0,     0,    84,    85,     0,    86,    87,     0,    88,  1418,
      90,   800,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   204,     0,   613,     0,     0,   532,
     533,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1224,   218,   218,     0,   218,   167,     0,   202,
      84,   311,     0,    86,    87,     0,    88,   168,    90,     0,
     646,     0,     0,   324,     0,     0,   417,   418,   419,     0,
       0,   315,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   316,   204,     0,     0,   420,     0,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   204,   445,   204,     0,     0,     0,     0,     0,
     218,     0,   218,     0,     0,     0,     0,   800,   218,     0,
       0,   800,     0,   800,     0,     0,   800,     0,     0,     0,
       0,     0,   204,   803,   218,   218,     0,     0,   218,     0,
       0,     0,     0,   929,   930,   218,     0,     0,   803,   803,
     803,   803,   803,     0,     0,     0,   803,     0,     0,     0,
       0,     0,     0,   931,     0,     0,     0,  1032,   320,     0,
     320,   932,   933,   934,    36,     0,     0,     0,     0,   782,
       0,     0,     0,   935,   204,     0,     0,   218,     0,     0,
       0,     0,     0,     0,     0,    48,     0,  1050,     0,   204,
     204,     0,     0,   800,     0,     0,     0,     0,     0,     0,
       0,     0,   218,   218,  1050,     0,     0,     0,     0,     0,
     218,     0,   218,   204,     0,   320,     0,   852,     0,     0,
     936,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   937,   218,     0,   218,     0,     0,     0,
     803,     0,   218,  1091,    86,    87,     0,    88,   168,    90,
     870,   871,     0,     0,     0,     0,     0,     0,     0,   879,
       0,     0,   938,     0,     0,   220,     0,     0,     0,   206,
     206,     0,     0,   224,     0,     0,     0,     0,   800,   800,
       0,     0,     0,     0,   800,     0,   218,     0,     0,  1002,
    1003,  1004,   218,     0,   218,     0,   320,     0,     0,   320,
       0,     0,     0,     0,     0,     0,     0,     0,  1005,   204,
     204,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   803,     0,   204,  1028,     0,   803,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   803,   803,   803,   803,   803,     0,  -969,  -969,  -969,
    -969,  -969,   436,   437,   438,   439,   440,   441,   442,   443,
     803,   444,     0,   218,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   445,     0,     0,     0,     0,     0,     0,
     218,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   646,   646,     0,     0,     0,   218,     0,     0,
       0,     0,   204,   800,     0,   320,   785,     0,   259,   801,
       0,     0,     0,     0,   800,     0,     0,     0,     0,     0,
     800,     0,     0,   206,   800,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   261,  1173,     0,     0,
     801,     0,   204,     0,     0,   218,     0,   204,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,  1060,
       0,     0,   204,   204,     0,   803,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1070,     0,     0,     0,    48,
       0,   803,     0,   803,   800,     0,   320,   320,  1084,     0,
       0,     0,     0,   218,     0,   320,     0,     0,     0,   803,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
       0,     0,     0,     0,   532,   533,     0,  1104,   218,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   167,   218,  1323,    84,   311,   204,    86,    87,
       0,    88,   168,    90,     0,   984,     0,     0,     0,     0,
       0,   206,     0,     0,     0,     0,   315,     0,     0,     0,
     206,     0,     0,     0,     0,     0,   316,   206,     0,     0,
     417,   418,   419,     0,   206,     0,     0,     0,     0,  1156,
       0,     0,  1158,     0,     0,   224,     0,     0,     0,   420,
       0,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   803,   204,   445,     0,   803,
       0,   803,     0,     0,   803,     0,     0,     0,     0,     0,
       0,     0,     0,  1410,     0,   801,  1423,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   224,   320,   320,
     801,   801,   801,   801,   801,     0,     0,     0,   801,     0,
       0,     0,     0,     0,     0,   805,  1239,     0,   879,     0,
       0,   487,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,     0,   204,     0,   206,     0,   259,
       0,     0,     0,     0,     0,  1259,   825,     0,  1262,     0,
       0,   803,     0,     0,     0,     0,     0,     0,     0,     0,
    1482,  1483,     0,     0,     0,   320,     0,   261,     0,     0,
    1423,     0,   472,   473,     0,     0,     0,     0,     0,     0,
       0,   320,     0,     0,     0,     0,     0,     0,     0,    36,
       0,   884,     0,   804,   320,     0,     0,     0,     0,     0,
    1300,     0,   801,     0,  1084,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,  -385,     0,
       0,     0,     0,   320,   804,     0,    60,    61,    62,   163,
     164,   407,     0,     0,     0,     0,   803,   803,   474,   475,
       0,     0,   803,     0,  1627,   532,   533,     0,     0,     0,
       0,     0,  1423,     0,     0,     0,     0,  1321,  1322,     0,
       0,     0,     0,   167,     0,     0,    84,   311,     0,    86,
      87,     0,    88,   168,    90,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   320,     0,   315,   320,     0,
     785,     0,     0,   408,     0,     0,     0,   316,     0,     0,
     206,     0,     0,     0,     0,   801,     0,     0,     0,     0,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,     0,     0,
       0,     0,     0,     0,  1392,     0,  1393,     0,     0,     0,
       0,   967,   801,     0,     0,   259,     0,     0,   206,     0,
       0,     0,     0,     0,     0,     0,   989,   990,   991,   992,
       0,     0,     0,     0,   999,     0,     0,     0,     0,  1434,
       0,     0,   320,   261,   320,     0,     0,     0,     0,     0,
       0,   803,     0,     0,     0,     0,     0,     0,   206,     0,
     206,     0,   803,     0,     0,    36,     0,     0,   803,     0,
       0,   320,   803,     0,   320,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,   206,   804,
    -969,  -969,  -969,  -969,  -969,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,     0,   804,   804,   804,   804,   804,     0,
       0,     0,   804,     0,     0,     0,  1028,   801,     0,     0,
       0,   532,   533,     0,     0,     0,   320,     0,  1088,     0,
     320,     0,   803,   801,     0,   801,     0,     0,     0,   167,
     206,  1738,    84,   311,     0,    86,    87,     0,    88,   168,
      90,   801,  1302,     0,     0,   206,   206,  1410,     0,     0,
       0,     0,     0,   315,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   316,     0,     0,     0,     0,     0,   224,
       0,     0,     0,   320,   320,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1629,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   804,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   224,     0,     0,     0,     0,  1176,  1179,  1180,  1181,
    1183,  1184,  1185,  1186,  1187,  1188,  1189,  1190,  1191,  1192,
    1193,  1194,  1195,  1196,  1197,  1198,  1199,  1200,  1201,  1202,
    1203,  1204,  1205,  1206,     0,     0,     0,     0,     0,     0,
     320,     0,   320,     0,     0,   206,   206,   801,  1212,     0,
       0,   801,     0,   801,     0,     0,   801,     0,     0,     0,
       0,     0,     0,     0,   320,     0,     0,  1652,     0,     0,
       0,     0,     0,     0,     0,   320,     0,     0,     0,   804,
       0,   224,     0,     0,   804,   804,   804,   804,   804,   804,
     804,   804,   804,   804,   804,   804,   804,   804,   804,   804,
     804,   804,   804,   804,   804,   804,   804,   804,   804,   804,
     804,   804,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   804,     0,     0,     0,
       0,     0,     0,   801,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1712,
     320,     0,     0,  1290,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   206,  1305,
       0,  1306,     0,     0,   320,     0,   320,     0,     0,     0,
       0,     0,   320,     0,     0,     0,     0,  1316,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   224,   444,
       0,     0,     0,   206,     0,     0,     0,     0,   801,   801,
       0,   445,   879,     0,   801,     0,     0,     0,   206,   206,
       0,   804,   320,     0,     0,     0,     0,   879,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   804,     0,   804,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   804,     0,     0,     0,   420,
       0,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,     0,     0,
       0,     0,     0,   206,     0,     0,     0,   445,     0,     0,
       0,     0,     0,  1395,     0,     0,     0,  1397,     0,  1398,
       0,     0,  1399,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   320,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     320,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1672,     0,     0,
       0,     0,     0,   801,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   801,     0,     0,     0,     0,     0,
     801,   804,   224,     0,   801,   804,     0,   804,     0,  1478,
     804,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   320,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   250,   251,     0,   252,
     253,   923,     0,   254,   255,   256,   257,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     258,     0,     0,     0,   801,     0,     0,     0,     0,     0,
       0,   224,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   804,   260,     0,
       0,     0,     0,     0,  1621,  1622,     0,     0,   320,     0,
    1626,     0,   262,   263,   264,   265,   266,   267,   268,     0,
       0,     0,    36,   320,   197,     0,     0,     0,     0,     0,
       0,     0,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,    48,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,     0,     0,     0,
     693,   304,   305,   306,     0,     0,     0,   307,   542,   543,
       0,     0,   804,   804,     0,     0,     0,     0,   804,     0,
       0,     0,     0,     0,     0,     0,   544,  1632,     0,     0,
       0,     0,    86,    87,     0,    88,   168,    90,   312,     0,
     313,     0,     0,   314,     0,   417,   418,   419,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   694,   420,   103,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,  1681,
     444,     0,     0,     0,     0,   417,   418,   419,     0,     0,
    1691,     0,   445,     0,     0,     0,  1696,     0,     0,     0,
    1698,     0,     0,     0,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,     0,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,   445,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   804,     0,     0,
    1731,    11,    12,     0,     0,     0,     0,     0,   804,     0,
       0,     0,     0,     0,   804,     0,     0,     0,   804,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,  1713,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,   927,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,   804,    50,
      51,    52,    53,    54,    55,    56,     0,    57,    58,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
      69,    70,    71,     0,     0,     0,  1042,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,    81,    82,    83,
      84,    85,     0,    86,    87,     0,    88,    89,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,    95,     0,    96,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1058,   103,   104,     0,   105,
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
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1225,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,   632,   103,   104,
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
     102,  1031,   103,   104,     0,   105,   106,     5,     6,     7,
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
       0,   100,     0,   101,   102,  1072,   103,   104,     0,   105,
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
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1139,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,  1141,    45,     0,    46,     0,    47,
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
       0,    46,     0,    47,  1291,     0,    48,    49,     0,     0,
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
     102,  1401,   103,   104,     0,   105,   106,     5,     6,     7,
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
       0,   100,     0,   101,   102,  1623,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,  1668,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,    91,     0,     0,    92,     0,     0,    93,
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
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,  1702,   103,   104,     0,   105,   106,     5,
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
      99,     0,     0,   100,     0,   101,   102,  1703,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,  1706,    46,     0,    47,     0,     0,
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
       0,   100,     0,   101,   102,  1722,   103,   104,     0,   105,
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
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1778,
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
       0,   101,   102,  1785,   103,   104,     0,   105,   106,     5,
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
      99,     0,     0,   100,     0,   101,   102,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     515,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    11,    12,     0,   770,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,   969,     0,
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
      12,     0,  1473,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    11,    12,     0,  1616,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   379,    12,     0,     0,     0,     0,     0,     0,     0,
     705,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   100,     0,     0,     0,     0,   103,   104,     0,   105,
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
       0,     0,     0,     0,     0,    10,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,  1027,     0,     0,   647,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1028,
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
       0,    86,    87,     0,    88,   168,    90,     0,   648,     0,
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
      99,     0,     0,   169,     0,     0,   765,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,  1085,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   445,     0,    14,    15,
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
      87,     0,    88,   168,    90,     0,  1086,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   169,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   379,    12,     0,     0,     0,     0,     0,     0,     0,
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
       0,   100,     0,   417,   418,   419,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   420,     0,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     445,     0,    16,     0,    17,    18,    19,    20,    21,    22,
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
       0,     0,     0,     0,  1098,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   169,     0,     0,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1027,     0,     0,     0,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1028,     0,
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
       0,     0,    60,    61,    62,   163,   164,   165,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     166,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,  1109,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   169,     0,   245,   418,   419,   103,   104,
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
       0,   379,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   100,     0,   417,   418,   419,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   420,     0,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     445,     0,    16,     0,    17,    18,    19,    20,    21,    22,
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
       0,     0,     0,     0,  1133,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   169,   513,     0,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   660,     0,     0,     0,     0,     0,     0,     0,
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
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
       0,     0,   705,     0,     0,     0,     0,     0,     0,     0,
       0,   445,     0,     0,    14,    15,     0,     0,     0,     0,
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
       0,     0,     0,    10,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,     0,     0,     0,     0,   745,     0,     0,     0,
       0,     0,     0,     0,     0,  1028,     0,     0,    14,    15,
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
       8,     9,     0,     0,     0,     0,     0,    10,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
     747,     0,     0,     0,     0,     0,     0,     0,   445,     0,
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
       0,    10,  -969,  -969,  -969,  -969,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,     0,     0,     0,  1130,     0,     0,     0,     0,     0,
       0,     0,   445,     0,     0,     0,    14,    15,     0,     0,
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
      97,    98,    99,     0,     0,   169,     0,   417,   418,   419,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   420,     0,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,     0,   444,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   445,     0,    16,     0,    17,    18,
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
      92,     0,     0,    93,     0,     0,     0,     0,  1446,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   169,
       0,   417,   418,   419,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   445,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,   594,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   163,   164,   165,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     166,    73,     0,    74,    75,    76,    77,    78,   250,   251,
       0,   252,   253,     0,    80,   254,   255,   256,   257,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,     0,   258,     0,    92,     0,     0,    93,     0,     0,
       0,     0,  1447,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   169,     0,     0,     0,     0,   103,   104,
     260,   105,   106,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   262,   263,   264,   265,   266,   267,
     268,     0,     0,     0,    36,     0,   197,     0,     0,     0,
       0,     0,     0,     0,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    48,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,     0,
       0,     0,   303,   304,   305,   306,     0,     0,     0,   307,
     542,   543,     0,     0,     0,     0,     0,   250,   251,     0,
     252,   253,     0,     0,   254,   255,   256,   257,   544,     0,
       0,     0,     0,     0,    86,    87,     0,    88,   168,    90,
     312,   258,   313,   259,     0,   314,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,     0,     0,   694,     0,   103,     0,   260,
       0,   261,     0,     0,     0,     0,  1028,     0,     0,     0,
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
       0,   316,     0,     0,     0,  1596,     0,     0,   260,     0,
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
     316,     0,     0,     0,  1664,     0,     0,   260,     0,   261,
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
       0,   314,     0,     0,     0,     0,     0,     0,   315,  1405,
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
     314,  1497,  1498,  1499,  1500,  1501,     0,   315,  1502,  1503,
    1504,  1505,     0,     0,     0,     0,     0,   316,     0,     0,
       0,     0,     0,     0,     0,  1506,  1507,     0,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,  1508,   444,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   445,  1509,  1510,  1511,
    1512,  1513,  1514,  1515,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1516,  1517,  1518,
    1519,  1520,  1521,  1522,  1523,  1524,  1525,  1526,    48,  1527,
    1528,  1529,  1530,  1531,  1532,  1533,  1534,  1535,  1536,  1537,
    1538,  1539,  1540,  1541,  1542,  1543,  1544,  1545,  1546,  1547,
    1548,  1549,  1550,  1551,  1552,  1553,  1554,  1555,  1556,     0,
       0,     0,  1557,  1558,     0,  1559,  1560,  1561,  1562,  1563,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1564,  1565,  1566,     0,     0,     0,    86,    87,     0,
      88,   168,    90,  1567,     0,  1568,  1569,     0,  1570,   417,
     418,   419,     0,     0,     0,  1571,  1572,     0,  1573,     0,
    1574,  1575,     0,     0,     0,     0,     0,     0,   420,  1295,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,   417,   418,   419,     0,     0,
       0,     0,     0,     0,     0,     0,   445,     0,     0,     0,
       0,     0,     0,     0,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,   417,   418,   419,     0,     0,     0,     0,     0,     0,
       0,     0,   445,     0,     0,     0,     0,     0,     0,     0,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   445,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   417,   418,
     419,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   420,  1296,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,   417,   418,   419,     0,     0,     0,
       0,     0,     0,     0,     0,   445,     0,     0,     0,     0,
       0,     0,     0,   420,   446,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
       0,   445,     0,     0,     0,     0,     0,     0,     0,   420,
     529,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   417,   418,   419,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   420,   531,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,     0,   444,   417,   418,   419,     0,     0,     0,     0,
       0,     0,     0,     0,   445,     0,     0,     0,     0,     0,
       0,     0,   420,   548,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   250,   251,
     445,   252,   253,  1003,  1004,   254,   255,   256,   257,   552,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1005,   258,     0,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,  1027,     0,     0,     0,     0,
     260,     0,     0,     0,     0,     0,     0,     0,     0,  1028,
       0,     0,     0,     0,   262,   263,   264,   265,   266,   267,
     268,     0,     0,     0,    36,   737,     0,     0,     0,     0,
       0,     0,     0,     0,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    48,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,     0,
       0,   762,   303,   304,   305,   306,     0,     0,     0,   307,
     542,   543,   250,   251,     0,   252,   253,     0,     0,   254,
     255,   256,   257,     0,     0,     0,     0,     0,   544,     0,
       0,     0,     0,     0,    86,    87,   258,    88,   168,    90,
     312,     0,   313,     0,     0,   314,     0,  -969,  -969,  -969,
    -969,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,   260,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1028,   262,   263,
     264,   265,   266,   267,   268,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,    48,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,     0,     0,     0,  1174,   304,   305,   306,
       0,     0,     0,   307,   542,   543,   250,   251,     0,   252,
     253,     0,     0,   254,   255,   256,   257,     0,     0,     0,
       0,     0,   544,     0,     0,     0,     0,     0,    86,    87,
     258,    88,   168,    90,   312,     0,   313,     0,     0,   314,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   260,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   262,   263,   264,   265,   266,   267,   268,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,    48,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,     0,     0,     0,
       0,   304,   305,   306,  1182,     0,     0,   307,   542,   543,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   791,   792,     0,     0,   544,     0,   793,     0,
     794,     0,    86,    87,     0,    88,   168,    90,   312,     0,
     313,     0,   795,   314,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    36,     0,     0,     0,     0,     0,   417,
     418,   419,   198,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   963,   444,     0,     0,     0,     0,   796,
       0,    74,    75,    76,    77,    78,   445,     0,     0,     0,
       0,     0,   200,     0,     0,     0,     0,   167,    82,    83,
      84,   797,     0,    86,    87,    28,    88,   168,    90,     0,
       0,     0,    92,    33,    34,    35,    36,     0,   197,     0,
       0,   798,     0,     0,     0,   198,    97,     0,     0,     0,
       0,   799,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   490,     0,     0,     0,     0,     0,   199,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   964,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,   791,   792,     0,    92,     0,     0,   793,     0,
     794,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,   795,     0,   201,     0,     0,     0,     0,   103,
      33,    34,    35,    36,     0,     0,     0,     0,     0,   417,
     418,   419,   198,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,     0,     0,     0,   796,
       0,    74,    75,    76,    77,    78,   445,     0,     0,     0,
       0,     0,   200,     0,     0,     0,     0,   167,    82,    83,
      84,   797,     0,    86,    87,    28,    88,   168,    90,     0,
       0,     0,    92,    33,    34,    35,    36,     0,   197,     0,
       0,   798,     0,     0,     0,   198,    97,     0,     0,     0,
       0,   799,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   499,     0,     0,     0,     0,     0,   199,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,    28,    88,
     168,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   197,     0,     0,     0,     0,     0,     0,   198,    97,
       0,     0,     0,     0,   201,     0,     0,   565,     0,   103,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   199,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   585,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   200,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,    28,     0,   918,    92,     0,
       0,     0,     0,    33,    34,    35,    36,     0,   197,     0,
       0,     0,    97,     0,     0,   198,     0,   201,     0,     0,
       0,     0,   103,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   199,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,    28,    88,
     168,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   197,     0,     0,     0,     0,     0,     0,   198,    97,
       0,     0,     0,     0,   201,     0,     0,     0,     0,   103,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   199,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1053,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   200,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,    28,    88,   168,    90,     0,     0,     0,    92,    33,
      34,    35,    36,     0,   197,     0,     0,     0,     0,     0,
       0,   198,    97,     0,     0,     0,     0,   201,     0,     0,
       0,     0,   103,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   199,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,     0,     0,
       0,    92,     0,     0,     0,   417,   418,   419,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,     0,     0,
     201,     0,     0,     0,   420,   103,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,   417,   418,   419,     0,     0,     0,     0,     0,     0,
       0,     0,   445,     0,     0,     0,     0,     0,     0,     0,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   445,     0,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   904,   420,
       0,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,  1002,  1003,  1004,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
       0,     0,     0,     0,   948,  1005,     0,     0,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1028,     0,  1002,  1003,  1004,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1254,  1005,     0,     0,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1002,
    1003,  1004,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1028,     0,     0,     0,     0,     0,  1005,  1164,
       0,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1028,  1002,  1003,  1004,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1005,     0,  1312,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,     0,     0,     0,    33,    34,    35,    36,     0,   197,
       0,     0,     0,     0,  1028,     0,   198,     0,     0,     0,
       0,     0,  1394,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   215,
       0,     0,     0,     0,     0,   216,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
    1480,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,    33,    34,    35,
      36,     0,   197,     0,     0,     0,     0,     0,     0,   608,
      97,     0,     0,     0,     0,   217,     0,     0,     0,     0,
     103,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   199,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,     0,     0,     0,    92,
      33,    34,    35,    36,     0,   197,     0,     0,     0,     0,
       0,     0,   198,    97,     0,     0,     0,     0,   609,     0,
       0,     0,     0,   103,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   215,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   200,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,     0,     0,     0,   417,   418,   419,     0,
       0,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,   217,     0,     0,   774,   420,   103,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   445,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   417,   418,   419,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   775,   420,   901,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,     0,   444,   417,   418,   419,     0,     0,     0,     0,
       0,     0,     0,     0,   445,     0,     0,     0,     0,     0,
       0,     0,   420,     0,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,  1002,
    1003,  1004,     0,     0,     0,     0,     0,     0,     0,     0,
     445,     0,     0,     0,     0,     0,     0,     0,  1005,  1317,
       0,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1002,  1003,  1004,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1028,     0,     0,     0,
       0,     0,  1005,     0,     0,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,   419,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1028,     0,     0,     0,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,  1004,
     444,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   445,     0,     0,     0,  1005,     0,     0,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1005,     0,  1028,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1028,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   445,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1028,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   445,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1028
};

static const yytype_int16 yycheck[] =
{
       5,     6,    54,     8,     9,    10,    11,    12,   146,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    54,   121,    28,    29,   650,   171,     4,     4,    32,
       4,   372,     4,     4,   372,    30,   372,    42,   100,    30,
    1075,    44,   503,    55,   218,    50,    49,    52,   213,   621,
      55,   877,    57,   620,   151,   146,   866,   100,   444,   601,
     480,   481,    42,   780,     9,    30,   897,   962,   100,    30,
     773,   229,  1062,   172,    79,   515,     9,   100,     9,   749,
     476,    14,   913,    14,     9,     9,     9,   739,   476,   509,
      14,    47,     4,     9,   702,   100,     4,    47,     9,    79,
       9,    30,     9,  1071,    58,     9,     9,     9,     9,   549,
       9,     9,     9,     9,     9,   511,     9,     9,   949,     9,
      68,     9,     9,   511,     9,     9,   169,    81,     9,     9,
      84,   484,     9,   230,    81,    47,    87,   169,    87,     9,
       9,   158,    47,    81,   121,     9,   169,    34,  1610,     9,
     103,     0,    30,   112,   152,    68,   152,   187,   201,    81,
      99,    47,   119,   187,   169,   131,   132,   508,    36,   201,
     127,   176,    91,   190,   217,   173,    95,    96,   201,    36,
     120,    55,   622,   131,   132,     4,    47,   127,   998,   187,
     187,   187,   190,    67,   217,   172,   201,   109,   149,   357,
     149,   120,   114,  1665,   116,   117,   118,   119,   120,   121,
     122,   170,   217,    81,    68,    68,    68,   156,    68,   166,
      68,   187,    36,    68,    81,    68,   231,    68,    68,   234,
     152,    68,   188,     8,    68,   185,   241,   242,   188,    36,
     173,    68,   190,    68,   189,   190,   916,   159,   160,   174,
     162,  1146,   190,    68,   192,   972,   189,   974,   189,   235,
     189,  1251,    68,   239,   409,   189,   189,    81,  1258,    68,
    1260,   183,   324,   189,   187,  1106,   174,   190,   189,   191,
     189,  1249,   189,   188,    81,   189,   189,   189,   189,  1279,
     189,   324,   189,   189,   189,    81,   189,   189,   166,   189,
     188,   188,   188,   188,   188,    99,   349,   188,   188,   166,
     174,   188,   190,   855,   174,  1125,    99,   349,   188,   188,
      81,    36,   894,    68,   185,   765,   349,   146,   493,   187,
     770,    68,   187,   187,   187,   187,   190,   152,   190,   187,
     190,   486,   190,   348,   349,   190,   152,   190,   400,   190,
     355,    36,   166,   190,   187,   360,   190,   131,   132,   458,
     152,   190,   156,   190,   187,   190,    81,   400,   348,   166,
      26,    27,   187,   156,   379,   190,    68,   331,   332,   333,
    1370,   173,   387,   187,   190,     4,   131,   132,   187,   188,
      81,  1359,   187,  1361,   399,   157,    81,   459,   450,    81,
     187,     4,    48,    49,    52,   166,   192,    26,    27,   386,
     364,   187,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,  1142,
     445,   187,   447,   448,   449,   497,   611,   459,    51,   444,
     187,    54,  1060,   444,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,    68,    71,   627,
     378,   629,   477,   478,  1126,   480,   481,   482,   483,   444,
     457,   100,   487,   444,   403,   490,    89,   883,    91,   842,
     910,  1459,    95,    96,   499,   883,   501,   450,   190,  1309,
     638,   192,   184,    81,   509,   517,    68,  1074,   190,   157,
      81,  1325,   517,   508,   519,    81,    87,   120,  1090,   103,
     104,  1093,   696,   476,   172,   866,    52,   692,   866,   969,
     866,   173,   918,   187,   453,   454,   455,   456,    99,    30,
     103,   104,   173,    81,   497,   187,   522,   638,    14,    87,
     169,   152,    99,   727,   210,   508,   187,   156,   511,   704,
     565,   187,    99,   108,    30,    81,   609,   386,  1378,   131,
     132,   116,   117,   118,   119,   120,   121,   131,   132,   150,
     151,   184,   201,    49,   189,   743,   744,   173,   166,   190,
     152,   210,   750,   751,    36,   156,    81,  1411,   217,   553,
     166,   187,    87,   557,   609,    81,   189,   191,   562,   156,
     189,    87,   150,   151,    29,  1267,   235,  1269,   152,   156,
     239,  1435,   787,  1437,   195,   187,   800,    79,   191,    73,
      74,   157,   235,   807,   150,   151,   239,   191,   183,   173,
     243,   189,    79,   648,    81,  1613,    81,  1108,   100,   189,
     156,  1259,    87,   750,   158,   660,   190,   998,   196,   189,
     998,    68,   998,   100,    79,   150,   151,   171,   448,    73,
      74,  1238,    68,    88,   150,   151,   190,    29,   173,  1099,
     152,  1253,   338,   189,   190,   100,   190,   187,    81,   694,
    1110,   347,   187,   349,    87,    81,    48,   477,   354,    51,
     187,   153,   482,    81,   156,   361,    68,   159,   160,    87,
     162,   163,   164,   621,  1749,   150,   151,   722,   152,   338,
     157,   324,   159,   160,  1376,   162,   163,   164,   347,  1764,
     349,   187,  1340,    99,   100,   354,   156,   656,   153,   191,
     173,   156,   361,   189,   159,   160,    46,   162,   163,   164,
     129,   130,   757,   190,   187,   192,   149,   150,   151,    79,
      29,    81,    67,   149,   150,   151,   769,   386,   773,  1209,
     173,   149,   150,   151,     9,  1743,   189,   190,   943,    48,
     100,   152,    51,   386,  1125,   187,   763,  1125,   187,  1125,
    1758,   358,  1145,   396,   194,   362,   152,   400,   152,  1371,
     403,     8,   778,   187,  1412,    51,    52,    53,   116,   117,
     118,   119,   120,   121,  1466,   189,   190,   869,   152,   638,
     189,    67,   389,   988,   391,   392,   393,   394,   484,   187,
     995,    14,   809,  1639,  1640,  1255,   152,   157,   189,   159,
     160,  1281,   162,   163,   164,   127,   851,   450,   451,   452,
     453,   454,   455,   456,   102,   103,   104,  1297,    14,   813,
     865,  1635,  1636,   817,   127,   484,   188,  1220,   789,   790,
     190,   173,   192,   476,    14,   183,    99,   854,   854,   188,
     854,   189,   854,   854,   188,   188,   891,   188,    51,    52,
      53,  1717,    55,   187,   497,   193,   901,   108,   875,   904,
     187,   906,   187,   522,    67,   910,     9,  1081,   511,   149,
    1736,   102,   103,   104,   188,   188,   869,   188,  1744,   522,
     116,   117,   118,   918,   104,   105,   106,   918,   881,   188,
     883,    91,     9,   189,    14,   173,   187,  1357,   541,     9,
     187,  1381,    81,   948,   763,   997,   854,   188,  1000,   188,
    1390,   188,   955,   918,   129,  1120,   189,   918,   187,    30,
    1400,   564,   188,  1615,    68,   130,   172,   152,  1309,   877,
       9,  1309,   133,  1309,   188,    76,    77,    78,    79,   152,
     956,   958,    14,   960,   185,     9,   894,   590,   591,     9,
     809,   647,   174,  1055,   188,     9,    14,   950,   917,   100,
     129,   194,   194,   191,     9,    14,  1171,   116,   117,   118,
     119,   120,   121,  1178,   188,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   647,   188,
     194,   152,   194,  1473,  1659,   854,   187,  1378,   188,    99,
    1378,   189,  1378,  1055,   189,    88,   133,   152,     9,   705,
    1055,   188,   187,   656,   152,   187,   875,   152,   159,   160,
     190,   162,   163,   164,   190,    14,    65,    66,   190,    79,
     189,    14,    14,   190,   183,   188,   194,   189,   185,    30,
      30,  1086,  1059,  1059,    14,  1059,   705,  1059,  1059,   745,
     100,   747,    14,   187,  1099,    48,    49,    50,    51,    52,
      53,   187,    55,   187,   187,  1110,  1111,     9,   187,  1734,
      50,  1276,   187,    79,    67,   188,   133,   189,    14,   775,
     189,   187,   133,     9,   188,     9,   745,   194,   747,    67,
       9,  1134,   131,   132,   100,    81,   739,  1142,   741,   958,
     187,   960,   133,   187,   763,    14,   156,  1152,   189,   159,
     160,  1059,   162,   163,   164,    81,   775,   188,   187,   778,
     763,  1137,   190,    79,   187,  1605,   188,  1607,   133,   190,
     189,   827,  1149,     9,   777,   778,  1616,   190,    79,   194,
     190,    88,  1090,   190,   100,  1093,   842,   843,    30,   188,
     809,   157,   149,   159,   160,   161,   162,   163,   164,   100,
      75,   189,   188,  1341,   189,   174,   809,    30,   827,   125,
     133,   188,   815,   188,   133,     9,   819,   820,   188,   191,
       9,   187,  1662,   842,   843,  1230,   188,   191,    14,  1234,
     190,  1236,    79,    81,   188,   854,   839,  1336,   190,  1244,
    1059,   188,   187,   159,   160,   189,   162,   163,   164,  1254,
    1255,   854,   188,   100,   187,   133,   875,     4,   159,   160,
     188,   162,   163,   164,   188,     9,   869,   188,    30,   189,
     188,   187,   875,   188,   109,   189,   189,   189,   881,   190,
     883,   161,    81,   157,    14,   114,   187,   190,  1433,   188,
     188,   188,   133,   133,    26,    27,    14,    26,    27,   190,
      47,   173,    79,    81,   189,    14,   962,   963,    14,    81,
    1218,   188,   159,   160,   917,   162,   163,   164,   188,  1227,
     187,   133,  1762,   100,   190,   189,   929,   930,   931,  1769,
    1149,   108,   109,   189,    14,    14,   189,   956,    79,   958,
     187,   960,  1319,   962,   963,  1253,    14,   950,     9,  1354,
      57,   190,  1357,   956,   191,   958,  1408,   960,  1335,   100,
      81,   187,   109,   173,    81,     9,   112,   114,   189,   116,
     117,   118,   119,   120,   121,   122,    99,   980,   152,   156,
      99,   164,   159,   160,    34,   162,   163,   164,    48,    49,
      50,    51,    52,    53,   997,    14,   187,  1000,   188,  1218,
     187,   189,   170,   174,    81,  1382,   167,    67,  1227,    29,
       9,  1387,   159,   160,  1391,   162,   157,    81,   159,   160,
     189,   162,   163,   164,   188,    79,  1029,    81,    82,  1085,
     188,   188,  1035,    14,   190,  1038,   183,    14,    81,    81,
    1059,    14,    81,    14,   191,  1450,   100,    81,   562,   813,
    1595,  1725,  1360,   456,   817,   451,  1059,   911,  1366,    79,
    1368,   453,   857,  1371,  1441,  1630,  1085,  1740,  1143,  1736,
    1472,   567,  1380,  1294,  1130,    26,    27,  1462,   210,    30,
     100,   210,  1495,  1413,  1579,  1331,  1405,  1768,  1756,  1145,
    1146,  1591,  1327,  1470,  1458,  1471,  1472,   996,  1063,  1476,
    1319,   930,   122,    54,  1481,   159,   160,  1121,   162,   163,
     164,  1130,  1122,   945,   881,   135,   136,   355,  1137,  1122,
    1320,   789,  1341,  1126,   400,  1693,  1145,  1146,  1046,    -1,
    1149,   981,  1029,   153,  1137,    -1,   156,   157,  1590,   159,
     160,  1360,   162,   163,   164,    -1,  1149,  1366,    -1,  1368,
      -1,    -1,    -1,    -1,    79,    -1,  1464,  1590,    -1,    -1,
      -1,  1380,     4,  1382,  1220,    -1,    -1,    -1,    -1,    79,
      -1,    -1,  1391,  1711,    -1,   100,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
     100,    -1,    -1,    -1,    -1,  1600,    -1,    -1,    -1,    -1,
      -1,  1220,    -1,    -1,    -1,    47,   338,    -1,    -1,   338,
    1213,    -1,    -1,    -1,    -1,   347,    -1,    -1,   347,    -1,
      -1,    79,   354,    -1,    -1,   354,    -1,    65,    66,   361,
    1729,   156,   361,    -1,   159,   160,    -1,   162,   163,   164,
     372,    -1,   100,  1620,    -1,  1464,   156,    -1,    -1,   159,
     160,  1470,   162,   163,   164,    -1,    -1,  1476,    -1,   210,
      -1,    -1,  1481,    -1,  1267,    -1,  1269,   109,    -1,    -1,
      -1,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,  1660,  1661,    -1,    -1,    -1,    -1,    -1,
    1667,    -1,    -1,   131,   132,   153,    -1,    -1,   156,   157,
    1319,   159,   160,    -1,   162,   163,   164,    -1,   259,    -1,
     261,    -1,    -1,    -1,    79,    -1,  1319,   159,   160,    -1,
     162,    -1,  1325,    -1,    -1,    -1,    -1,  1704,  1331,    -1,
      -1,    -1,    -1,    -1,  1653,   100,    -1,    -1,    -1,    79,
      -1,   183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   191,
     188,    -1,   484,    -1,    -1,   484,    -1,    -1,    -1,    -1,
     100,    -1,    -1,  1382,    -1,   316,    -1,    -1,  1387,  1774,
      -1,    -1,  1391,  1376,    -1,    -1,    -1,  1782,    -1,  1382,
      -1,    -1,    -1,  1788,  1387,    -1,  1791,   338,  1391,    -1,
       4,   156,    -1,  1770,   159,   160,   347,   162,   163,   164,
    1777,  1620,  1405,   354,    -1,  1408,    -1,    -1,  1411,  1717,
     361,    -1,     4,    -1,    -1,    -1,    -1,    -1,  1421,   159,
     160,   372,   162,   163,   164,  1428,    -1,    -1,  1736,    -1,
      -1,    -1,  1435,    47,  1437,    -1,  1744,    -1,    -1,    -1,
    1443,  1660,  1661,    -1,    -1,    -1,   397,     4,  1667,   400,
      -1,  1470,  1471,  1472,    -1,    47,    -1,  1476,    -1,    -1,
      -1,    -1,  1481,  1466,    -1,    -1,    -1,  1470,  1471,  1472,
      -1,    -1,    -1,  1476,    -1,    -1,    -1,    -1,  1481,    -1,
      -1,    -1,    -1,    -1,    -1,  1704,    -1,    -1,    -1,    -1,
      47,    -1,  1711,   444,    -1,   109,    -1,    -1,    -1,    -1,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,    -1,    -1,    -1,   647,    -1,   109,   647,    -1,
      -1,    -1,   114,    -1,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,   484,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   159,   160,    -1,   162,    -1,
      -1,  1770,   109,    -1,    -1,    -1,    -1,   114,  1777,   116,
     117,   118,   119,   120,   121,   122,     4,   159,   160,   183,
     162,    -1,    -1,   705,    -1,    -1,   705,   191,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   536,   537,  1590,    -1,   540,
      -1,   183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   191,
      -1,  1620,   159,   160,    -1,   162,    -1,    -1,  1611,    47,
      -1,    -1,  1615,   745,    -1,   747,   745,  1620,   747,    -1,
     571,    -1,    -1,    26,    27,  1628,   183,    30,    -1,    -1,
      -1,    -1,  1635,  1636,   191,    -1,  1639,  1640,    -1,    -1,
      -1,  1660,  1661,   775,    -1,    -1,   775,    -1,  1667,    -1,
    1653,    -1,    -1,    -1,    -1,    -1,    -1,  1660,  1661,    -1,
      -1,    -1,    -1,    -1,  1667,    -1,   617,   618,    -1,    -1,
      -1,   109,    -1,    -1,    -1,   626,   114,    -1,   116,   117,
     118,   119,   120,   121,   122,  1704,    -1,    -1,   116,   117,
     118,   119,   120,   121,    -1,   827,   647,    -1,   827,   127,
     128,  1704,    -1,    -1,    -1,    -1,    -1,  1710,    -1,    -1,
     842,   843,    -1,   842,   843,    -1,    -1,    -1,    -1,    -1,
      -1,   159,   160,    -1,   162,  1728,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   866,   163,    -1,   165,    -1,    -1,
      29,    -1,    -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,
     178,  1770,   180,   191,   705,   183,    -1,    -1,  1777,   116,
     117,   118,   119,   120,   121,    -1,    -1,  1770,    57,    -1,
     127,   128,    -1,    -1,  1777,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      79,    -1,    -1,    -1,   745,    -1,   747,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   210,   165,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,   775,   776,   183,    65,    66,    -1,
     962,   963,    -1,   962,   963,    -1,    -1,    -1,   789,   790,
     791,   792,   793,   794,   795,    -1,   135,   136,   799,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   810,
      -1,    -1,    -1,    -1,   153,    -1,   998,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,   827,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,   840,
      -1,   842,   843,   131,   132,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,    -1,    -1,   856,   857,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   866,    -1,    -1,    -1,    -1,
      -1,   872,    -1,    -1,    -1,    -1,    -1,    76,    77,    78,
      -1,    -1,    -1,    -1,   885,   338,    -1,    54,    -1,    88,
      -1,    -1,   893,    -1,   347,   896,    -1,    -1,    -1,    -1,
     188,   354,    -1,  1085,    -1,    -1,  1085,    -1,   361,    -1,
      -1,    -1,    -1,   914,    -1,    -1,    -1,   918,    -1,   372,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,   137,   138,
     139,   140,   141,  1125,    -1,    -1,    -1,    -1,  1130,   148,
      -1,  1130,    -1,    -1,    -1,   154,   155,    -1,    -1,    -1,
      -1,   962,   963,  1145,  1146,    -1,  1145,  1146,    -1,   168,
      -1,    -1,    -1,    65,    66,   976,    -1,    -1,   979,    -1,
     981,    -1,    -1,   182,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   444,    -1,    -1,    -1,   996,    -1,   998,    -1,    -1,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,    10,    11,
      12,   484,    -1,    -1,    -1,    -1,    -1,    -1,  1220,   131,
     132,  1220,  1043,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,  1073,    55,  1075,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1085,    67,    -1,   540,    -1,    -1,
      -1,    -1,   259,    29,   261,    79,    -1,    -1,    -1,    -1,
      -1,  1102,    -1,    -1,  1105,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,   571,    -1,
      -1,    57,    -1,    -1,  1125,    -1,    -1,  1309,    -1,  1130,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    79,  1145,  1146,    -1,  1148,    -1,   316,
      -1,   135,   136,    54,    -1,    -1,  1157,    -1,    -1,    -1,
    1161,    -1,    -1,  1164,   100,  1166,    -1,    -1,    -1,   153,
      -1,    -1,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,  1182,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   647,    -1,  1378,    -1,    -1,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   194,  1214,  1215,    -1,  1217,   153,    -1,  1220,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
     397,    -1,    -1,   400,    -1,    -1,    10,    11,    12,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   187,   705,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   745,    67,   747,    -1,    -1,    -1,    -1,    -1,
    1301,    -1,  1303,    -1,    -1,    -1,    -1,  1308,  1309,    -1,
      -1,  1312,    -1,  1314,    -1,    -1,  1317,    -1,    -1,    -1,
      -1,    -1,   775,   776,  1325,  1326,    -1,    -1,  1329,    -1,
      -1,    -1,    -1,    48,    49,  1336,    -1,    -1,   791,   792,
     793,   794,   795,    -1,    -1,    -1,   799,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,   810,   259,    -1,
     261,    76,    77,    78,    79,    -1,    -1,    -1,    -1,   536,
      -1,    -1,    -1,    88,   827,    -1,    -1,  1378,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,   840,    -1,   842,
     843,    -1,    -1,  1394,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1403,  1404,   857,    -1,    -1,    -1,    -1,    -1,
    1411,    -1,  1413,   866,    -1,   316,    -1,   191,    -1,    -1,
     135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   148,  1435,    -1,  1437,    -1,    -1,    -1,
     893,    -1,  1443,   896,   159,   160,    -1,   162,   163,   164,
     617,   618,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   626,
      -1,    -1,   177,    -1,    -1,   918,    -1,    -1,    -1,    26,
      27,    -1,    -1,    30,    -1,    -1,    -1,    -1,  1479,  1480,
      -1,    -1,    -1,    -1,  1485,    -1,  1487,    -1,    -1,    10,
      11,    12,  1493,    -1,  1495,    -1,   397,    -1,    -1,   400,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   962,
     963,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   996,    -1,   998,    67,    -1,  1001,  1002,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1027,  1028,    -1,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
    1043,    55,    -1,  1594,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
    1611,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   789,   790,    -1,    -1,    -1,  1628,    -1,    -1,
      -1,    -1,  1085,  1634,    -1,   536,   537,    -1,    29,   540,
      -1,    -1,    -1,    -1,  1645,    -1,    -1,    -1,    -1,    -1,
    1651,    -1,    -1,   210,  1655,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,   188,    -1,    -1,
     571,    -1,  1125,    -1,    -1,  1676,    -1,  1130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,   856,
      -1,    -1,  1145,  1146,    -1,  1148,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   872,    -1,    -1,    -1,   100,
      -1,  1164,    -1,  1166,  1715,    -1,   617,   618,   885,    -1,
      -1,    -1,    -1,  1724,    -1,   626,    -1,    -1,    -1,  1182,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1740,
      -1,    -1,    -1,    -1,   135,   136,    -1,   914,  1749,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   153,  1764,  1217,   156,   157,  1220,   159,   160,
      -1,   162,   163,   164,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   338,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
     347,    -1,    -1,    -1,    -1,    -1,   187,   354,    -1,    -1,
      10,    11,    12,    -1,   361,    -1,    -1,    -1,    -1,   976,
      -1,    -1,   979,    -1,    -1,   372,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1308,  1309,    67,    -1,  1312,
      -1,  1314,    -1,    -1,  1317,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1326,    -1,   776,  1329,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   444,   789,   790,
     791,   792,   793,   794,   795,    -1,    -1,    -1,   799,    -1,
      -1,    -1,    -1,    -1,    -1,   540,  1073,    -1,  1075,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,  1378,    -1,   484,    -1,    29,
      -1,    -1,    -1,    -1,    -1,  1102,   571,    -1,  1105,    -1,
      -1,  1394,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1403,  1404,    -1,    -1,    -1,   856,    -1,    57,    -1,    -1,
    1413,    -1,    65,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   872,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,
      -1,   191,    -1,   540,   885,    -1,    -1,    -1,    -1,    -1,
    1157,    -1,   893,    -1,  1161,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,
      -1,    -1,    -1,   914,   571,    -1,   116,   117,   118,   119,
     120,   121,    -1,    -1,    -1,    -1,  1479,  1480,   131,   132,
      -1,    -1,  1485,    -1,  1487,   135,   136,    -1,    -1,    -1,
      -1,    -1,  1495,    -1,    -1,    -1,    -1,  1214,  1215,    -1,
      -1,    -1,    -1,   153,    -1,    -1,   156,   157,    -1,   159,
     160,    -1,   162,   163,   164,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   976,    -1,   177,   979,    -1,
     981,    -1,    -1,   183,    -1,    -1,    -1,   187,    -1,    -1,
     647,    -1,    -1,    -1,    -1,   996,    -1,    -1,    -1,    -1,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,    -1,    -1,
      -1,    -1,    -1,    -1,  1301,    -1,  1303,    -1,    -1,    -1,
      -1,   776,  1043,    -1,    -1,    29,    -1,    -1,   705,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   791,   792,   793,   794,
      -1,    -1,    -1,    -1,   799,    -1,    -1,    -1,    -1,  1336,
      -1,    -1,  1073,    57,  1075,    -1,    -1,    -1,    -1,    -1,
      -1,  1634,    -1,    -1,    -1,    -1,    -1,    -1,   745,    -1,
     747,    -1,  1645,    -1,    -1,    79,    -1,    -1,  1651,    -1,
      -1,  1102,  1655,    -1,  1105,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,   775,   776,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,   791,   792,   793,   794,   795,    -1,
      -1,    -1,   799,    -1,    -1,    -1,    67,  1148,    -1,    -1,
      -1,   135,   136,    -1,    -1,    -1,  1157,    -1,   893,    -1,
    1161,    -1,  1715,  1164,    -1,  1166,    -1,    -1,    -1,   153,
     827,  1724,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,  1182,   166,    -1,    -1,   842,   843,  1740,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,   866,
      -1,    -1,    -1,  1214,  1215,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1493,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   893,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   918,    -1,    -1,    -1,    -1,  1001,  1002,  1003,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1028,    -1,    -1,    -1,    -1,    -1,    -1,
    1301,    -1,  1303,    -1,    -1,   962,   963,  1308,  1043,    -1,
      -1,  1312,    -1,  1314,    -1,    -1,  1317,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1325,    -1,    -1,  1594,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1336,    -1,    -1,    -1,   996,
      -1,   998,    -1,    -1,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1043,    -1,    -1,    -1,
      -1,    -1,    -1,  1394,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1676,
    1411,    -1,    -1,  1148,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1085,  1164,
      -1,  1166,    -1,    -1,  1435,    -1,  1437,    -1,    -1,    -1,
      -1,    -1,  1443,    -1,    -1,    -1,    -1,  1182,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,  1125,    55,
      -1,    -1,    -1,  1130,    -1,    -1,    -1,    -1,  1479,  1480,
      -1,    67,  1749,    -1,  1485,    -1,    -1,    -1,  1145,  1146,
      -1,  1148,  1493,    -1,    -1,    -1,    -1,  1764,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1164,    -1,  1166,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1182,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1220,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,  1308,    -1,    -1,    -1,  1312,    -1,  1314,
      -1,    -1,  1317,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1594,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1611,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1628,    -1,    -1,
      -1,    -1,    -1,  1634,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1645,    -1,    -1,    -1,    -1,    -1,
    1651,  1308,  1309,    -1,  1655,  1312,    -1,  1314,    -1,  1394,
    1317,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1676,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,   191,    -1,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,  1715,    -1,    -1,    -1,    -1,    -1,
      -1,  1378,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1394,    55,    -1,
      -1,    -1,    -1,    -1,  1479,  1480,    -1,    -1,  1749,    -1,
    1485,    -1,    69,    70,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    79,  1764,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,    -1,    -1,
     127,   128,   129,   130,    -1,    -1,    -1,   134,   135,   136,
      -1,    -1,  1479,  1480,    -1,    -1,    -1,    -1,  1485,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   153,  1494,    -1,    -1,
      -1,    -1,   159,   160,    -1,   162,   163,   164,   165,    -1,
     167,    -1,    -1,   170,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   190,    29,   192,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,  1634,
      55,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
    1645,    -1,    67,    -1,    -1,    -1,  1651,    -1,    -1,    -1,
    1655,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    67,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1634,    -1,    -1,
    1715,    27,    28,    -1,    -1,    -1,    -1,    -1,  1645,    -1,
      -1,    -1,    -1,    -1,  1651,    -1,    -1,    -1,  1655,    -1,
      -1,    47,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,  1678,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,   191,    83,    84,    -1,
      -1,    -1,    88,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    97,    -1,    -1,   100,   101,    -1,    -1,  1715,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    -1,   123,   124,   125,
     126,   127,   128,    -1,    -1,    -1,   191,    -1,   134,   135,
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
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      48,    49,    50,    51,    52,    53,    -1,    -1,    36,    -1,
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
      -1,    -1,    -1,    -1,   191,   177,    -1,    -1,    -1,    -1,
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
      -1,    -1,   191,   177,    -1,    -1,    -1,    -1,   182,   183,
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
      -1,   187,    -1,    -1,    -1,    -1,   192,   193,    -1,   195,
     196,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    48,    49,    -1,    -1,
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
      -1,    -1,   191,   177,    -1,    -1,    -1,    -1,   182,   183,
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
     165,    27,   167,    29,    -1,   170,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    -1,   190,    -1,   192,    -1,    55,
      -1,    57,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   153,   154,   155,    -1,    -1,    -1,   159,   160,    -1,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,    10,
      11,    12,    -1,    -1,    -1,   177,   178,    -1,   180,    -1,
     182,   183,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   189,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   189,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     189,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   189,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   189,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
      67,     6,     7,    11,    12,    10,    11,    12,    13,   189,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    27,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    79,   188,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,    -1,
      -1,   188,   127,   128,   129,   130,    -1,    -1,    -1,   134,
     135,   136,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   153,    -1,
      -1,    -1,    -1,    -1,   159,   160,    27,   162,   163,   164,
     165,    -1,   167,    -1,    -1,   170,    -1,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    69,    70,
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
      -1,    -1,    -1,    -1,    -1,   127,    -1,    -1,    -1,    -1,
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
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,   182,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,   192,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,   168,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    28,    29,   192,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    12,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    29,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    67,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67
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
     187,   189,   190,   192,   193,   195,   196,   201,   204,   213,
     214,   215,   216,   217,   220,   236,   237,   241,   244,   251,
     257,   317,   318,   326,   330,   331,   332,   333,   334,   335,
     336,   337,   339,   342,   354,   355,   356,   358,   359,   361,
     371,   372,   373,   375,   380,   383,   402,   410,   412,   413,
     414,   415,   416,   417,   418,   419,   420,   421,   422,   423,
     437,   439,   441,   119,   120,   121,   134,   153,   163,   187,
     204,   236,   317,   336,   414,   336,   187,   336,   336,   336,
     105,   336,   336,   400,   401,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,    81,    88,   121,
     148,   187,   214,   355,   372,   375,   380,   414,   417,   414,
      36,   336,   428,   429,   336,   121,   127,   187,   214,   249,
     372,   373,   374,   376,   380,   411,   412,   413,   421,   425,
     426,   187,   327,   377,   187,   327,   346,   328,   336,   222,
     327,   187,   187,   187,   327,   189,   336,   204,   189,   336,
       3,     4,     6,     7,    10,    11,    12,    13,    27,    29,
      55,    57,    69,    70,    71,    72,    73,    74,    75,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   127,   128,   129,   130,   134,   135,   136,
     153,   157,   165,   167,   170,   177,   187,   204,   205,   206,
     216,   442,   457,   458,   460,   189,   333,   336,   190,   229,
     336,   108,   109,   156,   207,   210,   213,    81,   192,   283,
     284,   120,   127,   119,   127,    81,   285,   187,   187,   187,
     187,   204,   255,   445,   187,   187,   328,    81,    87,   149,
     150,   151,   434,   435,   156,   190,   213,   213,   204,   256,
     445,   157,   187,   445,   445,    81,   184,   190,   347,    27,
     326,   330,   336,   337,   414,   418,   218,   190,   423,    87,
     378,   434,    87,   434,   434,    30,   156,   173,   446,   187,
       9,   189,    36,   235,   157,   254,   445,   121,   183,   236,
     318,   189,   189,   189,   189,   189,   189,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    55,    67,   189,    68,    68,   190,
     152,   128,   163,   165,   178,   180,   257,   316,   317,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    65,    66,   131,   132,   404,    68,   190,   409,
     187,   187,    68,   190,   187,   235,   236,    14,   336,   189,
     133,    46,   204,   399,    87,   326,   337,   152,   414,   133,
     194,     9,   385,   250,   326,   337,   414,   446,   152,   187,
     379,   404,   409,   188,   336,    30,   220,     8,   348,     9,
     189,   220,   221,   328,   329,   336,   204,   269,   224,   189,
     189,   189,   135,   136,   460,   460,   173,   187,   108,   460,
      14,   152,   135,   136,   153,   204,   206,   189,   189,   230,
     112,   170,   189,   156,   208,   211,   213,   156,   209,   212,
     213,   213,     9,   189,    99,   190,   414,     9,   189,   127,
     127,    14,     9,   189,   414,   438,   328,   326,   337,   414,
     417,   418,   188,   173,   247,   134,   414,   427,   428,   189,
      68,   404,   149,   435,    80,   336,   414,    87,   149,   435,
     213,   203,   189,   190,   242,   252,   362,   364,    88,   187,
     349,   350,   352,   375,   420,   422,   439,    14,    99,   440,
     343,   344,   345,   279,   280,   402,   403,   188,   188,   188,
     188,   188,   191,   219,   220,   237,   244,   251,   402,   336,
     193,   195,   196,   204,   447,   448,   460,    36,   166,   281,
     282,   336,   442,   187,   445,   245,   235,   336,   336,   336,
      30,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   376,   336,   336,   424,   424,
     336,   430,   431,   127,   190,   205,   206,   423,   255,   204,
     256,   445,   445,   254,   236,    36,   330,   333,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   157,   190,   204,   405,   406,   407,   408,   423,   424,
     336,   281,   281,   424,   336,   427,   235,   188,   336,   187,
     398,     9,   385,   188,   188,    36,   336,    36,   336,   379,
     188,   188,   188,   421,   422,   423,   281,   190,   204,   405,
     406,   423,   188,   218,   273,   190,   333,   336,   336,    91,
      30,   220,   267,   189,    28,    99,    14,     9,   188,    30,
     190,   270,   460,    29,    88,   216,   454,   455,   456,   187,
       9,    48,    49,    54,    56,    68,   135,   157,   177,   187,
     214,   216,   357,   372,   380,   381,   382,   204,   459,   218,
     187,   228,   213,     9,   189,    99,   213,     9,   189,    99,
      99,   210,   204,   336,   284,   381,    81,     9,   188,   188,
     188,   188,   188,   188,   188,   189,    48,    49,   452,   453,
     129,   260,   187,     9,   188,   188,    81,    82,   204,   436,
     204,    68,   191,   191,   200,   202,    30,   130,   259,   172,
      52,   157,   172,   366,   337,   133,     9,   385,   188,   152,
     460,   460,    14,   348,   279,   218,   185,     9,   386,   460,
     461,   404,   409,   404,   191,     9,   385,   174,   414,   336,
     188,     9,   386,    14,   340,   238,   129,   258,   187,   445,
     336,    30,   194,   194,   133,   191,     9,   385,   336,   446,
     187,   248,   243,   253,    14,   440,   246,   235,    70,   414,
     336,   446,   194,   191,   188,   188,   194,   191,   188,    48,
      49,    68,    76,    77,    78,    88,   135,   148,   177,   204,
     388,   390,   391,   394,   397,   204,   414,   414,   133,   258,
     404,   409,   188,   336,   274,    73,    74,   275,   218,   327,
     218,   329,    99,    36,   134,   264,   414,   381,   204,    30,
     220,   268,   189,   271,   189,   271,     9,   174,    88,   133,
     152,     9,   385,   188,   166,   447,   448,   449,   447,   381,
     381,   381,   381,   381,   384,   387,   187,   152,   187,   381,
     152,   190,    10,    11,    12,    29,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    67,   152,
     446,   191,   372,   190,   232,    99,   211,   204,    99,   212,
     204,   204,   191,    14,   414,   189,     9,   174,   204,   261,
     372,   190,   427,   134,   414,    14,   194,   336,   191,   200,
     460,   261,   190,   365,    14,   188,   336,   349,   423,   189,
     460,   185,   191,    30,   450,   403,    36,    81,   166,   405,
     406,   408,   405,   406,   460,    36,   166,   336,   381,   279,
     187,   372,   259,   341,   239,   336,   336,   336,   191,   187,
     281,   260,    30,   259,   460,    14,   258,   445,   376,   191,
     187,    14,    76,    77,    78,   204,   389,   389,   391,   392,
     393,    50,   187,    87,   149,   187,     9,   385,   188,   398,
      36,   336,   259,   191,    73,    74,   276,   327,   220,   191,
     189,    92,   189,   264,   414,   187,   133,   263,    14,   218,
     271,   102,   103,   104,   271,   191,   460,   133,   460,   204,
     454,     9,   188,   385,   133,   194,     9,   385,   384,   205,
     349,   351,   353,   188,   127,   205,   381,   432,   433,   381,
     381,   381,    30,   381,   381,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   381,   381,   459,    81,   233,
     204,   204,   381,   453,    99,   100,   451,     9,   289,   188,
     187,   330,   333,   336,   194,   191,   440,   289,   158,   171,
     190,   361,   368,   158,   190,   367,   133,   189,   450,   460,
     348,   461,    81,   166,    14,    81,   446,   414,   336,   188,
     279,   190,   279,   187,   133,   187,   281,   188,   190,   460,
     190,   189,   460,   259,   240,   379,   281,   133,   194,     9,
     385,   390,   392,   149,   349,   395,   396,   391,   414,   190,
     327,    30,    75,   220,   189,   329,   263,   427,   264,   188,
     381,    98,   102,   189,   336,    30,   189,   272,   191,   174,
     460,   133,   166,    30,   188,   381,   381,   188,   133,     9,
     385,   188,   133,   191,     9,   385,   381,    30,   188,   218,
     204,   460,   460,   372,     4,   109,   114,   120,   122,   159,
     160,   162,   191,   290,   315,   316,   317,   322,   323,   324,
     325,   402,   427,   191,   190,   191,    52,   336,   336,   336,
     348,    36,    81,   166,    14,    81,   336,   187,   450,   188,
     289,   188,   279,   336,   281,   188,   289,   440,   289,   189,
     190,   187,   188,   391,   391,   188,   133,   188,     9,   385,
     289,    30,   218,   189,   188,   188,   188,   225,   189,   189,
     272,   218,   460,   460,   133,   381,   349,   381,   381,   381,
     190,   191,   451,   129,   130,   178,   205,   443,   460,   262,
     372,   109,   325,    29,   122,   135,   136,   157,   163,   299,
     300,   301,   302,   372,   161,   307,   308,   125,   187,   204,
     309,   310,   291,   236,   460,     9,   189,     9,   189,   189,
     440,   316,   188,   286,   157,   363,   191,   191,    81,   166,
      14,    81,   336,   281,   114,   338,   450,   191,   450,   188,
     188,   191,   190,   191,   289,   279,   133,   391,   349,   191,
     218,   223,   226,    30,   220,   266,   218,   188,   381,   133,
     133,   218,   372,   372,   445,    14,   205,     9,   189,   190,
     443,   440,   302,   173,   190,     9,   189,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    55,    69,
      70,    71,    72,    73,    74,    75,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   134,   135,   137,
     138,   139,   140,   141,   153,   154,   155,   165,   167,   168,
     170,   177,   178,   180,   182,   183,   204,   369,   370,     9,
     189,   157,   161,   204,   310,   311,   312,   189,    81,   321,
     235,   292,   443,   443,    14,   236,   191,   287,   288,   443,
      14,    81,   336,   188,   187,   190,   189,   190,   313,   338,
     450,   286,   191,   188,   391,   133,    30,   220,   265,   266,
     218,   381,   381,   191,   189,   189,   381,   372,   295,   460,
     303,   304,   380,   300,    14,    30,    49,   305,   308,     9,
      34,   188,    29,    48,    51,    14,     9,   189,   206,   444,
     321,    14,   460,   235,   189,    14,   336,    36,    81,   360,
     218,   218,   190,   313,   191,   450,   391,   218,    96,   231,
     191,   204,   216,   296,   297,   298,     9,   174,     9,   385,
     191,   381,   370,   370,    57,   306,   311,   311,    29,    48,
      51,   381,    81,   173,   187,   189,   381,   445,   381,    81,
       9,   386,   191,   191,   218,   313,    94,   189,   112,   227,
     152,    99,   460,   380,   164,    14,   452,   293,   187,    36,
      81,   188,   191,   189,   187,   170,   234,   204,   316,   317,
     174,   381,   174,   277,   278,   403,   294,    81,   372,   232,
     167,   204,   189,   188,     9,   386,   116,   117,   118,   319,
     320,   277,    81,   262,   189,   450,   403,   461,   188,   188,
     189,   189,   190,   314,   319,    36,    81,   166,   450,   190,
     218,   461,    81,   166,    14,    81,   314,   218,   191,    36,
      81,   166,    14,    81,   336,   191,    81,   166,    14,    81,
     336,    14,    81,   336,   336
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
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 850 "hphp.y"
    { ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 851 "hphp.y"
    { ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 856 "hphp.y"
    { ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 857 "hphp.y"
    { ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 862 "hphp.y"
    { ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 863 "hphp.y"
    { ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 867 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 868 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 869 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 871 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 879 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 885 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 898 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 904 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 947 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 989 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1022 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { (yyval).reset();;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { (yyval).reset();;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1065 "hphp.y"
    { (yyval).reset();;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1094 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1101 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1107 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1115 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1119 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1123 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1127 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1136 "hphp.y"
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
#line 1151 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
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
#line 1168 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1176 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1188 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1215 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1227 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1230 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1235 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval).reset();;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { (yyval).reset();;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { (yyval).reset();;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval).reset();;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { (yyval).reset();;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { (yyval).reset();;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { (yyval).reset();;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { (yyval).reset();;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1325 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1344 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1363 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1371 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1386 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1391 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1401 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1431 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { (yyval).reset();;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { (yyval).reset();;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { (yyval).reset();;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { (yyval).reset();;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { (yyval).reset();;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { (yyval).reset();;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { (yyval).reset();;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { (yyval).reset();;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { (yyval).reset();;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval).reset();;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { (yyval).reset();;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
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
#line 1994 "hphp.y"
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
#line 2004 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
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
#line 2022 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
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
#line 2039 "hphp.y"
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
#line 2047 "hphp.y"
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
#line 2054 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2062 "hphp.y"
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
#line 2072 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2073 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
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
#line 2180 "hphp.y"
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
#line 2191 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval).reset();;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval).reset();;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
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
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval).reset();;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { (yyval).reset();;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval).reset();;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval).reset();;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval).reset();;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval).reset();;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval).reset();;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { (yyval).reset();;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { (yyval).reset();;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { (yyval).reset();;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval).reset();;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
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
#line 2692 "hphp.y"
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
#line 2707 "hphp.y"
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

  case 813:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
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
#line 2755 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
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
#line 2784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { (yyval).reset();;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
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
#line 2900 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
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
#line 2918 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { (yyval).reset();;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
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
#line 3005 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3011 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3079 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3089 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3099 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3106 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3107 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3114 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    {;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3157 "hphp.y"
    {;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    {;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3164 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
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
#line 3180 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3191 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3208 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3212 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3226 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3229 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3235 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3242 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3257 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3266 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13716 "hphp.7.tab.cpp"
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
#line 3269 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

