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
#define YYLAST   16763

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  197
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  264
/* YYNRULES -- Number of rules.  */
#define YYNRULES  982
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1803

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
     242,   244,   247,   251,   256,   260,   262,   264,   267,   270,
     272,   276,   278,   282,   285,   288,   291,   297,   302,   305,
     306,   308,   310,   312,   314,   318,   324,   333,   334,   339,
     340,   347,   348,   359,   360,   365,   368,   372,   375,   379,
     382,   386,   390,   394,   398,   402,   406,   412,   414,   416,
     418,   419,   429,   430,   441,   447,   448,   462,   463,   469,
     473,   477,   480,   483,   486,   489,   492,   495,   499,   502,
     505,   506,   511,   521,   522,   523,   528,   531,   532,   534,
     535,   537,   538,   548,   549,   560,   561,   573,   574,   584,
     585,   596,   597,   606,   607,   617,   618,   626,   627,   636,
     637,   646,   647,   655,   656,   665,   667,   669,   671,   673,
     675,   678,   682,   686,   689,   692,   693,   696,   697,   700,
     701,   703,   707,   709,   713,   716,   717,   719,   722,   727,
     729,   734,   736,   741,   743,   748,   750,   755,   759,   765,
     769,   774,   779,   785,   791,   796,   797,   799,   801,   806,
     807,   813,   814,   817,   818,   822,   823,   831,   840,   847,
     850,   856,   863,   868,   869,   874,   880,   888,   895,   902,
     910,   920,   929,   936,   944,   950,   953,   958,   964,   968,
     969,   973,   978,   985,   991,   997,  1004,  1013,  1021,  1024,
    1025,  1027,  1030,  1033,  1037,  1042,  1047,  1051,  1053,  1055,
    1058,  1063,  1067,  1073,  1075,  1079,  1082,  1083,  1086,  1090,
    1093,  1094,  1095,  1100,  1101,  1107,  1110,  1113,  1116,  1117,
    1128,  1129,  1141,  1145,  1149,  1153,  1158,  1163,  1167,  1173,
    1176,  1179,  1180,  1187,  1193,  1198,  1202,  1204,  1206,  1210,
    1215,  1217,  1220,  1222,  1224,  1229,  1236,  1238,  1240,  1245,
    1247,  1249,  1253,  1256,  1259,  1260,  1263,  1264,  1266,  1270,
    1272,  1274,  1276,  1278,  1282,  1287,  1292,  1297,  1299,  1301,
    1304,  1307,  1310,  1314,  1318,  1320,  1322,  1324,  1326,  1330,
    1332,  1336,  1338,  1340,  1342,  1343,  1345,  1348,  1350,  1352,
    1354,  1356,  1358,  1360,  1362,  1364,  1365,  1367,  1369,  1371,
    1375,  1381,  1383,  1387,  1393,  1398,  1402,  1406,  1410,  1415,
    1419,  1423,  1427,  1430,  1433,  1435,  1437,  1441,  1445,  1447,
    1449,  1450,  1452,  1455,  1460,  1464,  1468,  1475,  1478,  1482,
    1489,  1491,  1493,  1495,  1497,  1499,  1506,  1510,  1515,  1522,
    1526,  1530,  1534,  1538,  1542,  1546,  1550,  1554,  1558,  1562,
    1566,  1570,  1573,  1576,  1579,  1582,  1586,  1590,  1594,  1598,
    1602,  1606,  1610,  1614,  1618,  1622,  1626,  1630,  1634,  1638,
    1642,  1646,  1650,  1653,  1656,  1659,  1662,  1666,  1670,  1674,
    1678,  1682,  1686,  1690,  1694,  1698,  1702,  1706,  1712,  1717,
    1721,  1723,  1726,  1729,  1732,  1735,  1738,  1741,  1744,  1747,
    1750,  1752,  1754,  1756,  1760,  1763,  1765,  1771,  1772,  1773,
    1785,  1786,  1799,  1800,  1805,  1806,  1814,  1815,  1821,  1822,
    1826,  1827,  1834,  1837,  1840,  1845,  1847,  1849,  1855,  1859,
    1865,  1869,  1872,  1873,  1876,  1877,  1882,  1887,  1891,  1896,
    1901,  1906,  1911,  1913,  1915,  1917,  1919,  1923,  1927,  1932,
    1934,  1937,  1942,  1945,  1952,  1953,  1955,  1960,  1961,  1964,
    1965,  1967,  1969,  1973,  1975,  1979,  1981,  1983,  1987,  1991,
    1993,  1995,  1997,  1999,  2001,  2003,  2005,  2007,  2009,  2011,
    2013,  2015,  2017,  2019,  2021,  2023,  2025,  2027,  2029,  2031,
    2033,  2035,  2037,  2039,  2041,  2043,  2045,  2047,  2049,  2051,
    2053,  2055,  2057,  2059,  2061,  2063,  2065,  2067,  2069,  2071,
    2073,  2075,  2077,  2079,  2081,  2083,  2085,  2087,  2089,  2091,
    2093,  2095,  2097,  2099,  2101,  2103,  2105,  2107,  2109,  2111,
    2113,  2115,  2117,  2119,  2121,  2123,  2125,  2127,  2129,  2131,
    2133,  2135,  2137,  2139,  2141,  2143,  2145,  2147,  2149,  2151,
    2156,  2158,  2160,  2162,  2164,  2166,  2168,  2172,  2174,  2178,
    2180,  2182,  2186,  2188,  2190,  2192,  2195,  2197,  2198,  2199,
    2201,  2203,  2207,  2208,  2210,  2212,  2214,  2216,  2218,  2220,
    2222,  2224,  2226,  2228,  2230,  2232,  2234,  2238,  2241,  2243,
    2245,  2250,  2254,  2259,  2261,  2263,  2267,  2271,  2275,  2279,
    2283,  2287,  2291,  2295,  2299,  2303,  2307,  2311,  2315,  2319,
    2323,  2327,  2331,  2335,  2338,  2341,  2344,  2347,  2351,  2355,
    2359,  2363,  2367,  2371,  2375,  2379,  2383,  2389,  2394,  2398,
    2402,  2406,  2408,  2410,  2412,  2414,  2418,  2422,  2426,  2429,
    2430,  2432,  2433,  2435,  2436,  2442,  2446,  2450,  2452,  2454,
    2456,  2458,  2462,  2465,  2467,  2469,  2471,  2473,  2475,  2479,
    2481,  2483,  2485,  2488,  2491,  2496,  2500,  2505,  2508,  2509,
    2515,  2519,  2523,  2525,  2529,  2531,  2534,  2535,  2541,  2545,
    2548,  2549,  2553,  2554,  2559,  2562,  2563,  2567,  2571,  2573,
    2574,  2576,  2578,  2580,  2582,  2586,  2588,  2590,  2592,  2596,
    2598,  2600,  2604,  2608,  2611,  2616,  2619,  2624,  2630,  2636,
    2642,  2648,  2650,  2652,  2654,  2656,  2658,  2660,  2664,  2668,
    2673,  2678,  2682,  2684,  2686,  2688,  2690,  2694,  2696,  2701,
    2705,  2709,  2711,  2713,  2715,  2717,  2719,  2723,  2727,  2732,
    2737,  2741,  2743,  2745,  2753,  2763,  2771,  2778,  2787,  2789,
    2794,  2799,  2801,  2803,  2808,  2811,  2813,  2814,  2816,  2818,
    2820,  2824,  2828,  2832,  2833,  2835,  2837,  2841,  2845,  2848,
    2852,  2859,  2860,  2862,  2867,  2870,  2871,  2877,  2881,  2885,
    2887,  2894,  2899,  2904,  2907,  2910,  2911,  2917,  2921,  2925,
    2927,  2930,  2931,  2937,  2941,  2945,  2947,  2950,  2953,  2955,
    2958,  2960,  2965,  2969,  2973,  2980,  2984,  2986,  2988,  2990,
    2995,  3000,  3005,  3010,  3015,  3020,  3023,  3026,  3031,  3034,
    3037,  3039,  3043,  3047,  3051,  3052,  3055,  3061,  3068,  3075,
    3083,  3085,  3088,  3090,  3093,  3095,  3100,  3102,  3107,  3111,
    3112,  3114,  3118,  3121,  3125,  3127,  3129,  3130,  3131,  3134,
    3137,  3140,  3145,  3148,  3154,  3158,  3160,  3162,  3163,  3167,
    3172,  3178,  3182,  3184,  3187,  3188,  3193,  3195,  3199,  3202,
    3205,  3208,  3210,  3212,  3214,  3216,  3220,  3225,  3232,  3234,
    3243,  3250,  3252
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     198,     0,    -1,    -1,   199,   200,    -1,   200,   201,    -1,
      -1,   219,    -1,   236,    -1,   243,    -1,   240,    -1,   250,
      -1,   440,    -1,   126,   187,   188,   189,    -1,   153,   212,
     189,    -1,    -1,   153,   212,   190,   202,   200,   191,    -1,
      -1,   153,   190,   203,   200,   191,    -1,   114,   208,   189,
      -1,   114,   108,   208,   189,    -1,   114,   109,   208,   189,
      -1,   114,   207,   190,   210,   191,   189,    -1,   114,   108,
     207,   190,   208,   191,   189,    -1,   114,   109,   207,   190,
     208,   191,   189,    -1,   216,   189,    -1,    79,    -1,   100,
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
      -1,   112,    -1,   123,    -1,   205,    -1,   127,    -1,   212,
     156,    -1,   156,   212,   156,    -1,   208,     9,   209,    -1,
     209,    -1,   212,    -1,   156,   212,    -1,   212,    99,   204,
      -1,   156,   212,    99,   204,    -1,   210,     9,   211,    -1,
     211,    -1,   209,    -1,   108,   209,    -1,   109,   209,    -1,
     204,    -1,   212,   156,   204,    -1,   212,    -1,   153,   156,
     212,    -1,   156,   212,    -1,   213,   445,    -1,   213,   445,
      -1,   216,     9,   441,    14,   380,    -1,   109,   441,    14,
     380,    -1,   217,   218,    -1,    -1,   219,    -1,   236,    -1,
     243,    -1,   250,    -1,   190,   217,   191,    -1,    72,   326,
     219,   272,   274,    -1,    72,   326,    30,   217,   273,   275,
      75,   189,    -1,    -1,    91,   326,   220,   266,    -1,    -1,
      90,   221,   219,    91,   326,   189,    -1,    -1,    93,   187,
     328,   189,   328,   189,   328,   188,   222,   264,    -1,    -1,
     101,   326,   223,   269,    -1,   105,   189,    -1,   105,   335,
     189,    -1,   107,   189,    -1,   107,   335,   189,    -1,   110,
     189,    -1,   110,   335,   189,    -1,    27,   105,   189,    -1,
     115,   282,   189,    -1,   121,   284,   189,    -1,    89,   327,
     189,    -1,   145,   327,   189,    -1,   123,   187,   437,   188,
     189,    -1,   189,    -1,    83,    -1,    84,    -1,    -1,    95,
     187,   335,    99,   263,   262,   188,   224,   265,    -1,    -1,
      95,   187,   335,    28,    99,   263,   262,   188,   225,   265,
      -1,    97,   187,   268,   188,   267,    -1,    -1,   111,   228,
     112,   187,   371,    81,   188,   190,   217,   191,   230,   226,
     233,    -1,    -1,   111,   228,   170,   227,   231,    -1,   113,
     335,   189,    -1,   106,   204,   189,    -1,   335,   189,    -1,
     329,   189,    -1,   330,   189,    -1,   331,   189,    -1,   332,
     189,    -1,   333,   189,    -1,   110,   332,   189,    -1,   334,
     189,    -1,   204,    30,    -1,    -1,   190,   229,   217,   191,
      -1,   230,   112,   187,   371,    81,   188,   190,   217,   191,
      -1,    -1,    -1,   190,   232,   217,   191,    -1,   170,   231,
      -1,    -1,    36,    -1,    -1,   108,    -1,    -1,   235,   234,
     444,   237,   187,   278,   188,   449,   312,    -1,    -1,   316,
     235,   234,   444,   238,   187,   278,   188,   449,   312,    -1,
      -1,   401,   315,   235,   234,   444,   239,   187,   278,   188,
     449,   312,    -1,    -1,   163,   204,   241,    30,   459,   439,
     190,   285,   191,    -1,    -1,   401,   163,   204,   242,    30,
     459,   439,   190,   285,   191,    -1,    -1,   256,   253,   244,
     257,   258,   190,   288,   191,    -1,    -1,   401,   256,   253,
     245,   257,   258,   190,   288,   191,    -1,    -1,   128,   254,
     246,   259,   190,   288,   191,    -1,    -1,   401,   128,   254,
     247,   259,   190,   288,   191,    -1,    -1,   127,   249,   378,
     257,   258,   190,   288,   191,    -1,    -1,   165,   255,   251,
     258,   190,   288,   191,    -1,    -1,   401,   165,   255,   252,
     258,   190,   288,   191,    -1,   444,    -1,   157,    -1,   444,
      -1,   444,    -1,   127,    -1,   120,   127,    -1,   120,   119,
     127,    -1,   119,   120,   127,    -1,   119,   127,    -1,   129,
     371,    -1,    -1,   130,   260,    -1,    -1,   129,   260,    -1,
      -1,   371,    -1,   260,     9,   371,    -1,   371,    -1,   261,
       9,   371,    -1,   133,   263,    -1,    -1,   413,    -1,    36,
     413,    -1,   134,   187,   426,   188,    -1,   219,    -1,    30,
     217,    94,   189,    -1,   219,    -1,    30,   217,    96,   189,
      -1,   219,    -1,    30,   217,    92,   189,    -1,   219,    -1,
      30,   217,    98,   189,    -1,   204,    14,   380,    -1,   268,
       9,   204,    14,   380,    -1,   190,   270,   191,    -1,   190,
     189,   270,   191,    -1,    30,   270,   102,   189,    -1,    30,
     189,   270,   102,   189,    -1,   270,   103,   335,   271,   217,
      -1,   270,   104,   271,   217,    -1,    -1,    30,    -1,   189,
      -1,   272,    73,   326,   219,    -1,    -1,   273,    73,   326,
      30,   217,    -1,    -1,    74,   219,    -1,    -1,    74,    30,
     217,    -1,    -1,   277,     9,   402,   318,   460,   166,    81,
      -1,   277,     9,   402,   318,   460,    36,   166,    81,    -1,
     277,     9,   402,   318,   460,   166,    -1,   277,   385,    -1,
     402,   318,   460,   166,    81,    -1,   402,   318,   460,    36,
     166,    81,    -1,   402,   318,   460,   166,    -1,    -1,   402,
     318,   460,    81,    -1,   402,   318,   460,    36,    81,    -1,
     402,   318,   460,    36,    81,    14,   335,    -1,   402,   318,
     460,    81,    14,   335,    -1,   277,     9,   402,   318,   460,
      81,    -1,   277,     9,   402,   318,   460,    36,    81,    -1,
     277,     9,   402,   318,   460,    36,    81,    14,   335,    -1,
     277,     9,   402,   318,   460,    81,    14,   335,    -1,   279,
       9,   402,   460,   166,    81,    -1,   279,     9,   402,   460,
      36,   166,    81,    -1,   279,     9,   402,   460,   166,    -1,
     279,   385,    -1,   402,   460,   166,    81,    -1,   402,   460,
      36,   166,    81,    -1,   402,   460,   166,    -1,    -1,   402,
     460,    81,    -1,   402,   460,    36,    81,    -1,   402,   460,
      36,    81,    14,   335,    -1,   402,   460,    81,    14,   335,
      -1,   279,     9,   402,   460,    81,    -1,   279,     9,   402,
     460,    36,    81,    -1,   279,     9,   402,   460,    36,    81,
      14,   335,    -1,   279,     9,   402,   460,    81,    14,   335,
      -1,   281,   385,    -1,    -1,   335,    -1,    36,   413,    -1,
     166,   335,    -1,   281,     9,   335,    -1,   281,     9,   166,
     335,    -1,   281,     9,    36,   413,    -1,   282,     9,   283,
      -1,   283,    -1,    81,    -1,   192,   413,    -1,   192,   190,
     335,   191,    -1,   284,     9,    81,    -1,   284,     9,    81,
      14,   380,    -1,    81,    -1,    81,    14,   380,    -1,   285,
     286,    -1,    -1,   287,   189,    -1,   442,    14,   380,    -1,
     288,   289,    -1,    -1,    -1,   314,   290,   320,   189,    -1,
      -1,   316,   459,   291,   320,   189,    -1,   321,   189,    -1,
     322,   189,    -1,   323,   189,    -1,    -1,   315,   235,   234,
     443,   187,   292,   276,   188,   449,   313,    -1,    -1,   401,
     315,   235,   234,   444,   187,   293,   276,   188,   449,   313,
      -1,   159,   298,   189,    -1,   160,   306,   189,    -1,   162,
     308,   189,    -1,     4,   129,   371,   189,    -1,     4,   130,
     371,   189,    -1,   114,   261,   189,    -1,   114,   261,   190,
     294,   191,    -1,   294,   295,    -1,   294,   296,    -1,    -1,
     215,   152,   204,   167,   261,   189,    -1,   297,    99,   315,
     204,   189,    -1,   297,    99,   316,   189,    -1,   215,   152,
     204,    -1,   204,    -1,   299,    -1,   298,     9,   299,    -1,
     300,   368,   304,   305,    -1,   157,    -1,    29,   301,    -1,
     301,    -1,   135,    -1,   135,   173,   459,   174,    -1,   135,
     173,   459,     9,   459,   174,    -1,   371,    -1,   122,    -1,
     163,   190,   303,   191,    -1,   136,    -1,   379,    -1,   302,
       9,   379,    -1,   302,   384,    -1,    14,   380,    -1,    -1,
      57,   164,    -1,    -1,   307,    -1,   306,     9,   307,    -1,
     161,    -1,   309,    -1,   204,    -1,   125,    -1,   187,   310,
     188,    -1,   187,   310,   188,    51,    -1,   187,   310,   188,
      29,    -1,   187,   310,   188,    48,    -1,   309,    -1,   311,
      -1,   311,    51,    -1,   311,    29,    -1,   311,    48,    -1,
     310,     9,   310,    -1,   310,    34,   310,    -1,   204,    -1,
     157,    -1,   161,    -1,   189,    -1,   190,   217,   191,    -1,
     189,    -1,   190,   217,   191,    -1,   316,    -1,   122,    -1,
     316,    -1,    -1,   317,    -1,   316,   317,    -1,   116,    -1,
     117,    -1,   118,    -1,   121,    -1,   120,    -1,   119,    -1,
     183,    -1,   319,    -1,    -1,   116,    -1,   117,    -1,   118,
      -1,   320,     9,    81,    -1,   320,     9,    81,    14,   380,
      -1,    81,    -1,    81,    14,   380,    -1,   321,     9,   442,
      14,   380,    -1,   109,   442,    14,   380,    -1,   322,     9,
     442,    -1,   120,   109,   442,    -1,   120,   324,   439,    -1,
     324,   439,    14,   459,    -1,   109,   178,   444,    -1,   187,
     325,   188,    -1,    70,   375,   378,    -1,    70,   248,    -1,
      69,   335,    -1,   360,    -1,   355,    -1,   187,   335,   188,
      -1,   327,     9,   335,    -1,   335,    -1,   327,    -1,    -1,
      27,    -1,    27,   335,    -1,    27,   335,   133,   335,    -1,
     187,   329,   188,    -1,   413,    14,   329,    -1,   134,   187,
     426,   188,    14,   329,    -1,    28,   335,    -1,   413,    14,
     332,    -1,   134,   187,   426,   188,    14,   332,    -1,   336,
      -1,   413,    -1,   325,    -1,   417,    -1,   416,    -1,   134,
     187,   426,   188,    14,   335,    -1,   413,    14,   335,    -1,
     413,    14,    36,   413,    -1,   413,    14,    36,    70,   375,
     378,    -1,   413,    26,   335,    -1,   413,    25,   335,    -1,
     413,    24,   335,    -1,   413,    23,   335,    -1,   413,    22,
     335,    -1,   413,    21,   335,    -1,   413,    20,   335,    -1,
     413,    19,   335,    -1,   413,    18,   335,    -1,   413,    17,
     335,    -1,   413,    16,   335,    -1,   413,    15,   335,    -1,
     413,    66,    -1,    66,   413,    -1,   413,    65,    -1,    65,
     413,    -1,   335,    32,   335,    -1,   335,    33,   335,    -1,
     335,    10,   335,    -1,   335,    12,   335,    -1,   335,    11,
     335,    -1,   335,    34,   335,    -1,   335,    36,   335,    -1,
     335,    35,   335,    -1,   335,    50,   335,    -1,   335,    48,
     335,    -1,   335,    49,   335,    -1,   335,    51,   335,    -1,
     335,    52,   335,    -1,   335,    67,   335,    -1,   335,    53,
     335,    -1,   335,    47,   335,    -1,   335,    46,   335,    -1,
      48,   335,    -1,    49,   335,    -1,    54,   335,    -1,    56,
     335,    -1,   335,    38,   335,    -1,   335,    37,   335,    -1,
     335,    40,   335,    -1,   335,    39,   335,    -1,   335,    41,
     335,    -1,   335,    45,   335,    -1,   335,    42,   335,    -1,
     335,    44,   335,    -1,   335,    43,   335,    -1,   335,    55,
     375,    -1,   187,   336,   188,    -1,   335,    29,   335,    30,
     335,    -1,   335,    29,    30,   335,    -1,   335,    31,   335,
      -1,   436,    -1,    64,   335,    -1,    63,   335,    -1,    62,
     335,    -1,    61,   335,    -1,    60,   335,    -1,    59,   335,
      -1,    58,   335,    -1,    71,   376,    -1,    57,   335,    -1,
     382,    -1,   354,    -1,   353,    -1,   193,   377,   193,    -1,
      13,   335,    -1,   357,    -1,   114,   187,   359,   385,   188,
      -1,    -1,    -1,   235,   234,   187,   339,   278,   188,   449,
     337,   190,   217,   191,    -1,    -1,   316,   235,   234,   187,
     340,   278,   188,   449,   337,   190,   217,   191,    -1,    -1,
     183,    81,   342,   347,    -1,    -1,   183,   184,   343,   278,
     185,   449,   347,    -1,    -1,   183,   190,   344,   217,   191,
      -1,    -1,    81,   345,   347,    -1,    -1,   184,   346,   278,
     185,   449,   347,    -1,     8,   335,    -1,     8,   332,    -1,
       8,   190,   217,   191,    -1,    88,    -1,   438,    -1,   349,
       9,   348,   133,   335,    -1,   348,   133,   335,    -1,   350,
       9,   348,   133,   380,    -1,   348,   133,   380,    -1,   349,
     384,    -1,    -1,   350,   384,    -1,    -1,   177,   187,   351,
     188,    -1,   135,   187,   427,   188,    -1,    68,   427,   194,
      -1,   371,   190,   429,   191,    -1,   371,   190,   431,   191,
      -1,   357,    68,   423,   194,    -1,   358,    68,   423,   194,
      -1,   354,    -1,   438,    -1,   416,    -1,    88,    -1,   187,
     336,   188,    -1,   359,     9,    81,    -1,   359,     9,    36,
      81,    -1,    81,    -1,    36,    81,    -1,   171,   157,   361,
     172,    -1,   363,    52,    -1,   363,   172,   364,   171,    52,
     362,    -1,    -1,   157,    -1,   363,   365,    14,   366,    -1,
      -1,   364,   367,    -1,    -1,   157,    -1,   158,    -1,   190,
     335,   191,    -1,   158,    -1,   190,   335,   191,    -1,   360,
      -1,   369,    -1,   368,    30,   369,    -1,   368,    49,   369,
      -1,   204,    -1,    71,    -1,   108,    -1,   109,    -1,   110,
      -1,    27,    -1,    28,    -1,   111,    -1,   112,    -1,   170,
      -1,   113,    -1,    72,    -1,    73,    -1,    75,    -1,    74,
      -1,    91,    -1,    92,    -1,    90,    -1,    93,    -1,    94,
      -1,    95,    -1,    96,    -1,    97,    -1,    98,    -1,    55,
      -1,    99,    -1,   101,    -1,   102,    -1,   103,    -1,   104,
      -1,   105,    -1,   107,    -1,   106,    -1,    89,    -1,    13,
      -1,   127,    -1,   128,    -1,   129,    -1,   130,    -1,    70,
      -1,    69,    -1,   122,    -1,     5,    -1,     7,    -1,     6,
      -1,     4,    -1,     3,    -1,   153,    -1,   114,    -1,   115,
      -1,   124,    -1,   125,    -1,   126,    -1,   121,    -1,   120,
      -1,   119,    -1,   118,    -1,   117,    -1,   116,    -1,   183,
      -1,   123,    -1,   134,    -1,   135,    -1,    10,    -1,    12,
      -1,    11,    -1,   137,    -1,   139,    -1,   138,    -1,   140,
      -1,   141,    -1,   155,    -1,   154,    -1,   182,    -1,   165,
      -1,   168,    -1,   167,    -1,   178,    -1,   180,    -1,   177,
      -1,   214,   187,   280,   188,    -1,   215,    -1,   157,    -1,
     371,    -1,   379,    -1,   121,    -1,   421,    -1,   187,   336,
     188,    -1,   372,    -1,   373,   152,   422,    -1,   372,    -1,
     419,    -1,   374,   152,   422,    -1,   371,    -1,   121,    -1,
     424,    -1,   187,   188,    -1,   326,    -1,    -1,    -1,    87,
      -1,   433,    -1,   187,   280,   188,    -1,    -1,    76,    -1,
      77,    -1,    78,    -1,    88,    -1,   140,    -1,   141,    -1,
     155,    -1,   137,    -1,   168,    -1,   138,    -1,   139,    -1,
     154,    -1,   182,    -1,   148,    87,   149,    -1,   148,   149,
      -1,   379,    -1,   213,    -1,   135,   187,   383,   188,    -1,
      68,   383,   194,    -1,   177,   187,   352,   188,    -1,   381,
      -1,   356,    -1,   187,   380,   188,    -1,   380,    32,   380,
      -1,   380,    33,   380,    -1,   380,    10,   380,    -1,   380,
      12,   380,    -1,   380,    11,   380,    -1,   380,    34,   380,
      -1,   380,    36,   380,    -1,   380,    35,   380,    -1,   380,
      50,   380,    -1,   380,    48,   380,    -1,   380,    49,   380,
      -1,   380,    51,   380,    -1,   380,    52,   380,    -1,   380,
      53,   380,    -1,   380,    47,   380,    -1,   380,    46,   380,
      -1,   380,    67,   380,    -1,    54,   380,    -1,    56,   380,
      -1,    48,   380,    -1,    49,   380,    -1,   380,    38,   380,
      -1,   380,    37,   380,    -1,   380,    40,   380,    -1,   380,
      39,   380,    -1,   380,    41,   380,    -1,   380,    45,   380,
      -1,   380,    42,   380,    -1,   380,    44,   380,    -1,   380,
      43,   380,    -1,   380,    29,   380,    30,   380,    -1,   380,
      29,    30,   380,    -1,   215,   152,   205,    -1,   157,   152,
     205,    -1,   215,   152,   127,    -1,   213,    -1,    80,    -1,
     438,    -1,   379,    -1,   195,   433,   195,    -1,   196,   433,
     196,    -1,   148,   433,   149,    -1,   386,   384,    -1,    -1,
       9,    -1,    -1,     9,    -1,    -1,   386,     9,   380,   133,
     380,    -1,   386,     9,   380,    -1,   380,   133,   380,    -1,
     380,    -1,    76,    -1,    77,    -1,    78,    -1,   148,    87,
     149,    -1,   148,   149,    -1,    76,    -1,    77,    -1,    78,
      -1,   204,    -1,    88,    -1,    88,    50,   389,    -1,   387,
      -1,   389,    -1,   204,    -1,    48,   388,    -1,    49,   388,
      -1,   135,   187,   391,   188,    -1,    68,   391,   194,    -1,
     177,   187,   394,   188,    -1,   392,   384,    -1,    -1,   392,
       9,   390,   133,   390,    -1,   392,     9,   390,    -1,   390,
     133,   390,    -1,   390,    -1,   393,     9,   390,    -1,   390,
      -1,   395,   384,    -1,    -1,   395,     9,   348,   133,   390,
      -1,   348,   133,   390,    -1,   393,   384,    -1,    -1,   187,
     396,   188,    -1,    -1,   398,     9,   204,   397,    -1,   204,
     397,    -1,    -1,   400,   398,   384,    -1,    47,   399,    46,
      -1,   401,    -1,    -1,   131,    -1,   132,    -1,   204,    -1,
     157,    -1,   190,   335,   191,    -1,   404,    -1,   422,    -1,
     204,    -1,   190,   335,   191,    -1,   406,    -1,   422,    -1,
      68,   423,   194,    -1,   190,   335,   191,    -1,   414,   408,
      -1,   187,   325,   188,   408,    -1,   425,   408,    -1,   187,
     325,   188,   408,    -1,   187,   325,   188,   403,   405,    -1,
     187,   336,   188,   403,   405,    -1,   187,   325,   188,   403,
     404,    -1,   187,   336,   188,   403,   404,    -1,   420,    -1,
     370,    -1,   418,    -1,   419,    -1,   409,    -1,   411,    -1,
     413,   403,   405,    -1,   374,   152,   422,    -1,   415,   187,
     280,   188,    -1,   416,   187,   280,   188,    -1,   187,   413,
     188,    -1,   370,    -1,   418,    -1,   419,    -1,   409,    -1,
     413,   403,   405,    -1,   412,    -1,   415,   187,   280,   188,
      -1,   187,   413,   188,    -1,   374,   152,   422,    -1,   420,
      -1,   409,    -1,   370,    -1,   354,    -1,   379,    -1,   187,
     413,   188,    -1,   187,   336,   188,    -1,   416,   187,   280,
     188,    -1,   415,   187,   280,   188,    -1,   187,   417,   188,
      -1,   338,    -1,   341,    -1,   413,   403,   407,   445,   187,
     280,   188,    -1,   187,   325,   188,   403,   407,   445,   187,
     280,   188,    -1,   374,   152,   206,   445,   187,   280,   188,
      -1,   374,   152,   422,   187,   280,   188,    -1,   374,   152,
     190,   335,   191,   187,   280,   188,    -1,   421,    -1,   421,
      68,   423,   194,    -1,   421,   190,   335,   191,    -1,   422,
      -1,    81,    -1,   192,   190,   335,   191,    -1,   192,   422,
      -1,   335,    -1,    -1,   420,    -1,   410,    -1,   411,    -1,
     424,   403,   405,    -1,   373,   152,   420,    -1,   187,   413,
     188,    -1,    -1,   410,    -1,   412,    -1,   424,   403,   404,
      -1,   187,   413,   188,    -1,   426,     9,    -1,   426,     9,
     413,    -1,   426,     9,   134,   187,   426,   188,    -1,    -1,
     413,    -1,   134,   187,   426,   188,    -1,   428,   384,    -1,
      -1,   428,     9,   335,   133,   335,    -1,   428,     9,   335,
      -1,   335,   133,   335,    -1,   335,    -1,   428,     9,   335,
     133,    36,   413,    -1,   428,     9,    36,   413,    -1,   335,
     133,    36,   413,    -1,    36,   413,    -1,   430,   384,    -1,
      -1,   430,     9,   335,   133,   335,    -1,   430,     9,   335,
      -1,   335,   133,   335,    -1,   335,    -1,   432,   384,    -1,
      -1,   432,     9,   380,   133,   380,    -1,   432,     9,   380,
      -1,   380,   133,   380,    -1,   380,    -1,   433,   434,    -1,
     433,    87,    -1,   434,    -1,    87,   434,    -1,    81,    -1,
      81,    68,   435,   194,    -1,    81,   403,   204,    -1,   150,
     335,   191,    -1,   150,    80,    68,   335,   194,   191,    -1,
     151,   413,   191,    -1,   204,    -1,    82,    -1,    81,    -1,
     124,   187,   327,   188,    -1,   125,   187,   413,   188,    -1,
     125,   187,   336,   188,    -1,   125,   187,   417,   188,    -1,
     125,   187,   416,   188,    -1,   125,   187,   325,   188,    -1,
       7,   335,    -1,     6,   335,    -1,     5,   187,   335,   188,
      -1,     4,   335,    -1,     3,   335,    -1,   413,    -1,   437,
       9,   413,    -1,   374,   152,   205,    -1,   374,   152,   127,
      -1,    -1,    99,   459,    -1,   178,   444,    14,   459,   189,
      -1,   401,   178,   444,    14,   459,   189,    -1,   180,   444,
     439,    14,   459,   189,    -1,   401,   180,   444,   439,    14,
     459,   189,    -1,   206,    -1,   459,   206,    -1,   205,    -1,
     459,   205,    -1,   206,    -1,   206,   173,   451,   174,    -1,
     204,    -1,   204,   173,   451,   174,    -1,   173,   447,   174,
      -1,    -1,   459,    -1,   446,     9,   459,    -1,   446,   384,
      -1,   446,     9,   166,    -1,   447,    -1,   166,    -1,    -1,
      -1,    30,   459,    -1,    99,   459,    -1,   100,   459,    -1,
     451,     9,   452,   204,    -1,   452,   204,    -1,   451,     9,
     452,   204,   450,    -1,   452,   204,   450,    -1,    48,    -1,
      49,    -1,    -1,    88,   133,   459,    -1,    29,    88,   133,
     459,    -1,   215,   152,   204,   133,   459,    -1,   454,     9,
     453,    -1,   453,    -1,   454,   384,    -1,    -1,   177,   187,
     455,   188,    -1,   215,    -1,   204,   152,   458,    -1,   204,
     445,    -1,    29,   459,    -1,    57,   459,    -1,   215,    -1,
     135,    -1,   136,    -1,   456,    -1,   457,   152,   458,    -1,
     135,   173,   459,   174,    -1,   135,   173,   459,     9,   459,
     174,    -1,   157,    -1,   187,   108,   187,   448,   188,    30,
     459,   188,    -1,   187,   459,     9,   446,   384,   188,    -1,
     459,    -1,    -1
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
     877,   878,   879,   881,   886,   888,   893,   895,   897,   902,
     903,   907,   908,   910,   914,   921,   928,   932,   938,   940,
     943,   944,   945,   946,   949,   950,   954,   959,   959,   965,
     965,   972,   971,   977,   977,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   995,   996,
    1000,   998,  1007,  1005,  1012,  1020,  1014,  1024,  1022,  1026,
    1027,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,  1039,
    1047,  1047,  1052,  1058,  1062,  1062,  1070,  1071,  1075,  1076,
    1080,  1086,  1084,  1099,  1096,  1112,  1109,  1126,  1125,  1134,
    1132,  1144,  1143,  1162,  1160,  1179,  1178,  1187,  1185,  1196,
    1196,  1203,  1202,  1214,  1212,  1225,  1226,  1230,  1233,  1236,
    1237,  1238,  1241,  1242,  1245,  1247,  1250,  1251,  1254,  1255,
    1258,  1259,  1263,  1264,  1269,  1270,  1273,  1274,  1275,  1279,
    1280,  1284,  1285,  1289,  1290,  1294,  1295,  1300,  1301,  1306,
    1307,  1308,  1309,  1312,  1315,  1317,  1320,  1321,  1325,  1327,
    1330,  1333,  1336,  1337,  1340,  1341,  1345,  1351,  1357,  1364,
    1366,  1371,  1376,  1382,  1386,  1390,  1394,  1399,  1404,  1409,
    1414,  1420,  1429,  1434,  1439,  1445,  1447,  1451,  1455,  1460,
    1464,  1467,  1470,  1474,  1478,  1482,  1486,  1491,  1499,  1501,
    1504,  1505,  1506,  1507,  1509,  1511,  1516,  1517,  1520,  1521,
    1522,  1526,  1527,  1529,  1530,  1534,  1536,  1539,  1543,  1549,
    1551,  1554,  1554,  1558,  1557,  1561,  1563,  1566,  1569,  1567,
    1583,  1579,  1593,  1595,  1597,  1599,  1601,  1603,  1605,  1609,
    1610,  1611,  1614,  1620,  1624,  1630,  1633,  1638,  1640,  1645,
    1650,  1654,  1655,  1659,  1660,  1662,  1664,  1670,  1671,  1673,
    1677,  1678,  1683,  1687,  1688,  1692,  1693,  1697,  1699,  1705,
    1710,  1711,  1713,  1717,  1718,  1719,  1720,  1724,  1725,  1726,
    1727,  1728,  1729,  1731,  1736,  1739,  1740,  1744,  1745,  1749,
    1750,  1753,  1754,  1757,  1758,  1761,  1762,  1766,  1767,  1768,
    1769,  1770,  1771,  1772,  1776,  1777,  1780,  1781,  1782,  1785,
    1787,  1789,  1790,  1793,  1795,  1799,  1801,  1805,  1809,  1813,
    1818,  1819,  1821,  1822,  1823,  1824,  1827,  1831,  1832,  1836,
    1837,  1841,  1842,  1843,  1844,  1848,  1852,  1857,  1861,  1865,
    1870,  1871,  1872,  1873,  1874,  1878,  1880,  1881,  1882,  1885,
    1886,  1887,  1888,  1889,  1890,  1891,  1892,  1893,  1894,  1895,
    1896,  1897,  1898,  1899,  1900,  1901,  1902,  1903,  1904,  1905,
    1906,  1907,  1908,  1909,  1910,  1911,  1912,  1913,  1914,  1915,
    1916,  1917,  1918,  1919,  1920,  1921,  1922,  1923,  1924,  1925,
    1926,  1927,  1929,  1930,  1932,  1933,  1935,  1936,  1937,  1938,
    1939,  1940,  1941,  1942,  1943,  1944,  1945,  1946,  1947,  1948,
    1949,  1950,  1951,  1952,  1953,  1954,  1958,  1962,  1967,  1966,
    1981,  1979,  1997,  1996,  2015,  2014,  2033,  2032,  2050,  2050,
    2065,  2065,  2083,  2084,  2085,  2090,  2092,  2096,  2100,  2106,
    2110,  2116,  2118,  2122,  2124,  2128,  2132,  2133,  2137,  2144,
    2151,  2153,  2158,  2159,  2160,  2161,  2163,  2167,  2168,  2169,
    2170,  2174,  2180,  2189,  2202,  2203,  2206,  2209,  2212,  2213,
    2216,  2220,  2223,  2226,  2233,  2234,  2238,  2239,  2241,  2245,
    2246,  2247,  2248,  2249,  2250,  2251,  2252,  2253,  2254,  2255,
    2256,  2257,  2258,  2259,  2260,  2261,  2262,  2263,  2264,  2265,
    2266,  2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,  2275,
    2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,
    2286,  2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,  2295,
    2296,  2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,  2305,
    2306,  2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314,  2315,
    2316,  2317,  2318,  2319,  2320,  2321,  2322,  2323,  2324,  2328,
    2333,  2334,  2338,  2339,  2340,  2341,  2343,  2347,  2348,  2359,
    2360,  2362,  2374,  2375,  2376,  2380,  2381,  2382,  2386,  2387,
    2388,  2391,  2393,  2397,  2398,  2399,  2400,  2402,  2403,  2404,
    2405,  2406,  2407,  2408,  2409,  2410,  2411,  2414,  2419,  2420,
    2421,  2423,  2424,  2426,  2427,  2428,  2429,  2431,  2433,  2435,
    2437,  2439,  2440,  2441,  2442,  2443,  2444,  2445,  2446,  2447,
    2448,  2449,  2450,  2451,  2452,  2453,  2454,  2455,  2457,  2459,
    2461,  2463,  2464,  2467,  2468,  2472,  2476,  2478,  2482,  2485,
    2488,  2494,  2495,  2496,  2497,  2498,  2499,  2500,  2505,  2507,
    2511,  2512,  2515,  2516,  2520,  2523,  2525,  2527,  2531,  2532,
    2533,  2534,  2537,  2541,  2542,  2543,  2544,  2548,  2550,  2557,
    2558,  2559,  2560,  2561,  2562,  2564,  2565,  2570,  2572,  2575,
    2578,  2580,  2582,  2585,  2587,  2591,  2593,  2596,  2599,  2605,
    2607,  2610,  2611,  2616,  2619,  2623,  2623,  2628,  2631,  2632,
    2636,  2637,  2641,  2642,  2643,  2647,  2652,  2657,  2658,  2662,
    2667,  2672,  2673,  2677,  2678,  2683,  2685,  2690,  2701,  2715,
    2727,  2742,  2743,  2744,  2745,  2746,  2747,  2748,  2758,  2767,
    2769,  2771,  2775,  2776,  2777,  2778,  2779,  2795,  2796,  2798,
    2800,  2807,  2808,  2809,  2810,  2811,  2812,  2813,  2814,  2816,
    2821,  2825,  2826,  2830,  2833,  2840,  2844,  2853,  2860,  2868,
    2870,  2871,  2875,  2876,  2878,  2883,  2884,  2895,  2896,  2897,
    2898,  2909,  2912,  2915,  2916,  2917,  2918,  2929,  2933,  2934,
    2935,  2937,  2938,  2939,  2943,  2945,  2948,  2950,  2951,  2952,
    2953,  2956,  2958,  2959,  2963,  2965,  2968,  2970,  2971,  2972,
    2976,  2978,  2981,  2984,  2986,  2988,  2992,  2993,  2995,  2996,
    3002,  3003,  3005,  3015,  3017,  3019,  3022,  3023,  3024,  3028,
    3029,  3030,  3031,  3032,  3033,  3034,  3035,  3036,  3037,  3038,
    3042,  3043,  3047,  3049,  3057,  3059,  3063,  3067,  3072,  3076,
    3084,  3085,  3089,  3090,  3096,  3097,  3106,  3107,  3115,  3118,
    3122,  3125,  3130,  3135,  3137,  3138,  3139,  3143,  3144,  3148,
    3149,  3152,  3155,  3157,  3161,  3167,  3168,  3169,  3173,  3177,
    3187,  3195,  3197,  3201,  3203,  3208,  3214,  3217,  3222,  3230,
    3233,  3236,  3237,  3240,  3243,  3244,  3249,  3252,  3256,  3260,
    3266,  3276,  3277
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
  "ident_for_class_const", "ident", "group_use_prefix", "use_declarations",
  "use_declaration", "mixed_use_declarations", "mixed_use_declaration",
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
     201,   201,   201,   201,   201,   204,   204,   204,   204,   204,
     204,   204,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   206,   206,   207,   207,   208,   208,
     209,   209,   209,   209,   210,   210,   211,   211,   211,   212,
     212,   213,   213,   213,   214,   215,   216,   216,   217,   217,
     218,   218,   218,   218,   219,   219,   219,   220,   219,   221,
     219,   222,   219,   223,   219,   219,   219,   219,   219,   219,
     219,   219,   219,   219,   219,   219,   219,   219,   219,   219,
     224,   219,   225,   219,   219,   226,   219,   227,   219,   219,
     219,   219,   219,   219,   219,   219,   219,   219,   219,   219,
     229,   228,   230,   230,   232,   231,   233,   233,   234,   234,
     235,   237,   236,   238,   236,   239,   236,   241,   240,   242,
     240,   244,   243,   245,   243,   246,   243,   247,   243,   249,
     248,   251,   250,   252,   250,   253,   253,   254,   255,   256,
     256,   256,   256,   256,   257,   257,   258,   258,   259,   259,
     260,   260,   261,   261,   262,   262,   263,   263,   263,   264,
     264,   265,   265,   266,   266,   267,   267,   268,   268,   269,
     269,   269,   269,   270,   270,   270,   271,   271,   272,   272,
     273,   273,   274,   274,   275,   275,   276,   276,   276,   276,
     276,   276,   276,   276,   277,   277,   277,   277,   277,   277,
     277,   277,   278,   278,   278,   278,   278,   278,   278,   278,
     279,   279,   279,   279,   279,   279,   279,   279,   280,   280,
     281,   281,   281,   281,   281,   281,   282,   282,   283,   283,
     283,   284,   284,   284,   284,   285,   285,   286,   287,   288,
     288,   290,   289,   291,   289,   289,   289,   289,   292,   289,
     293,   289,   289,   289,   289,   289,   289,   289,   289,   294,
     294,   294,   295,   296,   296,   297,   297,   298,   298,   299,
     299,   300,   300,   301,   301,   301,   301,   301,   301,   301,
     302,   302,   303,   304,   304,   305,   305,   306,   306,   307,
     308,   308,   308,   309,   309,   309,   309,   310,   310,   310,
     310,   310,   310,   310,   311,   311,   311,   312,   312,   313,
     313,   314,   314,   315,   315,   316,   316,   317,   317,   317,
     317,   317,   317,   317,   318,   318,   319,   319,   319,   320,
     320,   320,   320,   321,   321,   322,   322,   323,   323,   324,
     325,   325,   325,   325,   325,   325,   326,   327,   327,   328,
     328,   329,   329,   329,   329,   330,   331,   332,   333,   334,
     335,   335,   335,   335,   335,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   337,   337,   339,   338,
     340,   338,   342,   341,   343,   341,   344,   341,   345,   341,
     346,   341,   347,   347,   347,   348,   348,   349,   349,   350,
     350,   351,   351,   352,   352,   353,   354,   354,   355,   356,
     357,   357,   358,   358,   358,   358,   358,   359,   359,   359,
     359,   360,   361,   361,   362,   362,   363,   363,   364,   364,
     365,   366,   366,   367,   367,   367,   368,   368,   368,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   370,
     371,   371,   372,   372,   372,   372,   372,   373,   373,   374,
     374,   374,   375,   375,   375,   376,   376,   376,   377,   377,
     377,   378,   378,   379,   379,   379,   379,   379,   379,   379,
     379,   379,   379,   379,   379,   379,   379,   379,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   381,   381,
     381,   382,   382,   382,   382,   382,   382,   382,   383,   383,
     384,   384,   385,   385,   386,   386,   386,   386,   387,   387,
     387,   387,   387,   388,   388,   388,   388,   389,   389,   390,
     390,   390,   390,   390,   390,   390,   390,   391,   391,   392,
     392,   392,   392,   393,   393,   394,   394,   395,   395,   396,
     396,   397,   397,   398,   398,   400,   399,   401,   402,   402,
     403,   403,   404,   404,   404,   405,   405,   406,   406,   407,
     407,   408,   408,   409,   409,   410,   410,   411,   411,   412,
     412,   413,   413,   413,   413,   413,   413,   413,   413,   413,
     413,   413,   414,   414,   414,   414,   414,   414,   414,   414,
     414,   415,   415,   415,   415,   415,   415,   415,   415,   415,
     416,   417,   417,   418,   418,   419,   419,   419,   420,   421,
     421,   421,   422,   422,   422,   423,   423,   424,   424,   424,
     424,   424,   424,   425,   425,   425,   425,   425,   426,   426,
     426,   426,   426,   426,   427,   427,   428,   428,   428,   428,
     428,   428,   428,   428,   429,   429,   430,   430,   430,   430,
     431,   431,   432,   432,   432,   432,   433,   433,   433,   433,
     434,   434,   434,   434,   434,   434,   435,   435,   435,   436,
     436,   436,   436,   436,   436,   436,   436,   436,   436,   436,
     437,   437,   438,   438,   439,   439,   440,   440,   440,   440,
     441,   441,   442,   442,   443,   443,   444,   444,   445,   445,
     446,   446,   447,   448,   448,   448,   448,   449,   449,   450,
     450,   451,   451,   451,   451,   452,   452,   452,   453,   453,
     453,   454,   454,   455,   455,   456,   457,   458,   458,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   460,   460
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
       1,     2,     3,     4,     3,     1,     1,     2,     2,     1,
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
       0,   421,     0,   785,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   875,     0,
     863,   667,     0,   673,   674,   675,    25,   732,   852,   148,
     149,   676,     0,   129,     0,     0,     0,     0,    26,     0,
       0,     0,     0,   180,     0,     0,     0,     0,     0,     0,
     387,   388,   389,   392,   391,   390,     0,     0,     0,     0,
     209,     0,     0,     0,   680,   682,   683,   677,   678,     0,
       0,     0,   684,   679,     0,   651,    27,    28,    29,    31,
      30,     0,   681,     0,     0,     0,     0,   685,   393,   520,
       0,   147,   119,     0,   668,     0,     0,     4,   109,   111,
     731,     0,   650,     0,     6,   179,     7,     9,     8,    10,
       0,     0,   385,   432,     0,     0,     0,     0,     0,     0,
       0,   430,   841,   842,   502,   501,   415,   505,     0,   414,
     812,   652,   659,     0,   734,   500,   384,   815,   816,   827,
     431,     0,     0,   434,   433,   813,   814,   811,   848,   851,
     490,   733,    11,   392,   391,   390,     0,     0,    31,     0,
     109,   179,     0,   919,   431,   918,     0,   916,   915,   504,
       0,   422,   427,     0,     0,   472,   473,   474,   475,   499,
     497,   496,   495,   494,   493,   492,   491,   852,   676,   654,
       0,     0,   939,   834,   652,     0,   653,   454,     0,   452,
       0,   879,     0,   741,   413,   663,   199,     0,   939,   412,
     662,   657,     0,   672,   653,   858,   859,   865,   857,   664,
       0,     0,   666,   498,     0,     0,     0,     0,   418,     0,
     127,   420,     0,     0,   133,   135,     0,     0,   137,     0,
      72,    71,    66,    65,    57,    58,    49,    69,    80,     0,
      52,     0,    64,    56,    62,    82,    75,    74,    47,    70,
      89,    90,    48,    85,    45,    86,    46,    87,    44,    91,
      79,    83,    88,    76,    77,    51,    78,    81,    43,    73,
      59,    92,    67,    60,    50,    42,    41,    40,    39,    38,
      37,    61,    93,    95,    54,    35,    36,    63,   972,   973,
      55,   978,    34,    53,    84,     0,     0,   109,    94,   930,
     971,     0,   974,     0,     0,   139,     0,     0,   170,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   298,     0,
       0,   297,     0,   213,     0,   210,   303,     0,     0,     0,
       0,     0,   936,   195,   207,   871,   875,     0,   900,     0,
     687,     0,     0,     0,   898,     0,    16,     0,   113,   187,
     201,   208,   557,   532,     0,   924,   512,   514,   516,   789,
     421,   432,     0,     0,   430,   431,   433,     0,     0,   854,
     669,     0,   670,     0,     0,     0,   169,     0,     0,   115,
     289,     0,    24,   178,     0,   206,   191,   205,   390,   393,
     179,   386,   162,   163,   164,   165,   166,   168,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   863,     0,   161,   856,   856,
     885,     0,     0,     0,     0,     0,     0,     0,     0,   383,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   453,   451,   790,   791,     0,   856,     0,
     803,   289,   289,   856,     0,   871,     0,   179,     0,     0,
     141,     0,   787,   782,   741,     0,   432,   430,     0,   883,
       0,   537,   740,   874,   672,   432,   430,   431,   115,     0,
     289,   411,     0,   805,   665,     0,   119,   249,     0,   519,
       0,   144,     0,     0,   419,     0,     0,     0,     0,     0,
     136,   160,   138,   972,   973,   969,   970,     0,   964,     0,
       0,     0,     0,    68,    33,    55,    32,   931,   167,   140,
     119,     0,   157,   159,     0,     0,     0,     0,   101,     0,
       0,    18,     0,    96,     0,   299,     0,   142,   212,   211,
       0,     0,   143,   920,     0,     0,   432,   430,   431,   434,
     433,     0,   957,   219,     0,   872,     0,     0,   145,     0,
       0,   686,   899,   732,     0,     0,   897,   737,   896,   112,
       5,    13,    14,     0,   217,     0,     0,   525,     0,     0,
     741,     0,     0,   660,   655,   526,     0,     0,     0,     0,
     789,   119,     0,   743,   788,   982,   410,   424,   486,   821,
     840,   124,   118,   120,   121,   122,   123,   384,     0,   503,
     735,   736,   110,   741,     0,   940,     0,     0,     0,   743,
     290,     0,   508,   181,   215,     0,   457,   459,   458,     0,
       0,   489,   455,   456,   460,   462,   461,   477,   476,   479,
     478,   480,   482,   484,   483,   481,   471,   470,   464,   465,
     463,   466,   467,   469,   485,   468,   855,     0,     0,   889,
       0,   741,   923,     0,   922,   939,   818,   197,   189,   203,
       0,   924,   193,   179,     0,   425,   428,   436,   450,   449,
     448,   447,   446,   445,   444,   443,   442,   441,   440,   439,
     793,     0,   792,   795,   817,   799,   939,   796,     0,     0,
       0,     0,     0,     0,     0,     0,   917,   423,   780,   784,
     740,   786,     0,   656,     0,   878,     0,   877,   215,     0,
     656,   862,   861,   848,   851,     0,     0,   792,   795,   860,
     796,   416,   251,   253,   119,   523,   522,   417,     0,   119,
     233,   128,   420,     0,     0,     0,     0,     0,   245,   245,
     134,     0,     0,     0,     0,   962,   741,     0,   946,     0,
       0,     0,     0,     0,   739,     0,   651,     0,     0,   689,
     650,   694,     0,   688,   117,   693,   939,   975,     0,     0,
       0,     0,    19,     0,    20,     0,    97,     0,     0,     0,
     106,     0,   105,   100,    98,   102,     0,   296,   304,   301,
       0,     0,   909,   914,   911,   910,   913,   912,    12,   955,
     956,     0,     0,     0,     0,   871,   868,     0,   536,   908,
     907,   906,     0,   902,     0,   903,   905,     0,     5,     0,
       0,     0,   551,   552,   560,   559,     0,   430,     0,   740,
     531,   535,     0,     0,   925,     0,   513,     0,     0,   947,
     789,   275,   981,     0,     0,   804,     0,   853,   740,   942,
     938,   291,   292,   649,   742,   288,     0,   789,     0,     0,
     217,   510,   183,   488,     0,   540,   541,     0,   538,   740,
     884,     0,     0,   289,   219,     0,   217,     0,     0,   215,
       0,   863,   437,     0,     0,   801,   802,   819,   820,   849,
     850,     0,     0,     0,   768,   748,   749,   750,   757,     0,
       0,     0,   761,   759,   760,   774,   741,     0,   782,   882,
     881,     0,   217,     0,   806,   671,     0,   255,     0,     0,
     125,     0,     0,     0,     0,     0,     0,     0,   225,   226,
     237,     0,   119,   235,   154,   245,     0,   245,     0,     0,
     976,     0,     0,     0,   740,   963,   965,   945,   741,   944,
       0,   741,   715,   716,   713,   714,   747,     0,   741,   739,
       0,   534,     0,     0,   891,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   968,   171,     0,   174,   158,     0,     0,
     103,   107,   108,   101,     0,     0,   300,     0,   921,   146,
     957,   937,   952,   218,   220,   310,     0,     0,   869,     0,
     901,     0,    17,     0,   924,   216,   310,     0,     0,   656,
     528,     0,   661,   926,     0,   947,   517,     0,     0,   982,
       0,   280,   278,   795,   807,   939,   795,   808,   941,     0,
       0,   293,   116,     0,   789,   214,     0,   789,     0,   487,
     888,   887,     0,   289,     0,     0,     0,     0,     0,     0,
     217,   185,   672,   794,   289,     0,   753,   754,   755,   756,
     762,   763,   772,     0,   741,     0,   768,     0,   752,   776,
     740,   779,   781,   783,     0,   876,     0,   794,     0,     0,
       0,     0,   252,   524,   130,     0,   420,   225,   227,   871,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   239,
       0,     0,   958,     0,   961,   740,     0,     0,     0,   691,
     740,   738,     0,   729,     0,   741,     0,   695,   730,   728,
     895,     0,   741,   698,   700,   699,     0,     0,   696,   697,
     701,   703,   702,   718,   717,   720,   719,   721,   723,   725,
     724,   722,   711,   710,   705,   706,   704,   707,   708,   709,
     712,   967,     0,   119,     0,     0,   104,    21,   302,     0,
       0,     0,   954,     0,   384,   873,   871,   426,   429,   435,
       0,    15,     0,   384,   563,     0,     0,   565,   558,   561,
       0,   556,     0,   928,     0,   948,   521,     0,   281,     0,
       0,   276,     0,   295,   294,   947,     0,   310,     0,   789,
       0,   289,     0,   846,   310,   924,   310,   927,     0,     0,
       0,   438,     0,     0,   765,   740,   767,   758,     0,   751,
       0,     0,   741,   773,   880,   310,     0,   119,     0,   248,
     234,     0,     0,     0,   224,   150,   238,     0,     0,   241,
       0,   246,   247,   119,   240,   977,   959,     0,   943,     0,
     980,   746,   745,   690,     0,   740,   533,   692,     0,   539,
     740,   890,   727,     0,     0,     0,    22,    23,   951,   949,
     950,   221,     0,     0,     0,   391,   382,     0,     0,     0,
     196,   309,   311,     0,   381,     0,     0,     0,   924,   384,
       0,   904,   306,   202,   554,     0,     0,   527,   515,     0,
     284,   274,     0,   277,   283,   289,   507,   947,   384,   947,
       0,   886,     0,   845,   384,     0,   384,   929,   310,   789,
     843,   771,   770,   764,     0,   766,   740,   775,   384,   119,
     254,   126,   131,   152,   228,     0,   236,   242,   119,   244,
     960,     0,     0,   530,     0,   894,   893,   726,   119,   175,
     953,     0,     0,     0,   932,     0,     0,     0,   222,     0,
     924,     0,   347,   343,   349,   651,    31,     0,   337,     0,
     342,   346,   359,     0,   357,   362,     0,   361,     0,   360,
       0,   179,   313,     0,   315,     0,   316,   317,     0,     0,
     870,     0,   555,   553,   564,   562,   285,     0,     0,   272,
     282,     0,     0,     0,     0,   192,   507,   947,   847,   198,
     306,   204,   384,     0,     0,   778,     0,   200,   250,     0,
       0,   119,   231,   151,   243,   979,   744,     0,     0,     0,
       0,     0,   409,     0,   933,     0,   327,   331,   406,   407,
     341,     0,     0,     0,   322,   615,   614,   611,   613,   612,
     632,   634,   633,   603,   574,   575,   593,   609,   608,   570,
     580,   581,   583,   582,   602,   586,   584,   585,   587,   588,
     589,   590,   591,   592,   594,   595,   596,   597,   598,   599,
     601,   600,   571,   572,   573,   576,   577,   579,   617,   618,
     627,   626,   625,   624,   623,   622,   610,   629,   619,   620,
     621,   604,   605,   606,   607,   630,   631,   635,   637,   636,
     638,   639,   616,   641,   640,   643,   645,   644,   578,   648,
     646,   647,   642,   628,   569,   354,   566,     0,   323,   375,
     376,   374,   367,     0,   368,   324,   401,     0,     0,     0,
       0,   405,     0,   179,   188,   305,     0,     0,     0,   273,
     287,   844,     0,   119,   377,   119,   182,     0,     0,     0,
     194,   947,   769,     0,   119,   229,   132,   153,     0,   529,
     892,   173,   325,   326,   404,   223,     0,     0,   741,     0,
     350,   338,     0,     0,     0,   356,   358,     0,     0,   363,
     370,   371,   369,     0,     0,   312,   934,     0,     0,     0,
     408,     0,   307,     0,   286,     0,   549,   743,     0,     0,
     119,   184,   190,     0,   777,     0,     0,   155,   328,   109,
       0,   329,   330,     0,     0,   344,   740,   352,   348,   353,
     567,   568,     0,   339,   372,   373,   365,   366,   364,   402,
     399,   957,   318,   314,   403,     0,   308,   550,   742,     0,
     509,   378,     0,   186,     0,   232,     0,   177,     0,   384,
       0,   351,   355,     0,     0,   789,   320,     0,   547,   506,
     511,   230,     0,     0,   156,   335,     0,   383,   345,   400,
     935,     0,   743,   395,   789,   548,     0,   176,     0,     0,
     334,   947,   789,   259,   396,   397,   398,   982,   394,     0,
       0,     0,   333,     0,   395,     0,   947,     0,   332,   379,
     119,   319,   982,     0,   264,   262,     0,   119,     0,     0,
     265,     0,     0,   260,   321,     0,   380,     0,   268,   258,
       0,   261,   267,   172,   269,     0,     0,   256,   266,     0,
     257,   271,   270
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   858,   600,   170,  1414,   695,
     334,   335,   336,   821,   822,   109,   110,   111,   112,   113,
     387,   632,   633,   523,   239,  1479,   529,  1395,  1480,  1717,
     810,   329,   550,  1677,  1037,  1213,  1734,   404,   171,   634,
     898,  1098,  1270,   117,   603,   915,   635,   654,   919,   583,
     914,   219,   504,   636,   604,   916,   406,   353,   370,   120,
     900,   861,   844,  1053,  1417,  1151,   968,  1626,  1483,   771,
     974,   528,   780,   976,  1303,   763,   957,   960,  1140,  1741,
    1742,   622,   623,   648,   649,   340,   341,   347,  1451,  1605,
    1606,  1224,  1341,  1440,  1599,  1725,  1744,  1636,  1681,  1682,
    1683,  1427,  1428,  1429,  1430,  1638,  1639,  1645,  1693,  1433,
    1434,  1438,  1592,  1593,  1594,  1616,  1771,  1342,  1343,   172,
     122,  1757,  1758,  1597,  1345,  1346,  1347,  1348,   123,   232,
     524,   525,   124,   125,   126,   127,   128,   129,   130,   131,
    1463,   132,   897,  1097,   133,   619,   620,   621,   236,   379,
     519,   609,   610,  1175,   611,  1176,   134,   135,   136,   801,
     137,   138,  1667,   139,   605,  1453,   606,  1067,   866,  1241,
    1238,  1585,  1586,   140,   141,   142,   222,   143,   223,   233,
     391,   511,   144,   996,   805,   145,   997,   889,   881,   998,
     943,  1120,   944,  1122,  1123,  1124,   946,  1281,  1282,   947,
     739,   494,   183,   184,   637,   625,   477,  1083,  1084,   725,
     726,   885,   147,   225,   148,   149,   174,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   687,   229,   230,   586,
     212,   213,   690,   691,  1181,  1182,   363,   364,   852,   160,
     574,   161,   618,   162,   321,  1607,  1657,   354,   399,   643,
     644,   990,  1078,  1222,   841,   842,   785,   786,   787,   322,
     323,   807,  1416,   883
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1103
static const yytype_int16 yypact[] =
{
   -1103,   174, -1103, -1103,  5475, 13041, 13041,   -18, 13041, 13041,
   13041, 10907, 13041, -1103, 13041, 13041, 13041, 13041, 13041, 13041,
   13041, 13041, 13041, 13041, 13041, 13041, 15549, 15549, 11101, 13041,
   16004,     2,    15, -1103, -1103, -1103, -1103, -1103,   201, -1103,
   -1103,   236, 13041, -1103,    15,    24,    47,   177, -1103,    15,
   11295,  1001, 11489, -1103, 13925,  9937,   183, 13041,  1234,   116,
   -1103, -1103, -1103,    60,    65,    54,   180,   212,   223,   255,
   -1103,  1001,   281,   283, -1103, -1103, -1103, -1103, -1103, 13041,
     648,  1027, -1103, -1103,  1001, -1103, -1103, -1103, -1103,  1001,
   -1103,  1001, -1103,   384,   314,  1001,  1001, -1103,   456, -1103,
   11683, -1103, -1103,   391,   592,   606,   606, -1103,   516,   388,
     669,   363, -1103,   103, -1103,   549, -1103, -1103, -1103, -1103,
     903,  1319, -1103, -1103,   411,   431,   434,   463,   476,   481,
    4992, -1103, -1103, -1103, -1103,   322, -1103,   612,   628, -1103,
     173,   510, -1103,   553,     8, -1103,  2006,   184, -1103, -1103,
    2464,   171,   520,   333, -1103,   176,   329,   528,   338, -1103,
   -1103,   652, -1103, -1103, -1103,   570,   540,   578, -1103, 13041,
   -1103,   549,  1319, 16469,  3299, 16469, 13041, 16469, 16469, 14372,
     560, 15038, 14372,   720,  1001,   713,   713,   586,   713,   713,
     713,   713,   713,   713,   713,   713,   713, -1103, -1103, -1103,
      81, 13041,   610, -1103, -1103,   640,   601,   545,   616,   545,
   15549, 15248,   614,   809, -1103,   570, -1103, 13041,   610, -1103,
     673, -1103,   683,   646, -1103,   187, -1103, -1103, -1103,   545,
     171, 11877, -1103, -1103, 13041,  8773,   830,   111, 16469,  9743,
   -1103, 13041, 13041,  1001, -1103, -1103, 10892,   664, -1103, 11280,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,  3536,
   -1103,  3536, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,   110,   113,
     578, -1103, -1103, -1103, -1103,   668,  2091,   114, -1103, -1103,
     706,   846, -1103,   721, 14594, -1103,   693, 11862, -1103,    20,
   13026,  1218,  1218,  1001,   697,   120, -1103,    84, -1103,  3869,
     128, -1103,   765, -1103,   767, -1103,   876,   132, 15549, 13041,
   13041,   715,   732, -1103, -1103, 15266, 11101,   133,    93,    49,
   -1103, 13235, 15549,   681, -1103,  1001, -1103,   213,   388, -1103,
   -1103, -1103, -1103, 16097,   893,   810, -1103, -1103, -1103,    79,
   13041,   724,   726, 16469,   740,   593,   755,  5669, 13041, -1103,
     497,   754,   636,   497,   568,   558, -1103,  1001,  3536,   762,
   10131, 13925, -1103, -1103,   853, -1103, -1103, -1103, -1103, -1103,
     549, -1103, -1103, -1103, -1103, -1103, -1103, -1103, 13041, 13041,
   13041, 12071, 13041, 13041, 13041, 13041, 13041, 13041, 13041, 13041,
   13041, 13041, 13041, 13041, 13041, 13041, 13041, 13041, 13041, 13041,
   13041, 13041, 13041, 13041, 13041, 16190, 13041, -1103, 13041, 13041,
   13041, 13374,  1001,  1001,  1001,  1001,  1001,   903,   844,  1424,
    4824, 13041, 13041, 13041, 13041, 13041, 13041, 13041, 13041, 13041,
   13041, 13041, 13041, -1103, -1103, -1103, -1103,   647, 13041, 13041,
   -1103, 10131, 10131, 13041, 13041, 15266,   768,   549, 12265, 13220,
   -1103, 13041, -1103,   772,   960,   823,   785,   787, 13508,   545,
   12459, -1103, 12653, -1103,   646,   788,   789,  2193, -1103,   312,
   10131, -1103,   660, -1103, -1103, 14532, -1103, -1103, 10325, -1103,
   13041, -1103,   889,  8967,   974,   797, 16352,   970,   106,    66,
   -1103, -1103, -1103,   815, -1103, -1103, -1103,  3536,   473,   811,
     987, 15173,  1001, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103,   813, -1103, -1103,   812,   134,   814,   135,   446,  1246,
    1486, -1103,  1001,  1001, 13041,   545,   116, -1103, -1103, -1103,
   15173,   924, -1103,   545,   109,   122,   818,   832,  2289,    45,
     834,   835,   542,   896,   841,   545,   130,   843, -1103,  1686,
    1001, -1103, -1103,   966,  2919,    19, -1103, -1103, -1103,   388,
   -1103, -1103, -1103,  1005,   906,   865,   417,   886, 13041,   910,
    1035,   857,   894, -1103,   310, -1103,  3536,  3536,  1033,   830,
      79, -1103,   864,  1041, -1103,  3536,   137, -1103,   512,   192,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103,  1542,  2965, -1103,
   -1103, -1103, -1103,  1046,   885, -1103, 15549, 13041,   873,  1055,
   16469,  1059, -1103, -1103,   942,  1199, 11474, 16511, 14372, 13041,
   16423, 16590, 16661, 10111,  2567, 12243, 12630, 12824, 12824, 12824,
   12824,  2873,  2873,  2873,  2873,  2873,  1626,  1626,   761,   761,
     761,   586,   586,   586, -1103,   713, 16469,   880,   881, 15711,
     888,  1072,    -6, 13041,   395,   610,    41, -1103, -1103, -1103,
    1062,   810, -1103,   549, 15363, -1103, -1103, 14372, 14372, 14372,
   14372, 14372, 14372, 14372, 14372, 14372, 14372, 14372, 14372, 14372,
   -1103, 13041,   451, -1103,   321, -1103,   610,   487,   895,  3012,
     899,   907,   902,  3234,   148,   915, -1103, 16469,   613, -1103,
    1001, -1103,   137,   499, 15549, 16469, 15549, 15757,   942,   137,
     545,   332, -1103,   310,   951,   916, 13041, -1103,   341, -1103,
   -1103, -1103,  8579,   679, -1103, -1103, 16469, 16469,    15, -1103,
   -1103, -1103, 13041,  1006, 15056, 15173,  1001,  9161,   922,   925,
   -1103,    92,  1031,   991,   968, -1103,  1121,   945,  2609,  3536,
   15173, 15173, 15173, 15173, 15173,   950,   986,   952, 15173,   684,
     992, -1103,   955, -1103,  4768, -1103,   218, -1103,  5863,  1394,
     957,  1486, -1103,  1486, -1103,  1001,  1001,  1486,  1486,  1001,
   -1103,    88, -1103,   448, -1103, -1103,  3493, -1103,  4768,  1128,
   15549,   969, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103,    96,  1001,  1394,   977, 15266, 15456,  1145, -1103, -1103,
   -1103, -1103,   978, -1103, 13041, -1103, -1103,  5069, -1103,  3536,
    1394,   981, -1103, -1103, -1103, -1103,  1148,   994, 13041, 16097,
   -1103, -1103, 13374,   996, -1103,  3536, -1103,  1007,  6057,  1149,
      69, -1103, -1103,   141,   647, -1103,   660, -1103,  3536, -1103,
   -1103,   545, 16469, -1103, 10519, -1103, 15173,   108,  1015,  1394,
     906, -1103, -1103, 16590, 13041, -1103, -1103, 13041, -1103, 13041,
   -1103,  3539,  1016, 10131,   896,  1174,   906,  3536,  1194,   942,
    1001, 16190,   545,  3730,  1022, -1103, -1103,   194,  1023, -1103,
   -1103,  1201,  1173,  1173,   613, -1103, -1103, -1103,  1163,  1029,
     423,  1032, -1103, -1103, -1103, -1103,  1209,  1034,   772,   545,
     545, 12847,   906,   660, -1103, -1103,  3855,   685,    15,  9743,
   -1103,  6251,  1039,  6445,  1043, 15056, 15549,  1047,  1087,   545,
    4768,  1207, -1103, -1103, -1103, -1103,   763, -1103,   472,  3536,
   -1103,  1097,  3536,  1001,   473, -1103, -1103, -1103,  1224, -1103,
    1048,  1046,   795,   795,  1175,  1175, 15862,  1053,  1239, 15173,
   14862, 16097,  2394, 14728, 15173, 15173, 15173, 15173, 14963, 15173,
   15173, 15173, 15173, 15173, 15173, 15173, 15173, 15173, 15173, 15173,
   15173, 15173, 15173, 15173, 15173, 15173, 15173, 15173, 15173, 15173,
   15173, 15173,  1001, -1103, -1103,  1172, -1103, -1103,    89,    90,
   -1103, -1103, -1103,   494,  1246,  1065, -1103, 15173,   545, -1103,
     542, -1103,   670,  1248, -1103, -1103,   166,  1071,   545, 10713,
   -1103,  2688, -1103,  5281,   810,  1248, -1103,   514,    33, -1103,
   16469,  1126,  1074, -1103,  1075,  1149, -1103,  3536,   830,  3536,
     284,  1249,  1184,   344, -1103,   610,   380, -1103, -1103, 15549,
   13041, 16469,  4768,  1081,   108, -1103,  1077,   108,  1084, 16590,
   16469, 15816,  1085, 10131,  1088,  1089,  3536,  1090,  1092,  3536,
     906, -1103,   646,   495, 10131, 13041, -1103, -1103, -1103, -1103,
   -1103, -1103,  1144,  1091,  1283,  1205,   613,  1146, -1103, 16097,
     613, -1103, -1103, -1103, 15549, 16469,  1104, -1103,    15,  1268,
    1225,  9743, -1103, -1103, -1103,  1112, 13041,  1087,   545, 15266,
   15056,  1114, 15173,  6639,   776,  1116, 13041,    76,   493, -1103,
    1129,  3536, -1103,  1177, -1103,  2685,  1276,  1119, 15173, -1103,
   15173, -1103,  1120, -1103,  1178,  1306,  1132, -1103, -1103, -1103,
   15921,  1125,  1312, 14591, 16553,  3348, 15173,  4380, 16696, 10499,
   11080, 12437, 13509, 13642, 13642, 13642, 13642,  2510,  2510,  2510,
    2510,  2510,  1765,  1765,   795,   795,   795,  1175,  1175,  1175,
    1175, -1103,  1134, -1103,  1138,  1140, -1103, -1103,  4768,  1001,
    3536,  3536, -1103,  1394,    99, -1103, 15266, -1103, -1103, 14372,
    1139, -1103,  1133,   129, -1103,    50, 13041, -1103, -1103, -1103,
   13041, -1103, 13041, -1103,   830, -1103, -1103,   323,  1324,  1266,
   13041, -1103,  1161,   545, 16469,  1149,  1168, -1103,  1176,   108,
   13041, 10131,  1181, -1103, -1103,   810, -1103, -1103,  1160,  1182,
    1179, -1103,  1183,   613, -1103,   613, -1103, -1103,  1185, -1103,
    1232,  1191,  1376, -1103,   545, -1103,  1358, -1103,  1200, -1103,
   -1103,  1204,  1215,   169, -1103, -1103,  4768,  1206,  1222, -1103,
    4674, -1103, -1103, -1103, -1103, -1103, -1103,  3536, -1103,  3536,
   -1103,  4768, 15965, -1103, 15173, 16097, -1103, -1103, 15173, -1103,
   15173, -1103, 16626, 15173,  1214,  6833, -1103, -1103,   670, -1103,
   -1103, -1103,   714, 14064,  1394,  1298, -1103,   972,  1251,   953,
   -1103, -1103, -1103,   844,  4407,   136,   139,  1227,   810,  1424,
     170, -1103, -1103, -1103,  1256,  4071,  4255, 16469, -1103,   287,
    1400,  1337, 13041, -1103, 16469, 10131,  1307,  1149,  1079,  1149,
    1236, 16469,  1237, -1103,  1415,  1241,  1596, -1103, -1103,   108,
   -1103, -1103,  1289, -1103,   613, -1103, 16097, -1103,  1610, -1103,
    8579, -1103, -1103, -1103, -1103,  9355, -1103, -1103, -1103,  8579,
   -1103,  1253, 15173,  4768,  1296,  4768, 16023, 16626, -1103, -1103,
   -1103,  1394,  1394,  1001, -1103,  1420, 14862,    85, -1103, 14064,
     810,  1844, -1103,  1270, -1103,   140,  1254,   143, -1103, 14371,
   -1103, -1103, -1103,   145, -1103, -1103,  1127, -1103,  1258, -1103,
    1368,   549, -1103, 14203, -1103, 14203, -1103, -1103,  1437,   844,
   -1103, 13647, -1103, -1103, -1103, -1103,  1438,  1373, 13041, -1103,
   16469,  1267,  1271,  1269,   686, -1103,  1307,  1149, -1103, -1103,
   -1103, -1103,  1854,  1272,   613, -1103,  1328, -1103,  8579,  9549,
    9355, -1103, -1103, -1103,  8579, -1103,  4768, 15173, 15173,  7027,
    1274,  1280, -1103, 15173, -1103,  1394, -1103, -1103, -1103, -1103,
   -1103,  3536,  1485,   972, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103, -1103,   362, -1103,  1251, -1103, -1103,
   -1103, -1103, -1103,   125,   478, -1103,  1443,   157, 14594,  1368,
    1457, -1103,  3536,   549, -1103, -1103,  1287,  1458, 13041, -1103,
   16469, -1103,    72, -1103, -1103, -1103, -1103,  1288,   686, 13786,
   -1103,  1149, -1103,   613, -1103, -1103, -1103, -1103,  7221,  4768,
    4768, -1103, -1103, -1103,  4768, -1103,   990,    98,  1468,  1295,
   -1103, -1103, 15173, 14371, 14371,  1423, -1103,  1127,  1127,   682,
   -1103, -1103, -1103, 15173,  1408, -1103,  1317,  1305,   163, 15173,
   -1103,  1001, -1103, 15173, 16469,  1417, -1103,  1491,  7415,  7609,
   -1103, -1103, -1103,   686, -1103,  7803,  1314,  1392, -1103,  1410,
    1361, -1103, -1103,  1411,  3536, -1103,  1485, -1103, -1103,  4768,
   -1103, -1103,  1350, -1103,  1481, -1103, -1103, -1103, -1103,  4768,
    1507,   542, -1103, -1103,  4768,  1338,  4768, -1103,   146,  1339,
   -1103, -1103,  7997, -1103,  1349, -1103,  1352,  1356,  1001,  1424,
    1374, -1103, -1103, 15173,   127,   138, -1103,  1474, -1103, -1103,
   -1103, -1103,  1394,   957, -1103,  1393,  1001,  1057, -1103,  4768,
   -1103,  1371,  1555,   780,   138, -1103,  1490, -1103,  1394,  1377,
   -1103,  1149,   154, -1103, -1103, -1103, -1103,  3536, -1103,  1379,
    1384,   164, -1103,   711,   780,   326,  1149,  1386, -1103, -1103,
   -1103, -1103,  3536,   455,  1564,  1499,   711, -1103,  8191,   418,
    1567,  1501, 13041, -1103, -1103,  8385, -1103,   459,  1573,  1509,
   13041, -1103, 16469, -1103,  1574,  1514, 13041, -1103, 16469, 13041,
   -1103, 16469, 16469
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1103, -1103, -1103,  -517, -1103, -1103, -1103,   467,   -46,   -29,
     585,  -256,  -479, -1103,   557,   -17,  1492, -1103,  2832, -1103,
    -458, -1103,    28, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103, -1103, -1103,  -131, -1103, -1103,  -137,   121,    29,
   -1103, -1103, -1103, -1103, -1103, -1103,    34, -1103, -1103, -1103,
   -1103, -1103, -1103,    36, -1103, -1103,  1152,  1164,  1158,   -68,
    -648,  -812,   699,   757,  -133,   474,  -874, -1103,   147, -1103,
   -1103, -1103, -1103,  -690,   319, -1103, -1103, -1103, -1103,  -116,
   -1103,  -564, -1103,  -420, -1103, -1103,  1063, -1103,   161, -1103,
   -1103,  -980, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103,   144, -1103,   214, -1103, -1103, -1103, -1103, -1103,
      51, -1103,   302,  -722, -1103, -1102,  -132, -1103,  -125,   107,
    -119,  -112, -1103,    56, -1103, -1103, -1103,   330,   -13,    10,
      31,  -695,   -64, -1103, -1103,   -20, -1103, -1103,    -5,   -31,
     188, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103, -1103,
    -575,  -798, -1103, -1103, -1103, -1103, -1103,  1720, -1103, -1103,
   -1103, -1103, -1103,   597, -1103, -1103, -1103, -1103, -1103, -1103,
   -1103, -1103,  -707, -1103,  2332,    35, -1103,   522,  -361, -1103,
   -1103,  -441,  3613,  3190, -1103, -1103,   667,  -164,  -619, -1103,
   -1103,   735,   548,  -681,   554, -1103, -1103, -1103, -1103, -1103,
     734, -1103, -1103, -1103,   117,  -814,  -144,  -398,  -395, -1103,
     801,   -83, -1103, -1103,    40,    42,   671, -1103, -1103,   738,
      -7, -1103,  -345,    30,  -344,    55,   296, -1103, -1103,  -437,
    1331, -1103, -1103, -1103, -1103, -1103,   829,   599, -1103, -1103,
   -1103,  -334,  -670, -1103,  1290,  -705, -1103,   -69,  -172,   178,
     900, -1103, -1020,   361,   -11,   644,   712, -1103, -1103, -1103,
   -1103,   663,  1067, -1036
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -967
static const yytype_int16 yytable[] =
{
     173,   175,   411,   177,   178,   179,   181,   182,   318,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   458,   371,   211,   214,   319,   374,   375,   613,   614,
     895,   918,   114,   116,   486,   326,   382,   238,   118,   615,
     119,   337,   235,  1247,   876,   246,   508,   249,   734,   503,
     327,   407,   330,   411,   240,  1244,   877,   945,   762,   244,
     228,   730,   731,   748,   367,   221,  1079,   368,   480,   384,
     226,  1071,   227,   237,   238,   555,   557,   964,   457,   723,
     820,   824,   724,   857,   684,   512,  1233,   381,  1096,   978,
     755,  1147,   808,   386,  1495,   383,   778,  1044,   560,   560,
     952,   979,  1354,  1332,  1107,  1050,  1301,  1684,  1665,  -830,
     357,   121,   401,  -544,   758,   776,    13,   759,   830,   -68,
     520,   146,   -33,   -32,   -68,   115,    13,   -33,   -32,   560,
     358,   520,   551,  1332,  1647,   346,  1050,   566,   384,   846,
    1136,   571,   520,   560,   560,  1443,    13,   513,  1445,  -340,
     475,   476,  1503,  1666,  1587,    13,   381,   846,   389,  1648,
    -653,   589,   386,   878,   383,   753,  1654,   -95,   495,   176,
     497,   489,  1654,  1495,     3,   846,    13,  1080,   846,   846,
     342,   -95,  1727,   562,   344,    13,   506,   343,   496,   231,
     552,  1239,   345,  -661,   386,  -835,   383,   338,   591,   361,
     362,    13,   234,  1174,   505,   478,  -654,   372,  1333,  -518,
     856,   241,   383,  1334,   590,    60,    61,    62,   163,  1335,
     408,  1336,  1081,  1240,   475,   476,   515,  1728,   913,   515,
     360,  -830,   482,   836,   242,  1366,   238,   526,  1333,   478,
     563,  -822,   410,  1334,  -823,    60,    61,    62,   163,  1335,
     408,  1336,  -825,   459,  -742,  -864,   779,  -742,  1337,  1338,
    -829,  1339,  -828,   517,  -279,  1302,   980,   522,   475,   476,
    1051,  1110,  1685,   655,  1496,  1497,  1294,  1368,   318,  1045,
    1214,  1215,   409,   537,  1374,  1154,  1376,  1158,  1337,  1338,
    1340,  1339,   402,   487,   777,   547,  -279,   831,  1269,   -68,
     521,  1740,   -33,   -32,  -545,  1388,   961,  1082,   339,   561,
     832,   963,   409,  1649,   337,   337,   558,   567,   847,   577,
    1353,   572,   588,   812,   814,  1444,  -263,   479,  1446,  -340,
     741,  1280,  1504,  1093,  1588,   653,   931,   576,  1041,  1042,
     411,  1063,  -742,   580,   238,   383,  1655,  1464,   599,  1466,
     735,   211,  1703,  1768,  1225,   318,   594,  1394,  1450,  1359,
    -833,   479,  1773,  -822,   243,  1248,  -823,   348,  1456,   397,
    1032,  -832,   319,   328,  -825,   181,  1642,  -864,   483,  -836,
     575,  -839,  -829,   638,  -828,   371,   700,   701,   407,  -826,
    -542,   398,  1643,   197,  1232,   650,   705,  -824,  1472,   349,
    -867,  -544,   601,   602,  1360,   694,   483,  1774,  1056,  -866,
     350,  1644,  -809,   656,   657,   658,   660,   661,   662,   663,
     664,   665,   666,   667,   668,   669,   670,   671,   672,   673,
     674,   675,   676,   677,   678,   679,   680,   681,   682,   683,
     706,   685,   351,   686,   686,   689,   870,  1618,  -810,  1283,
    1249,  1291,   318,  1457,  1787,   707,   708,   709,   710,   711,
     712,   713,   714,   715,   716,   717,   718,   719,   355,   863,
     356,   108,   197,   686,   729,   228,   650,   650,   686,   733,
     221,  -660,   884,   707,   886,   226,   737,   227,  1086,  1361,
    -655,  1087,  1775,  1104,   121,   745,   624,   747,   765,  1788,
     484,   373,   782,  1246,   103,   650,   696,  1650,   115,  -834,
    1127,  -826,   458,   766,  1153,   767,  1671,  1404,   247,  -824,
     482,   317,  -867,   912,   613,   614,  1651,   910,   484,  1652,
    1256,  -866,   727,  1258,  -809,   615,  1780,   376,   352,   752,
    1794,   372,   823,   823,   397,   815,   396,   562,   205,   205,
     400,   770,    36,   696,   924,  1038,   369,  1039,   352,   826,
    1112,   783,   352,   352,   754,   820,   920,   760,   -94,   457,
    -810,  1713,  1128,    48,   864,  1156,  1157,   867,   358,   703,
    -546,   388,   -94,   103,  1789,   403,   902,   352,  1476,   865,
     839,   840,  1381,   815,  1382,  1375,  1156,  1157,   884,   886,
     412,  1673,   816,   383,   397,   953,   886,   488,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     413,  1781,   985,   414,  -797,  1795,   167,   508,  1415,    84,
     475,   476,    86,    87,  1033,    88,   168,    90,  -797,   358,
     377,   445,   892,   475,   476,   596,   378,   361,   362,   358,
     397,   493,   415,   446,   903,   596,   613,   614,   473,   474,
    -800,   932,   933,  1159,  -656,   416,   954,   615,  -798,  1358,
     417,  1271,  1234,   358,  -800,   150,   475,   476,  1448,   390,
     448,   934,  -798,  1262,  1304,  1235,  -837,   358,   911,   935,
     936,   937,    36,   393,  1272,  1370,   449,   207,   209,  -837,
     450,   938,   108,  1475,  1236,   451,   108,   481,   361,   362,
     527,  1696,  1293,    48,  1498,  -831,   923,   358,   361,   362,
    -543,  1765,  -654,   596,   475,   476,    36,   485,   197,   358,
    1697,  1763,   205,  1698,   365,   359,  1779,   624,  1600,    36,
    1601,   197,   361,   362,   459,   688,  1776,    48,   939,   490,
    1499,   956,   958,   959,   641,  1325,   361,   362,  1138,  1139,
      48,   940,   358,   640,   208,   208,   492,   238,   596,  1220,
    1221,   385,    86,    87,   728,    88,   168,    90,   962,   732,
     446,   629,  1131,   398,   613,   614,   361,   362,  -835,  1350,
     941,   546,   498,  1622,   823,   615,   823,   360,   361,   362,
     823,   823,  1043,   482,   720,   973,    86,    87,   501,    88,
     168,    90,   442,   443,   444,  1473,   445,   720,   502,    86,
      87,  -939,    88,   168,    90,  -652,   694,  1167,   446,  1390,
     597,   361,   362,   510,  1171,   509,  -939,   721,   518,   103,
     385,  1372,   398,  1411,  1412,  1399,  1028,  1029,  1030,  1061,
     756,  1111,   103,   531,   108,   538,  -939,   398,  -966,  -939,
     541,   205,  1031,  1070,   642,  1155,  1156,  1157,   317,   121,
     205,   352,   385,   542,  -939,  1614,  1615,   205,  1298,  1156,
    1157,   499,   548,   115,   205,   114,   116,   559,   507,  1091,
     570,   118,   568,   119,   569,   612,  1754,  1755,  1756,  1099,
    1769,  1770,  1100,   581,  1101,   582,   150,   616,   650,   617,
     150,  1743,   626,  1252,   627,   121,   554,   556,   546,   352,
     698,   352,   352,   352,   352,  1694,  1695,  1072,   628,   115,
    1743,  1478,    36,   392,   394,   395,  1690,  1691,  1764,   727,
    1484,   760,  1674,   630,   722,  1461,  1135,   639,   208,  -114,
    1489,   228,    53,    48,  1173,   652,   221,  1179,   592,   738,
    1276,   226,   598,   227,   121,   546,   988,   991,  1141,   740,
     613,   614,   591,   742,   146,   743,   749,   750,   115,   757,
     768,   615,    36,   520,   775,   121,   772,  1142,   537,   592,
     108,   598,   592,   598,   598,  1227,   789,   624,   788,   115,
     809,  1421,   811,    48,   813,   829,   833,   205,   760,   806,
     565,  1316,    86,    87,   624,    88,   168,    90,  1321,   573,
     834,   578,   837,  1628,   838,   843,   585,   823,   845,   825,
     642,   848,    36,   595,   854,   859,   860,   862,  -676,  1228,
     652,   613,   614,   868,   869,   871,   872,   875,  1709,   879,
     880,    36,   615,    48,  1229,   888,   851,   853,   150,   890,
     405,   893,    86,    87,   894,    88,   168,    90,   121,    36,
     121,   899,    48,   896,   905,   906,   917,   208,  1435,   908,
      36,   909,   115,  1332,   115,  1254,   208,   927,   579,   925,
      48,   114,   116,   208,  1422,   928,   929,   118,   650,   119,
     208,    48,   901,  -658,   955,   965,    36,  1423,  1424,   650,
    1229,   975,    86,    87,   977,    88,   168,    90,  1387,   981,
     983,   324,   352,  1753,   982,   167,    13,    48,    84,  1425,
     984,    86,    87,   986,    88,  1426,    90,   999,  1000,  1001,
    1436,   238,  1047,   167,  1003,  1004,    84,  1036,  1286,    86,
      87,  1300,    88,   168,    90,  1668,   585,  1669,  1049,  1059,
      86,    87,  1068,    88,   168,    90,  1675,  1055,   205,  1289,
     121,  1066,  1060,    60,    61,    62,   163,   164,   408,  1077,
     146,  1678,  1069,   365,   115,  1073,    86,    87,  1333,    88,
     168,    90,  1075,  1334,   150,    60,    61,    62,   163,  1335,
     408,  1336,  1094,  1103,  1106,   942,    36,   948,  1109,  1114,
    -838,   624,  1712,  1125,   624,  1115,  1126,   366,  1130,  1129,
    1150,  1152,  1132,   208,  1449,   411,   205,    48,  1144,   108,
    1161,  1355,  1146,  1165,  1149,  1356,  1166,  1357,  1337,  1338,
     409,  1339,  1031,   971,   108,  1364,  1750,  1169,  1170,  1116,
    1117,  1118,    36,  1212,  1217,  1371,   650,  1223,  1226,  1242,
     121,   913,   409,  1250,  1243,  1251,   205,  1257,   205,  1255,
    1465,  1259,  1261,    48,   115,   108,  1263,  1273,    36,  1264,
    1266,  1267,  1040,   642,  1589,  1274,    86,    87,  1590,    88,
     168,    90,  1275,   938,  1285,  1279,   205,    36,  1287,    48,
    1288,  1290,  1295,  1305,  1598,  1299,  1309,  1310,  1313,  1052,
    1307,  1314,  1778,    36,  1436,  1315,  1319,   891,    48,  1785,
    1317,  1320,  1324,  1352,   108,    36,   535,  1326,   536,  1327,
    1351,  1344,    86,    87,    48,    88,   168,    90,  1362,   546,
    1344,  1349,   331,   332,  1492,   108,    48,  1363,  1365,  1377,
    1349,   722,   205,   757,   817,   818,  1367,  1460,    86,    87,
     650,    88,   168,    90,  1369,  1384,  1379,   205,   205,  1373,
    1494,  1380,  1378,  1383,   333,   922,   624,    86,    87,  1385,
      88,   168,    90,   540,   208,  1386,   901,   352,  1389,  1391,
     333,   612,  1392,    86,    87,  1396,    88,   168,    90,  1119,
    1119,   942,   819,  1393,  1408,    86,    87,  1419,    88,   168,
      90,  1397,  1432,  1452,  1458,   949,  1447,   950,  1459,  1332,
     757,  1462,  1474,  1482,  1467,  1468,   108,    53,   108,  1487,
     108,  1470,   121,   150,  1493,    60,    61,    62,   163,   164,
     408,  1485,   208,  1501,  1502,   969,   115,  1595,   150,  1596,
    1163,  1602,  1608,  1610,  1609,  1611,   459,  1653,  1612,  1613,
    1621,  1623,    13,  1632,  1441,   645,  1661,   546,   324,  1633,
     546,  1659,  1663,    36,  1687,  1344,  1662,  1686,  1670,   150,
    1692,  1344,   208,  1344,   208,  1349,  1688,   205,   205,  1700,
    1701,  1349,  1702,  1349,    48,  1344,   624,   121,  1707,   806,
    1708,  1048,   409,  1715,  1716,  1349,   121,  1625,  1482,  -336,
    1719,   115,   208,  1718,  1722,  1648,   585,  1058,   202,   202,
     115,  1723,   218,   612,  1333,  1726,  1733,  1729,   150,  1334,
     108,    60,    61,    62,   163,  1335,   408,  1336,  1731,  1732,
      60,    61,    62,   163,   164,   408,   218,   167,  1738,   150,
      84,    85,   318,    86,    87,  1745,    88,   168,    90,  1751,
    1748,    33,    34,    35,  1752,    36,  1762,  1766,   208,  1656,
    1603,  1760,  1767,   198,  1337,  1338,  1777,  1339,  1782,  1344,
    1783,  1790,  1791,   208,   208,   121,    48,  1796,  1799,  1349,
    1797,   121,  1705,   942,  1736,  1800,   121,   942,   409,   115,
    1332,  1216,  1747,  1664,   781,   115,  1469,   409,   108,   702,
     115,   205,   699,  1105,  1332,  1761,   697,  1065,   411,  1398,
     108,  1292,    74,    75,    76,    77,    78,  1627,  1759,   827,
     150,  1619,   150,   200,   150,  1500,   969,  1148,  1646,    82,
      83,  1439,   819,    13,  1784,    86,    87,  1641,    88,   168,
      90,   612,  1772,    92,  1617,  1658,   205,    13,    60,    61,
      62,    63,    64,   408,  1237,  1420,  1172,    97,  1121,    70,
     452,   205,   205,  1277,   439,   440,   441,   442,   443,   444,
    1278,   445,  1133,   873,   874,  1085,  1328,   587,   989,  1410,
    1724,   651,   882,   446,  1219,  1211,  1164,     0,     0,     0,
       0,     0,   202,   208,   208,  1333,     0,   454,     0,     0,
    1334,     0,    60,    61,    62,   163,  1335,   408,  1336,  1333,
       0,     0,     0,     0,  1334,   409,    60,    61,    62,   163,
    1335,   408,  1336,     0,   150,   121,     0,     0,     0,     0,
     942,     0,   942,     0,     0,     0,   203,   203,   205,   115,
       0,   218,     0,   218,     0,  1337,  1338,     0,  1339,     0,
    1253,     0,     0,     0,     0,    36,     0,   849,   850,  1337,
    1338,     0,  1339,     0,     0,   121,   121,  1792,     0,   409,
       0,     0,   121,     0,     0,  1798,    48,  1471,     0,   115,
     115,  1801,   108,   409,  1802,     0,   115,     0,     0,     0,
     317,  1477,     0,     0,     0,  1284,  1437,     0,   218,     0,
       0,     0,   150,  1025,  1026,  1027,  1028,  1029,  1030,   121,
     585,   969,     0,     0,   150,     0,  1737,   208,     0,     0,
       0,   202,  1031,   115,     0,     0,     0,   612,     0,     0,
     202,     0,   624,     0,     0,    86,    87,   202,    88,   168,
      90,   942,     0,     0,   202,   645,   645,   108,  1332,     0,
       0,   624,   108,     0,     0,   218,   108,     0,     0,   624,
       0,     0,   208,     0,     0,     0,     0,     0,     0,     0,
     352,     0,     0,   546,     0,   121,   317,   208,   208,     0,
     218,     0,   121,   218,     0,     0,  1584,   585,     0,   115,
       0,    13,     0,  1591,     0,     0,   115,     0,   612,     0,
     317,     0,   317,     0,     0,     0,     0,     0,   317,     0,
       0,     0,     0,    36,     0,     0,  1064,     0,     0,     0,
     203,     0,     0,     0,     0,     0,     0,   218,     0,     0,
       0,   942,  1074,     0,    48,   108,   108,   108,     0,     0,
       0,   108,     0,     0,     0,  1088,   108,     0,     0,     0,
       0,     0,     0,  1333,   208,     0,  1422,     0,  1334,     0,
      60,    61,    62,   163,  1335,   408,  1336,   202,     0,  1423,
    1424,     0,     0,     0,  1108,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   150,   167,     0,     0,
      84,    85,     0,    86,    87,     0,    88,  1426,    90,     0,
       0,     0,     0,  1337,  1338,     0,  1339,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
     218,     0,     0,   799,     0,     0,     0,   409,     0,     0,
       0,     0,     0,     0,     0,  1620,  1160,     0,     0,  1162,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   203,
       0,   150,   799,     0,     0,   546,   150,     0,   203,     0,
     150,     0,     0,     0,     0,   203,     0,     0,     0,     0,
       0,     0,   203,     0,     0,     0,   317,     0,     0,     0,
     942,     0,     0,     0,     0,   108,     0,     0,     0,     0,
       0,     0,     0,  1679,     0,     0,     0,     0,   218,   218,
    1584,  1584,     0,     0,  1591,  1591,     0,   218,     0,     0,
     259,     0,    60,    61,    62,    63,    64,   408,   352,     0,
       0,     0,     0,    70,   452,   108,   108,     0,   202,     0,
       0,     0,   108,     0,  1245,     0,   882,     0,   261,   150,
     150,   150,     0,     0,     0,   150,     0,     0,     0,     0,
     150,     0,     0,     0,     0,     0,     0,     0,     0,   453,
      36,   454,     0,  1265,     0,     0,  1268,     0,     0,   108,
       0,     0,     0,     0,   455,  1735,   456,     0,     0,   409,
       0,    48,     0,     0,     0,     0,   202,     0,     0,   539,
       0,     0,     0,  1749,     0,   203,     0,   488,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
       0,     0,     0,     0,     0,     0,   533,   534,  1306,     0,
       0,     0,  1088,     0,     0,     0,   202,     0,   202,     0,
       0,     0,     0,     0,   167,   108,     0,    84,   311,     0,
      86,    87,   108,    88,   168,    90,     0,     0,   473,   474,
       0,     0,     0,     0,     0,     0,   202,   799,   315,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   316,     0,
     218,   218,   799,   799,   799,   799,   799,  1329,  1330,     0,
     799,     0,     0,     0,     0,     0,     0,     0,     0,   150,
       0,   218,     0,   488,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,     0,     0,     0,     0,
       0,     0,   202,     0,   475,   476,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   218,     0,   202,   202,   150,
     150,     0,     0,     0,     0,     0,   150,     0,     0,     0,
       0,   218,   218,     0,   473,   474,     0,     0,   204,   204,
       0,   218,   220,     0,     0,     0,   203,   218,     0,     0,
       0,     0,     0,     0,  1400,     0,  1401,     0,     0,     0,
     218,   751,     0,   150,     0,     0,     0,     0,   799,     0,
       0,   218,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1005,  1006,  1007,     0,     0,   218,
       0,  1442,     0,   218,     0,     0,     0,     0,     0,     0,
     475,   476,     0,  1008,   203,     0,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,     0,   150,
       0,     0,     0,     0,     0,     0,   150,   202,   202,     0,
       0,  1031,     0,     0,   203,     0,   203,     0,     0,     0,
       0,   218,     0,     0,   218,     0,   218,   835,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   799,     0,   218,   203,     0,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,     0,     0,     0,     0,     0,   473,
     474,     0,     0,     0,     0,     0,     0,     0,     0,   799,
       0,     0,   204,     0,     0,     0,     0,     0,     0,     0,
     203,  -967,  -967,  -967,  -967,  -967,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,     0,   203,   203,     0,  1637,   218,
       0,   218,     0,     0,     0,     0,     0,  1031,     0,     0,
       0,   202,  1177,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   475,   476,     0,   218,     0,
       0,   218,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   218,   445,     0,     0,     0,   202,     0,     0,     0,
       0,     0,     0,     0,   446,     0,     0,     0,   259,     0,
       0,   202,   202,     0,   799,     0,     0,     0,     0,     0,
       0,     0,     0,   218,     0,     0,     0,   218,     0,     0,
     799,     0,   799,     0,     0,     0,   261,     0,     0,  1660,
       0,   204,     0,     0,     0,     0,     0,     0,   799,     0,
     204,     0,     0,     0,     0,   203,   203,   204,    36,     0,
       0,     0,     0,     0,   204,     0,     0,     0,   418,   419,
     420,     0,     0,     0,     0,   204,     0,     0,     0,    48,
       0,     0,   218,   218,   259,   218,     0,   421,   202,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   261,   445,   533,   534,     0,     0,     0,     0,
       0,  1720,     0,     0,     0,   446,     0,     0,     0,     0,
       0,     0,   167,     0,    36,    84,   311,     0,    86,    87,
       0,    88,   168,    90,     0,   987,     0,   220,     0,     0,
       0,     0,     0,     0,     0,    48,   315,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   316,     0,     0,   218,
       0,   218,     0,     0,     0,     0,   799,   218,     0,   203,
     799,     0,   799,     0,     0,   799,     0,   204,     0,     0,
     533,   534,     0,     0,   882,   218,   218,     0,     0,   218,
       0,     0,     0,     0,     0,     0,   218,     0,   167,   882,
       0,    84,   311,     0,    86,    87,     0,    88,   168,    90,
       0,  1308,     0,     0,   203,     0,     0,     0,     0,     0,
       0,     0,   315,     0,     0,     0,     0,     0,     0,   203,
     203,     0,   316,   802,     0,     0,     0,     0,   218,     0,
       0,     0,  1230,     0,     0,     0,   320,     0,     0,     0,
       0,     0,     0,     0,   799,     0,     0,     0,     0,     0,
       0,     0,   802,   218,   218,     0,     0,     0,     0,     0,
       0,   218,     0,   218,  -967,  -967,  -967,  -967,  -967,   437,
     438,   439,   440,   441,   442,   443,   444,     0,   445,   418,
     419,   420,     0,     0,     0,   218,     0,   218,     0,     0,
     446,     0,     0,   218,     0,     0,   203,     0,   421,     0,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,     0,   445,   418,   419,   420,   204,   799,
     799,     0,     0,     0,     0,   799,   446,   218,     0,     0,
       0,     0,     0,   218,   421,   218,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,     0,
     445,     0,   418,   419,   420,     0,     0,     0,     0,     0,
       0,     0,   446,     0,     0,     0,   204,     0,     0,     0,
       0,   421,     0,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,     0,   445,     0,     0,
       0,     0,     0,     0,     0,     0,   204,     0,   204,   446,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   320,     0,   320,   218,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   204,   802,     0,     0,
     855,   218,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   802,   802,   802,   802,   802,     0,   218,     0,
     802,     0,     0,     0,   799,     0,     0,     0,     0,     0,
       0,  1035,     0,     0,     0,   799,     0,     0,   320,     0,
       0,   799,     0,     0,     0,   799,   887,     0,     0,     0,
       0,     0,   204,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1054,   218,   204,   204,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1054,     0,     0,     0,     0,     0,     0,     0,
       0,   204,     0,   926,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   799,     0,     0,     0,     0,
       0,     0,     0,     0,   218,     0,     0,     0,   802,     0,
     320,  1095,     0,   320,     0,     0,     0,     0,     0,     0,
     218,     0,     0,     0,   418,   419,   420,     0,     0,   218,
       0,     0,     0,   220,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   421,   218,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,     0,   445,
       0,     0,     0,     0,     0,     0,     0,   204,   204,     0,
       0,   446,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   488,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,     0,     0,     0,     0,
       0,   802,     0,   204,     0,     0,   802,   802,   802,   802,
     802,   802,   802,   802,   802,   802,   802,   802,   802,   802,
     802,   802,   802,   802,   802,   802,   802,   802,   802,   802,
     802,   802,   802,   802,   473,   474,     0,     0,     0,   320,
     784,     0,     0,   800,     0,     0,     0,  1008,     0,   802,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,   800,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1031,     0,     0,     0,     0,
       0,   204,     0,     0,     0,   930,     0,     0,     0,     0,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   320,   320,
       0,     0,     0,     0,     0,     0,     0,   320,     0,     0,
       0,   204,     0,     0,     0,     0,   204,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   204,   204,     0,   802,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     802,     0,   802,   418,   419,   420,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   802,     0,
       0,     0,   421,     0,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,     0,   445,   418,
     419,   420,     0,     0,     0,  1331,     0,     0,   204,     0,
     446,     0,     0,     0,     0,   259,     0,     0,   421,     0,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   261,   445,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   446,   800,     0,     0,
       0,     0,     0,     0,     0,    36,     0,     0,     0,     0,
     320,   320,   800,   800,   800,   800,   800,     0,     0,     0,
     800,     0,     0,     0,     0,     0,    48,     0,     0,   206,
     206,     0,     0,   224,     0,     0,   802,   204,     0,     0,
     802,     0,   802,     0,     0,   802,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1418,     0,     0,  1431,
       0,   533,   534,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1046,     0,     0,     0,     0,   167,
       0,   320,    84,   311,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,     0,     0,     0,   320,     0,     0,
       0,     0,     0,   315,     0,     0,     0,     0,   204,     0,
     320,     0,     0,   316,     0,     0,     0,     0,   800,     0,
    1102,   804,     0,     0,   802,     0,     0,     0,     0,     0,
     418,   419,   420,  1490,  1491,     0,     0,     0,     0,   320,
       0,     0,     0,  1431,     0,     0,     0,     0,     0,   421,
     828,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,     0,   445,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   446,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   320,     0,     0,   320,     0,   784,     0,     0,   802,
     802,     0,     0,   206,     0,   802,     0,  1635,     0,     0,
       0,   800,     0,     0,     0,  1431,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,     0,   418,   419,   420,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   800,
       0,     0,     0,     0,   421,     0,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   320,
     445,   320,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1113,   446,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    28,   320,     0,
       0,   320,     0,     0,     0,    33,    34,    35,    36,     0,
     197,     0,   206,     0,     0,     0,     0,   198,     0,     0,
       0,   206,     0,     0,     0,   970,     0,     0,   206,    48,
       0,     0,     0,     0,   802,   206,     0,     0,     0,     0,
     992,   993,   994,   995,   800,   802,   224,     0,  1002,     0,
     199,   802,     0,   320,     0,   802,     0,   320,     0,     0,
     800,     0,   800,     0,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   200,   800,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,     0,     0,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,  1137,     0,     0,     0,
       0,    97,   320,   320,     0,   802,   201,     0,   224,   564,
       0,   103,     0,     0,  1746,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1418,   418,   419,   420,     0,     0,  1092,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   206,     0,
     421,     0,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,     0,   445,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   446,   320,
       0,   320,     0,     0,     0,     0,   800,     0,     0,     0,
     800,     0,   800,     0,   803,   800,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   320,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   320,     0,     0,     0,
       0,     0,     0,   803,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1180,  1183,  1184,  1185,  1187,  1188,
    1189,  1190,  1191,  1192,  1193,  1194,  1195,  1196,  1197,  1198,
    1199,  1200,  1201,  1202,  1203,  1204,  1205,  1206,  1207,  1208,
    1209,  1210,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   800,     0,     0,  1218,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   320,     0,     0,     0,     0,     0,     0,     0,   206,
       0,     0,  1454,     0,     0,   418,   419,   420,     0,     0,
       0,     0,     0,     0,     0,   320,     0,   320,     0,     0,
       0,     0,     0,   320,   421,     0,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,     0,
     445,     0,     0,     0,     0,     0,     0,   206,     0,   800,
     800,     0,   446,     0,     0,   800,     0,     0,     0,     0,
       0,     0,     0,   320,     0,     0,     0,     0,     0,     0,
       0,     0,  1296,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   206,  1311,   206,
    1312,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1322,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   206,   803,     0,
    1005,  1006,  1007,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   803,   803,   803,   803,   803,     0,  1008,
    1323,   803,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,   320,     0,   259,     0,     0,     0,
       0,     0,     0,   206,     0,     0,  1455,  1031,     0,     0,
       0,   320,     0,     0,     0,     0,     0,     0,   206,   206,
       0,     0,     0,     0,   261,     0,     0,     0,  1680,     0,
       0,     0,     0,     0,   800,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,   800,    36,     0,     0,     0,
       0,   800,     0,     0,     0,   800,     0,     0,     0,     0,
       0,     0,     0,     0,  1403,     0,     0,    48,  1405,   803,
    1406,     0,     0,  1407,     0,  -383,   320,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   163,   164,   408,     0,
       0,     0,     0,     0,   224,     0,     0,     0,     0,     0,
       0,     0,   533,   534,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   800,     0,     0,     0,     0,
     167,     0,     0,    84,   311,     0,    86,    87,     0,    88,
     168,    90,     0,     0,     0,     0,     0,     0,   206,   206,
       0,     0,     0,     0,   315,     0,     0,     0,     0,   320,
     409,     0,  1486,     0,   316,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   320,     0,     0,     0,     0,     0,
       0,     0,   803,     0,   224,     0,     0,   803,   803,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   803,   803,   803,   803,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     803,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1629,  1630,     0,
       0,     0,     0,  1634,   418,   419,   420,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   206,   421,  1301,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,     0,   445,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   446,   224,     0,     0,     0,     0,   206,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   206,   206,     0,   803,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1005,  1006,
    1007,   803,     0,   803,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1008,     0,   803,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,  1689,     0,     0,  1031,     0,    10,     0,   206,
       0,     0,     0,  1699,     0,     0,     0,     0,     0,  1704,
       0,   380,    12,  1706,     0,     0,     0,     0,     0,     0,
     704,     0,     0,  1302,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,  1739,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,   803,   224,     0,
       0,   803,    53,   803,     0,     0,   803,     0,     0,     0,
      60,    61,    62,   163,   164,   165,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   166,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,   224,
       0,    94,   418,   419,   420,     0,    97,    98,    99,     0,
       0,   100,     0,     0,     0,   803,   103,   104,     0,   105,
     106,   421,     0,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,     0,   445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   446,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
     803,   803,     0,     0,     0,     0,   803,     0,     0,     0,
       0,     0,     0,     0,     0,  1640,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,    54,    55,
      56,   447,    57,    58,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,    81,    82,    83,    84,    85,     0,    86,    87,
       0,    88,    89,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,    95,     0,    96,
       0,    97,    98,    99,     0,   803,   100,     0,   101,   102,
    1062,   103,   104,     0,   105,   106,   803,     0,     0,     0,
       0,     0,   803,     0,     0,     0,   803,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,  1721,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,   803,    17,    18,    19,
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
     101,   102,  1231,   103,   104,     0,   105,   106,     5,     6,
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
     631,   103,   104,     0,   105,   106,     5,     6,     7,     8,
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
     100,     0,   101,   102,  1034,   103,   104,     0,   105,   106,
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
      98,    99,     0,     0,   100,     0,   101,   102,  1076,   103,
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
     101,   102,  1143,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,  1145,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
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
      44,     0,    45,     0,    46,     0,    47,  1297,     0,    48,
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
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,     0,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,  1409,   103,   104,     0,   105,   106,
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
      98,    99,     0,     0,   100,     0,   101,   102,  1631,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,  1676,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,     0,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,     0,   103,   104,     0,   105,   106,     5,     6,
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
       0,     0,   100,     0,   101,   102,  1710,   103,   104,     0,
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
    1711,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,  1714,    46,     0,
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
      98,    99,     0,     0,   100,     0,   101,   102,  1730,   103,
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
     101,   102,  1786,   103,   104,     0,   105,   106,     5,     6,
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
       0,     0,   100,     0,   101,   102,  1793,   103,   104,     0,
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
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,   516,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    11,    12,     0,   769,     0,     0,
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
       0,   972,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    11,    12,     0,  1481,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,    11,    12,     0,  1624,
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
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    12,     0,     0,     0,     0,
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
      98,    99,     0,     0,   169,     0,   325,     0,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,     0,   445,   646,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   446,    14,
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
      86,    87,     0,    88,   168,    90,     0,   647,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   169,     0,
       0,     0,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,     0,     0,     0,     0,     0,     0,
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
       0,     0,   169,     0,     0,   764,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,     0,     0,  1089,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1031,    14,    15,     0,
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
       0,    88,   168,    90,     0,  1090,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   169,     0,     0,     0,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     380,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     100,     0,   418,   419,   420,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   421,     0,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,     0,   445,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,   446,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,   180,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   163,   164,   165,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   166,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,   530,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   169,     0,     0,     0,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1031,     0,    14,
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
     418,   419,   420,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   421,
       0,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,     0,   445,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   446,     0,    16,
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
       0,     0,     0,    92,     0,     0,    93,     0,     0,   532,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   169,     0,   245,   419,   420,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   421,     0,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,     0,   445,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   446,     0,    16,     0,    17,    18,    19,    20,    21,
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
       0,    97,    98,    99,     0,     0,   169,     0,   248,     0,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     380,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     100,     0,   418,   419,   420,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   421,     0,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,     0,   445,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,   446,
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
       0,   549,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   169,   514,     0,     0,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   659,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,     0,   445,     0,
       0,   704,     0,     0,     0,     0,     0,     0,     0,     0,
     446,     0,     0,    14,    15,     0,     0,     0,     0,    16,
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
       0,     0,    10,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,     0,     0,     0,     0,   744,     0,     0,     0,     0,
       0,     0,     0,     0,  1031,     0,     0,    14,    15,     0,
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
       9,     0,     0,     0,     0,     0,    10,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,     0,   445,     0,     0,     0,   746,
       0,     0,     0,     0,     0,     0,     0,   446,     0,     0,
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
      10,  -967,  -967,  -967,  -967,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,     0,   445,
       0,     0,     0,  1134,     0,     0,     0,     0,     0,     0,
       0,   446,     0,     0,     0,    14,    15,     0,     0,     0,
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
      98,    99,     0,     0,   169,     0,   418,   419,   420,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   421,     0,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
       0,   445,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   446,     0,    16,     0,    17,    18,    19,
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
       0,     0,    93,     0,     0,   553,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   169,     0,
     418,   419,   420,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   421,
       0,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,     0,   445,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   446,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,   593,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   163,   164,   165,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   166,
      73,     0,    74,    75,    76,    77,    78,   250,   251,     0,
     252,   253,     0,    80,   254,   255,   256,   257,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
       0,   258,     0,    92,     0,     0,    93,     0,   736,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   169,     0,     0,     0,     0,   103,   104,   260,
     105,   106,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   262,   263,   264,   265,   266,   267,   268,
       0,     0,     0,    36,     0,   197,     0,     0,     0,     0,
       0,     0,     0,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,    48,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,     0,     0,
       0,   692,   304,   305,   306,     0,     0,     0,   307,   543,
     544,   250,   251,     0,   252,   253,     0,     0,   254,   255,
     256,   257,     0,     0,     0,     0,     0,   545,     0,     0,
       0,     0,     0,    86,    87,   258,    88,   168,    90,   312,
       0,   313,     0,     0,   314,     0,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,   260,   693,     0,   103,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1031,   262,   263,   264,
     265,   266,   267,   268,     0,     0,     0,    36,     0,   197,
       0,     0,     0,     0,     0,     0,     0,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,    48,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,     0,     0,     0,   303,   304,   305,   306,     0,
       0,     0,   307,   543,   544,     0,     0,     0,     0,     0,
     250,   251,     0,   252,   253,     0,     0,   254,   255,   256,
     257,   545,     0,     0,     0,     0,     0,    86,    87,     0,
      88,   168,    90,   312,   258,   313,   259,     0,   314,  -967,
    -967,  -967,  -967,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1028,  1029,  1030,     0,     0,   693,     0,
     103,     0,   260,     0,   261,     0,     0,     0,     0,  1031,
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
       0,     0,     0,     0,   316,     0,     0,     0,  1604,     0,
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
       0,     0,     0,   316,     0,     0,     0,  1672,     0,     0,
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
       0,   315,  1413,     0,     0,     0,     0,     0,     0,     0,
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
     313,     0,     0,   314,  1505,  1506,  1507,  1508,  1509,     0,
     315,  1510,  1511,  1512,  1513,     0,     0,     0,     0,     0,
     316,     0,     0,     0,     0,     0,     0,     0,  1514,  1515,
       0,   421,     0,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,  1516,   445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   446,
    1517,  1518,  1519,  1520,  1521,  1522,  1523,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1524,  1525,  1526,  1527,  1528,  1529,  1530,  1531,  1532,  1533,
    1534,    48,  1535,  1536,  1537,  1538,  1539,  1540,  1541,  1542,
    1543,  1544,  1545,  1546,  1547,  1548,  1549,  1550,  1551,  1552,
    1553,  1554,  1555,  1556,  1557,  1558,  1559,  1560,  1561,  1562,
    1563,  1564,     0,     0,     0,  1565,  1566,     0,  1567,  1568,
    1569,  1570,  1571,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1572,  1573,  1574,     0,     0,     0,
      86,    87,     0,    88,   168,    90,  1575,     0,  1576,  1577,
       0,  1578,   418,   419,   420,     0,     0,     0,  1579,  1580,
       0,  1581,     0,  1582,  1583,     0,     0,     0,     0,     0,
       0,   421,     0,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,     0,   445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   250,   251,   446,
     252,   253,  1006,  1007,   254,   255,   256,   257,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1008,   258,     0,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,     0,     0,     0,     0,   260,
       0,     0,     0,     0,     0,     0,     0,     0,  1031,     0,
       0,     0,     0,   262,   263,   264,   265,   266,   267,   268,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,    48,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,     0,     0,
     761,   303,   304,   305,   306,     0,     0,     0,   307,   543,
     544,   250,   251,     0,   252,   253,     0,     0,   254,   255,
     256,   257,     0,     0,     0,     0,     0,   545,     0,     0,
       0,     0,     0,    86,    87,   258,    88,   168,    90,   312,
       0,   313,     0,     0,   314,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   260,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   262,   263,   264,
     265,   266,   267,   268,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,    48,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,     0,     0,     0,  1178,   304,   305,   306,     0,
       0,     0,   307,   543,   544,   250,   251,     0,   252,   253,
       0,     0,   254,   255,   256,   257,     0,     0,     0,     0,
       0,   545,     0,     0,     0,     0,     0,    86,    87,   258,
      88,   168,    90,   312,     0,   313,     0,     0,   314,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   260,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   262,   263,   264,   265,   266,   267,   268,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,    48,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,     0,     0,     0,     0,
     304,   305,   306,  1186,     0,     0,   307,   543,   544,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   790,   791,     0,     0,   545,     0,   792,     0,   793,
       0,    86,    87,     0,    88,   168,    90,   312,     0,   313,
       0,   794,   314,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,     0,     0,     0,     0,     0,   418,   419,
     420,   198,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,   421,     0,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   966,   445,     0,     0,     0,     0,   795,     0,
      74,    75,    76,    77,    78,   446,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,   167,    82,    83,    84,
     796,     0,    86,    87,    28,    88,   168,    90,     0,     0,
       0,    92,    33,    34,    35,    36,     0,   197,     0,     0,
     797,     0,     0,     0,   198,    97,     0,     0,     0,     0,
     798,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   491,     0,     0,     0,     0,     0,   199,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     967,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   200,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,   790,   791,     0,    92,     0,     0,   792,     0,   793,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,   794,     0,   201,     0,     0,     0,     0,   103,    33,
      34,    35,    36,     0,     0,     0,     0,     0,   418,   419,
     420,   198,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,   421,     0,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,     0,   445,     0,     0,     0,     0,   795,     0,
      74,    75,    76,    77,    78,   446,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,   167,    82,    83,    84,
     796,     0,    86,    87,    28,    88,   168,    90,     0,     0,
       0,    92,    33,    34,    35,    36,     0,   197,     0,     0,
     797,     0,     0,     0,   198,    97,     0,     0,     0,     0,
     798,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   500,     0,     0,     0,     0,     0,   199,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     584,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   200,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,    28,     0,   921,    92,     0,     0,     0,     0,    33,
      34,    35,    36,     0,   197,     0,     0,     0,    97,     0,
       0,   198,     0,   201,     0,     0,     0,     0,   103,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   199,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,    28,    88,   168,    90,     0,     0,
       0,    92,    33,    34,    35,    36,     0,   197,     0,     0,
       0,     0,     0,     0,   198,    97,     0,     0,     0,     0,
     201,     0,     0,     0,     0,   103,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   199,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1057,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   200,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,    28,    88,   168,
      90,     0,     0,     0,    92,    33,    34,    35,    36,     0,
     197,     0,     0,     0,     0,     0,     0,   198,    97,     0,
       0,     0,     0,   201,     0,     0,     0,     0,   103,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     199,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   200,     0,     0,
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,     0,     0,     0,    92,     0,     0,
       0,   418,   419,   420,     0,     0,     0,     0,     0,     0,
       0,    97,     0,     0,     0,     0,   201,     0,     0,     0,
     421,   103,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,     0,   445,   418,   419,   420,
       0,     0,     0,     0,     0,     0,     0,     0,   446,     0,
       0,     0,     0,     0,     0,     0,   421,     0,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,     0,   445,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   446,     0,   418,   419,   420,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   907,   421,     0,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
       0,   445,  1005,  1006,  1007,     0,     0,     0,     0,     0,
       0,     0,     0,   446,     0,     0,     0,     0,     0,     0,
     951,  1008,     0,     0,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1028,  1029,  1030,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1031,
       0,  1005,  1006,  1007,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1260,
    1008,     0,     0,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,  1029,  1030,  1005,  1006,  1007,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1031,     0,
       0,     0,     0,     0,  1008,  1168,     0,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1031,  1005,  1006,  1007,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1008,     0,  1318,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,     0,     0,     0,
      33,    34,    35,    36,     0,   197,     0,     0,     0,     0,
    1031,     0,   198,     0,     0,     0,     0,     0,  1402,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   215,     0,     0,     0,     0,
       0,   216,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   200,     0,     0,     0,  1488,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,    33,    34,    35,    36,     0,   197,     0,
       0,     0,     0,     0,     0,   607,    97,     0,     0,     0,
       0,   217,     0,     0,     0,     0,   103,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   199,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   197,     0,     0,     0,     0,     0,     0,   198,    97,
       0,     0,     0,     0,   608,     0,     0,     0,     0,   103,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   200,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,     0,     0,     0,    92,     0,
       0,     0,   418,   419,   420,     0,     0,     0,     0,     0,
       0,     0,    97,     0,     0,     0,     0,   217,     0,     0,
     773,   421,   103,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,     0,   445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   446,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   418,   419,   420,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   774,   421,   904,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,     0,   445,   418,
     419,   420,     0,     0,     0,     0,     0,     0,     0,     0,
     446,     0,     0,     0,     0,     0,     0,     0,   421,     0,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   420,   445,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   446,     0,     0,     0,
     421,     0,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,  1007,   445,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   446,     0,
       0,     0,  1008,     0,     0,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,  1029,  1030,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1031,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,     0,   445,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   446,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1031,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,     0,   445,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   446,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1031
};

static const yytype_int16 yycheck[] =
{
       5,     6,   121,     8,     9,    10,    11,    12,    54,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   146,    91,    28,    29,    54,    95,    96,   373,   373,
     649,   701,     4,     4,   171,    55,   100,    42,     4,   373,
       4,    58,    32,  1079,   619,    50,   218,    52,   485,   213,
      55,   120,    57,   172,    44,  1075,   620,   738,   516,    49,
      30,   481,   482,   504,    81,    30,   880,    84,   151,   100,
      30,   869,    30,    42,    79,   331,   332,   772,   146,   477,
     559,   560,   477,   600,   445,   229,  1066,   100,   900,   779,
     510,   965,   550,   100,     9,   100,    30,     9,     9,     9,
     748,     9,    52,     4,   916,     9,    30,     9,    36,    68,
      79,     4,     9,    68,   512,     9,    47,   512,     9,     9,
       9,     4,     9,     9,    14,     4,    47,    14,    14,     9,
      81,     9,   112,     4,     9,    81,     9,     9,   169,     9,
     952,     9,     9,     9,     9,     9,    47,   230,     9,     9,
     131,   132,     9,    81,     9,    47,   169,     9,   103,    34,
     152,    68,   169,   621,   169,   509,     9,   173,    87,   187,
     201,   176,     9,     9,     0,     9,    47,    36,     9,     9,
     120,   187,    36,    99,   119,    47,   217,   127,   201,   187,
     170,   158,   127,   152,   201,   187,   201,    81,   149,   150,
     151,    47,   187,  1001,   217,    68,   152,   157,   109,     8,
     191,   187,   217,   114,   358,   116,   117,   118,   119,   120,
     121,   122,    81,   190,   131,   132,   231,    81,   187,   234,
     149,   190,   187,   188,   187,  1255,   241,   242,   109,    68,
     156,    68,   121,   114,    68,   116,   117,   118,   119,   120,
     121,   122,    68,   146,   185,    68,   190,   188,   159,   160,
      68,   162,    68,   235,   185,   189,   174,   239,   131,   132,
     174,   919,   174,   410,   189,   190,  1150,  1257,   324,   191,
     191,   191,   183,   173,  1264,   975,  1266,   977,   159,   160,
     191,   162,   189,   172,   188,   324,   188,   188,  1110,   189,
     189,   174,   189,   189,    68,  1285,   764,   166,   192,   189,
     188,   769,   183,   188,   331,   332,   333,   189,   188,   350,
     191,   189,   189,   189,   189,   189,   188,   190,   189,   189,
     494,  1129,   189,   897,   189,   404,   188,   350,   817,   818,
     459,   858,   188,   350,   349,   350,   189,  1367,   365,  1369,
     487,   356,   189,   189,   188,   401,   361,   188,   188,    36,
     187,   190,    36,   190,   187,    81,   190,   187,    81,   156,
     152,   187,   401,   190,   190,   380,    14,   190,    68,   187,
     349,   187,   190,   388,   190,   454,   455,   456,   457,    68,
      68,   173,    30,    81,  1064,   400,   460,    68,  1378,   187,
      68,    68,   189,   190,    81,   451,    68,    81,   845,    68,
     187,    49,    68,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     460,   446,   187,   448,   449,   450,   610,  1467,    68,  1130,
     166,  1146,   498,   166,    36,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   187,    52,
     187,     4,    81,   478,   479,   445,   481,   482,   483,   484,
     445,   152,   626,   488,   628,   445,   491,   445,   886,   166,
     152,   886,   166,   913,   387,   500,   379,   502,   518,    81,
     190,   187,    29,  1078,   192,   510,   451,    29,   387,   187,
      87,   190,   637,   518,   972,   520,  1618,  1315,    51,   190,
     187,    54,   190,   695,   869,   869,    48,   691,   190,    51,
    1094,   190,   477,  1097,   190,   869,    81,    81,    71,   509,
      81,   157,   559,   560,   156,    99,    30,    99,    26,    27,
     187,   523,    79,   498,   726,   811,    89,   813,    91,   564,
     921,    88,    95,    96,   509,  1044,   703,   512,   173,   637,
     190,  1673,   149,   100,   157,   103,   104,   608,    81,   458,
      68,   190,   187,   192,   166,    36,   655,   120,  1386,   172,
      48,    49,  1273,    99,  1275,  1265,   103,   104,   742,   743,
     189,  1621,   156,   608,   156,   749,   750,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     189,   166,   786,   189,   173,   166,   153,   799,  1333,   156,
     131,   132,   159,   160,   806,   162,   163,   164,   187,    81,
     184,    55,   647,   131,   132,    87,   190,   150,   151,    81,
     156,   184,   189,    67,   659,    87,  1001,  1001,    65,    66,
     173,    48,    49,   191,   152,   189,   749,  1001,   173,  1244,
     189,  1112,   158,    81,   187,     4,   131,   132,  1348,    87,
      68,    68,   187,  1103,   191,   171,   187,    81,   693,    76,
      77,    78,    79,    87,  1114,  1259,    68,    26,    27,   187,
     190,    88,   235,  1384,   190,   152,   239,   187,   150,   151,
     243,    29,  1149,   100,  1419,   187,   721,    81,   150,   151,
      68,  1757,   152,    87,   131,   132,    79,   187,    81,    81,
      48,  1751,   210,    51,   156,    87,  1772,   620,  1443,    79,
    1445,    81,   150,   151,   637,   449,  1766,   100,   135,   189,
    1420,   756,    73,    74,   196,  1213,   150,   151,    73,    74,
     100,   148,    81,   195,    26,    27,    46,   772,    87,    99,
     100,   100,   159,   160,   478,   162,   163,   164,   768,   483,
      67,   188,   946,   173,  1129,  1129,   150,   151,   187,  1226,
     177,   324,   152,  1474,   811,  1129,   813,   149,   150,   151,
     817,   818,   819,   187,   157,   777,   159,   160,   194,   162,
     163,   164,    51,    52,    53,  1379,    55,   157,     9,   159,
     160,   152,   162,   163,   164,   152,   872,   991,    67,  1287,
     149,   150,   151,   187,   998,   152,   152,   190,     8,   192,
     169,  1261,   173,   129,   130,  1303,    51,    52,    53,   854,
     190,   920,   192,   189,   387,   187,   187,   173,   152,   190,
      14,   339,    67,   868,   397,   102,   103,   104,   401,   762,
     348,   404,   201,   152,   190,   189,   190,   355,   102,   103,
     104,   210,   189,   762,   362,   857,   857,   190,   217,   894,
      14,   857,   127,   857,   127,   373,   116,   117,   118,   904,
     189,   190,   907,   188,   909,   173,   235,    14,   913,    99,
     239,  1725,   188,  1085,   188,   808,   331,   332,   451,   452,
     453,   454,   455,   456,   457,  1647,  1648,   872,   188,   808,
    1744,  1389,    79,   104,   105,   106,  1643,  1644,  1752,   884,
    1398,   886,  1623,   188,   477,  1365,   951,   193,   210,   187,
    1408,   921,   108,   100,  1000,   187,   921,  1003,   359,   187,
    1124,   921,   363,   921,   857,   498,   788,   789,   958,     9,
    1315,  1315,   149,   188,   857,   188,   188,   188,   857,   512,
      91,  1315,    79,     9,    14,   878,   189,   959,   173,   390,
     523,   392,   393,   394,   395,  1059,     9,   880,   187,   878,
     187,    29,   190,   100,   190,    81,   188,   485,   953,   542,
     339,  1175,   159,   160,   897,   162,   163,   164,  1182,   348,
     188,   350,   188,  1481,   189,   129,   355,  1044,   187,   562,
     563,   188,    79,   362,    68,    30,   130,   172,   152,  1059,
     187,  1386,  1386,   133,     9,   188,   152,    14,  1667,   185,
       9,    79,  1386,   100,  1059,     9,   589,   590,   387,   174,
     157,   188,   159,   160,     9,   162,   163,   164,   961,    79,
     963,   129,   100,    14,   194,   194,    14,   339,   125,   191,
      79,     9,   961,     4,   963,  1090,   348,   188,   350,   194,
     100,  1063,  1063,   355,   122,   188,   194,  1063,  1103,  1063,
     362,   100,   187,   152,   188,    99,    79,   135,   136,  1114,
    1115,   189,   159,   160,   189,   162,   163,   164,  1282,    88,
     152,    54,   655,  1742,   133,   153,    47,   100,   156,   157,
       9,   159,   160,   188,   162,   163,   164,   187,   152,   187,
     187,  1146,    14,   153,   152,   190,   156,   190,  1138,   159,
     160,  1156,   162,   163,   164,  1613,   485,  1615,   189,    14,
     159,   160,    14,   162,   163,   164,  1624,   190,   646,  1141,
    1063,   190,   194,   116,   117,   118,   119,   120,   121,    30,
    1063,   191,   188,   156,  1063,   189,   159,   160,   109,   162,
     163,   164,   185,   114,   523,   116,   117,   118,   119,   120,
     121,   122,   187,   187,    30,   738,    79,   740,    14,   187,
     187,  1094,  1670,    50,  1097,    14,   187,   190,     9,   187,
     133,    14,   188,   485,  1349,  1344,   704,   100,   189,   762,
     133,  1236,   189,     9,   187,  1240,   188,  1242,   159,   160,
     183,   162,    67,   776,   777,  1250,   189,   194,     9,    76,
      77,    78,    79,    81,   189,  1260,  1261,     9,   187,   133,
    1153,   187,   183,    14,   189,    81,   744,   190,   746,   188,
     191,   187,   187,   100,  1153,   808,   188,   133,    79,   190,
     190,   189,   815,   816,   157,   194,   159,   160,   161,   162,
     163,   164,     9,    88,   190,   149,   774,    79,    30,   100,
      75,   189,   188,   174,  1441,   189,    30,   188,   188,   842,
     133,   133,  1770,    79,   187,     9,   191,   646,   100,  1777,
     188,     9,   188,   190,   857,    79,   259,   189,   261,   189,
     191,  1224,   159,   160,   100,   162,   163,   164,    14,   872,
    1233,  1224,   108,   109,  1413,   878,   100,    81,   187,   189,
    1233,   884,   830,   886,   108,   109,   188,  1362,   159,   160,
    1365,   162,   163,   164,   188,   133,   187,   845,   846,   188,
    1416,   188,   190,   188,   156,   704,  1259,   159,   160,   188,
     162,   163,   164,   316,   646,     9,   187,   920,    30,   189,
     156,   869,   188,   159,   160,   189,   162,   163,   164,   932,
     933,   934,   156,   188,   190,   159,   160,   109,   162,   163,
     164,   189,   161,   157,    14,   744,   189,   746,    81,     4,
     953,   114,   133,  1395,   188,   188,   959,   108,   961,   133,
     963,   190,  1325,   762,    14,   116,   117,   118,   119,   120,
     121,   188,   704,   173,   190,   774,  1325,   189,   777,    81,
     983,    14,    14,  1458,    81,   188,  1349,    14,   187,   190,
     188,   133,    47,   189,  1343,   398,  1603,  1000,   401,   189,
    1003,    14,    14,    79,  1638,  1368,   189,     9,   190,   808,
      57,  1374,   744,  1376,   746,  1368,   191,   965,   966,    81,
     173,  1374,   187,  1376,   100,  1388,  1379,  1390,    81,  1032,
       9,   830,   183,   189,   112,  1388,  1399,  1479,  1480,    99,
      99,  1390,   774,   152,   164,    34,   845,   846,    26,    27,
    1399,    14,    30,  1001,   109,   187,   170,   188,   857,   114,
    1063,   116,   117,   118,   119,   120,   121,   122,   189,   187,
     116,   117,   118,   119,   120,   121,    54,   153,   174,   878,
     156,   157,  1598,   159,   160,    81,   162,   163,   164,   188,
     167,    76,    77,    78,     9,    79,   189,   188,   830,  1598,
    1449,    81,   188,    88,   159,   160,   190,   162,    14,  1472,
      81,    14,    81,   845,   846,  1478,   100,    14,    14,  1472,
      81,  1484,  1661,  1126,  1719,    81,  1489,  1130,   183,  1478,
       4,  1044,  1733,  1608,   537,  1484,   191,   183,  1141,   457,
    1489,  1089,   454,   914,     4,  1748,   452,   860,  1737,  1300,
    1153,  1147,   137,   138,   139,   140,   141,  1480,  1744,   566,
     959,  1470,   961,   148,   963,  1421,   965,   966,  1587,   154,
     155,  1339,   156,    47,  1776,   159,   160,  1503,   162,   163,
     164,  1129,  1764,   168,  1466,  1599,  1134,    47,   116,   117,
     118,   119,   120,   121,  1067,  1335,   999,   182,   933,   127,
     128,  1149,  1150,  1125,    48,    49,    50,    51,    52,    53,
    1126,    55,   948,   616,   617,   884,  1219,   356,   788,  1328,
    1701,   401,   625,    67,  1050,  1032,   984,    -1,    -1,    -1,
      -1,    -1,   210,   965,   966,   109,    -1,   165,    -1,    -1,
     114,    -1,   116,   117,   118,   119,   120,   121,   122,   109,
      -1,    -1,    -1,    -1,   114,   183,   116,   117,   118,   119,
     120,   121,   122,    -1,  1063,  1628,    -1,    -1,    -1,    -1,
    1273,    -1,  1275,    -1,    -1,    -1,    26,    27,  1226,  1628,
      -1,   259,    -1,   261,    -1,   159,   160,    -1,   162,    -1,
    1089,    -1,    -1,    -1,    -1,    79,    -1,    81,    82,   159,
     160,    -1,   162,    -1,    -1,  1668,  1669,  1782,    -1,   183,
      -1,    -1,  1675,    -1,    -1,  1790,   100,   191,    -1,  1668,
    1669,  1796,  1325,   183,  1799,    -1,  1675,    -1,    -1,    -1,
    1333,   191,    -1,    -1,    -1,  1134,  1339,    -1,   316,    -1,
      -1,    -1,  1141,    48,    49,    50,    51,    52,    53,  1712,
    1149,  1150,    -1,    -1,  1153,    -1,  1719,  1089,    -1,    -1,
      -1,   339,    67,  1712,    -1,    -1,    -1,  1315,    -1,    -1,
     348,    -1,  1725,    -1,    -1,   159,   160,   355,   162,   163,
     164,  1384,    -1,    -1,   362,   788,   789,  1390,     4,    -1,
      -1,  1744,  1395,    -1,    -1,   373,  1399,    -1,    -1,  1752,
      -1,    -1,  1134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1413,    -1,    -1,  1416,    -1,  1778,  1419,  1149,  1150,    -1,
     398,    -1,  1785,   401,    -1,    -1,  1429,  1226,    -1,  1778,
      -1,    47,    -1,  1436,    -1,    -1,  1785,    -1,  1386,    -1,
    1443,    -1,  1445,    -1,    -1,    -1,    -1,    -1,  1451,    -1,
      -1,    -1,    -1,    79,    -1,    -1,   859,    -1,    -1,    -1,
     210,    -1,    -1,    -1,    -1,    -1,    -1,   445,    -1,    -1,
      -1,  1474,   875,    -1,   100,  1478,  1479,  1480,    -1,    -1,
      -1,  1484,    -1,    -1,    -1,   888,  1489,    -1,    -1,    -1,
      -1,    -1,    -1,   109,  1226,    -1,   122,    -1,   114,    -1,
     116,   117,   118,   119,   120,   121,   122,   485,    -1,   135,
     136,    -1,    -1,    -1,   917,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1325,   153,    -1,    -1,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,    -1,   159,   160,    -1,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   537,
     538,    -1,    -1,   541,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   191,   979,    -1,    -1,   982,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   339,
      -1,  1390,   570,    -1,    -1,  1598,  1395,    -1,   348,    -1,
    1399,    -1,    -1,    -1,    -1,   355,    -1,    -1,    -1,    -1,
      -1,    -1,   362,    -1,    -1,    -1,  1619,    -1,    -1,    -1,
    1623,    -1,    -1,    -1,    -1,  1628,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1636,    -1,    -1,    -1,    -1,   616,   617,
    1643,  1644,    -1,    -1,  1647,  1648,    -1,   625,    -1,    -1,
      29,    -1,   116,   117,   118,   119,   120,   121,  1661,    -1,
      -1,    -1,    -1,   127,   128,  1668,  1669,    -1,   646,    -1,
      -1,    -1,  1675,    -1,  1077,    -1,  1079,    -1,    57,  1478,
    1479,  1480,    -1,    -1,    -1,  1484,    -1,    -1,    -1,    -1,
    1489,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      79,   165,    -1,  1106,    -1,    -1,  1109,    -1,    -1,  1712,
      -1,    -1,    -1,    -1,   178,  1718,   180,    -1,    -1,   183,
      -1,   100,    -1,    -1,    -1,    -1,   704,    -1,    -1,   108,
      -1,    -1,    -1,  1736,    -1,   485,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,   135,   136,  1161,    -1,
      -1,    -1,  1165,    -1,    -1,    -1,   744,    -1,   746,    -1,
      -1,    -1,    -1,    -1,   153,  1778,    -1,   156,   157,    -1,
     159,   160,  1785,   162,   163,   164,    -1,    -1,    65,    66,
      -1,    -1,    -1,    -1,    -1,    -1,   774,   775,   177,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
     788,   789,   790,   791,   792,   793,   794,  1220,  1221,    -1,
     798,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1628,
      -1,   809,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,
      -1,    -1,   830,    -1,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   843,    -1,   845,   846,  1668,
    1669,    -1,    -1,    -1,    -1,    -1,  1675,    -1,    -1,    -1,
      -1,   859,   860,    -1,    65,    66,    -1,    -1,    26,    27,
      -1,   869,    30,    -1,    -1,    -1,   646,   875,    -1,    -1,
      -1,    -1,    -1,    -1,  1307,    -1,  1309,    -1,    -1,    -1,
     888,   188,    -1,  1712,    -1,    -1,    -1,    -1,   896,    -1,
      -1,   899,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,   917,
      -1,  1344,    -1,   921,    -1,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,    29,   704,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,  1778,
      -1,    -1,    -1,    -1,    -1,    -1,  1785,   965,   966,    -1,
      -1,    67,    -1,    -1,   744,    -1,   746,    -1,    -1,    -1,
      -1,   979,    -1,    -1,   982,    -1,   984,   188,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   999,    -1,  1001,   774,    -1,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,    -1,    -1,    -1,    -1,    -1,    65,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1047,
      -1,    -1,   210,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     830,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,   845,   846,    -1,  1501,  1077,
      -1,  1079,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,  1089,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,  1106,    -1,
      -1,  1109,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,  1129,    55,    -1,    -1,    -1,  1134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    29,    -1,
      -1,  1149,  1150,    -1,  1152,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1161,    -1,    -1,    -1,  1165,    -1,    -1,
    1168,    -1,  1170,    -1,    -1,    -1,    57,    -1,    -1,  1602,
      -1,   339,    -1,    -1,    -1,    -1,    -1,    -1,  1186,    -1,
     348,    -1,    -1,    -1,    -1,   965,   966,   355,    79,    -1,
      -1,    -1,    -1,    -1,   362,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,   373,    -1,    -1,    -1,   100,
      -1,    -1,  1220,  1221,    29,  1223,    -1,    29,  1226,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    57,    55,   135,   136,    -1,    -1,    -1,    -1,
      -1,  1684,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,   153,    -1,    79,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,    -1,   166,    -1,   445,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,   177,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,  1307,
      -1,  1309,    -1,    -1,    -1,    -1,  1314,  1315,    -1,  1089,
    1318,    -1,  1320,    -1,    -1,  1323,    -1,   485,    -1,    -1,
     135,   136,    -1,    -1,  1757,  1333,  1334,    -1,    -1,  1337,
      -1,    -1,    -1,    -1,    -1,    -1,  1344,    -1,   153,  1772,
      -1,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,   166,    -1,    -1,  1134,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,  1149,
    1150,    -1,   187,   541,    -1,    -1,    -1,    -1,  1386,    -1,
      -1,    -1,   194,    -1,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1402,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   570,  1411,  1412,    -1,    -1,    -1,    -1,    -1,
      -1,  1419,    -1,  1421,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    10,
      11,    12,    -1,    -1,    -1,  1443,    -1,  1445,    -1,    -1,
      67,    -1,    -1,  1451,    -1,    -1,  1226,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    10,    11,    12,   646,  1487,
    1488,    -1,    -1,    -1,    -1,  1493,    67,  1495,    -1,    -1,
      -1,    -1,    -1,  1501,    29,  1503,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,   704,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   744,    -1,   746,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   259,    -1,   261,  1602,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   774,   775,    -1,    -1,
     191,  1619,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   790,   791,   792,   793,   794,    -1,  1636,    -1,
     798,    -1,    -1,    -1,  1642,    -1,    -1,    -1,    -1,    -1,
      -1,   809,    -1,    -1,    -1,  1653,    -1,    -1,   316,    -1,
      -1,  1659,    -1,    -1,    -1,  1663,   191,    -1,    -1,    -1,
      -1,    -1,   830,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   843,  1684,   845,   846,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   860,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   869,    -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1723,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1732,    -1,    -1,    -1,   896,    -1,
     398,   899,    -1,   401,    -1,    -1,    -1,    -1,    -1,    -1,
    1748,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1757,
      -1,    -1,    -1,   921,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,  1772,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   965,   966,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,
      -1,   999,    -1,  1001,    -1,    -1,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,    65,    66,    -1,    -1,    -1,   537,
     538,    -1,    -1,   541,    -1,    -1,    -1,    29,    -1,  1047,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,   570,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,  1089,    -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   616,   617,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   625,    -1,    -1,
      -1,  1129,    -1,    -1,    -1,    -1,  1134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1149,  1150,    -1,  1152,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1168,    -1,  1170,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1186,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    10,
      11,    12,    -1,    -1,    -1,  1223,    -1,    -1,  1226,    -1,
      67,    -1,    -1,    -1,    -1,    29,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    57,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,   775,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
     788,   789,   790,   791,   792,   793,   794,    -1,    -1,    -1,
     798,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    26,
      27,    -1,    -1,    30,    -1,    -1,  1314,  1315,    -1,    -1,
    1318,    -1,  1320,    -1,    -1,  1323,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1334,    -1,    -1,  1337,
      -1,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,   153,
      -1,   859,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,    -1,    -1,    -1,    -1,    -1,    -1,   875,    -1,    -1,
      -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,  1386,    -1,
     888,    -1,    -1,   187,    -1,    -1,    -1,    -1,   896,    -1,
     191,   541,    -1,    -1,  1402,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,  1411,  1412,    -1,    -1,    -1,    -1,   917,
      -1,    -1,    -1,  1421,    -1,    -1,    -1,    -1,    -1,    29,
     570,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   979,    -1,    -1,   982,    -1,   984,    -1,    -1,  1487,
    1488,    -1,    -1,   210,    -1,  1493,    -1,  1495,    -1,    -1,
      -1,   999,    -1,    -1,    -1,  1503,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
    1028,  1029,  1030,  1031,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1047,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,  1077,
      55,  1079,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   191,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,  1106,    -1,
      -1,  1109,    -1,    -1,    -1,    76,    77,    78,    79,    -1,
      81,    -1,   339,    -1,    -1,    -1,    -1,    88,    -1,    -1,
      -1,   348,    -1,    -1,    -1,   775,    -1,    -1,   355,   100,
      -1,    -1,    -1,    -1,  1642,   362,    -1,    -1,    -1,    -1,
     790,   791,   792,   793,  1152,  1653,   373,    -1,   798,    -1,
     121,  1659,    -1,  1161,    -1,  1663,    -1,  1165,    -1,    -1,
    1168,    -1,  1170,    -1,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,  1186,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,
      -1,   182,  1220,  1221,    -1,  1723,   187,    -1,   445,   190,
      -1,   192,    -1,    -1,  1732,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1748,    10,    11,    12,    -1,    -1,   896,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   485,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,  1307,
      -1,  1309,    -1,    -1,    -1,    -1,  1314,    -1,    -1,    -1,
    1318,    -1,  1320,    -1,   541,  1323,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1333,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1344,    -1,    -1,    -1,
      -1,    -1,    -1,   570,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1402,    -1,    -1,  1047,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1419,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   646,
      -1,    -1,   191,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1443,    -1,  1445,    -1,    -1,
      -1,    -1,    -1,  1451,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,   704,    -1,  1487,
    1488,    -1,    67,    -1,    -1,  1493,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1501,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1152,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   744,  1168,   746,
    1170,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1186,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   774,   775,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   790,   791,   792,   793,   794,    -1,    29,
      30,   798,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,  1602,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,   830,    -1,    -1,   191,    67,    -1,    -1,
      -1,  1619,    -1,    -1,    -1,    -1,    -1,    -1,   845,   846,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,  1636,    -1,
      -1,    -1,    -1,    -1,  1642,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   869,    -1,    -1,  1653,    79,    -1,    -1,    -1,
      -1,  1659,    -1,    -1,    -1,  1663,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1314,    -1,    -1,   100,  1318,   896,
    1320,    -1,    -1,  1323,    -1,   108,  1684,    -1,    -1,    -1,
      -1,    -1,    -1,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,    -1,    -1,   921,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1723,    -1,    -1,    -1,    -1,
     153,    -1,    -1,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,    -1,    -1,    -1,    -1,    -1,    -1,   965,   966,
      -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,  1757,
     183,    -1,  1402,    -1,   187,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1772,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   999,    -1,  1001,    -1,    -1,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,  1029,  1030,  1031,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1047,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1487,  1488,    -1,
      -1,    -1,    -1,  1493,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1089,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,  1129,    -1,    -1,    -1,    -1,  1134,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1149,  1150,    -1,  1152,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,  1168,    -1,  1170,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,  1186,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,  1642,    -1,    -1,    67,    -1,    13,    -1,  1226,
      -1,    -1,    -1,  1653,    -1,    -1,    -1,    -1,    -1,  1659,
      -1,    27,    28,  1663,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    -1,    -1,   189,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    71,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,  1723,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,  1314,  1315,    -1,
      -1,  1318,   108,  1320,    -1,    -1,  1323,    -1,    -1,    -1,
     116,   117,   118,   119,   120,   121,    -1,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,   168,    -1,    -1,   171,    -1,    -1,    -1,  1386,
      -1,   177,    10,    11,    12,    -1,   182,   183,   184,    -1,
      -1,   187,    -1,    -1,    -1,  1402,   192,   193,    -1,   195,
     196,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
    1487,  1488,    -1,    -1,    -1,    -1,  1493,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1502,    47,    48,    49,    -1,
      -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    76,    77,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    88,    89,    90,
      91,    -1,    93,    -1,    95,    -1,    97,    -1,    -1,   100,
     101,    -1,    -1,    -1,   105,   106,   107,   108,   109,   110,
     111,   189,   113,   114,   115,   116,   117,   118,   119,   120,
     121,    -1,   123,   124,   125,   126,   127,   128,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,   145,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,   165,    -1,    -1,   168,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
      -1,   182,   183,   184,    -1,  1642,   187,    -1,   189,   190,
     191,   192,   193,    -1,   195,   196,  1653,    -1,    -1,    -1,
      -1,    -1,  1659,    -1,    -1,    -1,  1663,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,  1686,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      49,    -1,    -1,    -1,    -1,    54,  1723,    56,    57,    58,
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
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    -1,
      95,    -1,    97,    -1,    -1,   100,   101,    -1,    -1,    -1,
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
      91,    -1,    93,    -1,    95,    -1,    97,    98,    -1,   100,
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
      89,    90,    91,    -1,    93,    -1,    95,    96,    97,    -1,
      -1,   100,   101,    -1,    -1,    -1,   105,   106,   107,   108,
      -1,   110,   111,    -1,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,    -1,   123,   124,   125,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,   145,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,   165,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,
     189,   190,    -1,   192,   193,    -1,   195,   196,     3,     4,
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
      -1,    88,    89,    90,    91,    -1,    93,    94,    95,    -1,
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
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
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
     183,   184,    -1,    -1,   187,    -1,   189,    -1,    -1,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    48,
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
     159,   160,    -1,   162,   163,   164,    -1,   166,    -1,   168,
      -1,    -1,   171,    -1,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,    -1,   182,   183,   184,    -1,    -1,   187,    -1,
      -1,    -1,    -1,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   187,    -1,    -1,   190,    -1,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    -1,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    48,    49,    -1,
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
      -1,   162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,
     171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,    -1,    -1,
      -1,   192,   193,    -1,   195,   196,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     187,    -1,    10,    11,    12,   192,   193,    -1,   195,   196,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    67,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      -1,    -1,    -1,    76,    77,    78,    79,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,   105,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   116,   117,   118,   119,   120,   121,    -1,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,
     153,   154,   155,   156,   157,    -1,   159,   160,    -1,   162,
     163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,    -1,
      -1,   189,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    48,
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
      -1,    -1,   187,    -1,   189,    11,    12,   192,   193,    -1,
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
     171,    -1,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   182,   183,   184,    -1,    -1,   187,    -1,   189,    -1,
      -1,   192,   193,    -1,   195,   196,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     187,    -1,    10,    11,    12,   192,   193,    -1,   195,   196,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    67,
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
      -1,   189,    -1,    -1,   177,    -1,    -1,    -1,    -1,   182,
     183,   184,    -1,    -1,   187,   188,    -1,    -1,    -1,   192,
     193,    -1,   195,   196,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   192,   193,    -1,   195,   196,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    -1,
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
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,    -1,
     195,   196,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    48,    49,    -1,
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
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
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
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,
     119,   120,   121,    -1,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,   137,   138,
     139,   140,   141,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,    -1,    -1,   153,   154,   155,   156,   157,    -1,
     159,   160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,
      -1,    -1,   171,    -1,    -1,   189,    -1,    -1,   177,    -1,
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
     135,    -1,   137,   138,   139,   140,   141,     3,     4,    -1,
       6,     7,    -1,   148,    10,    11,    12,    13,   153,   154,
     155,   156,   157,    -1,   159,   160,    -1,   162,   163,   164,
      -1,    27,    -1,   168,    -1,    -1,   171,    -1,   188,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,   182,   183,   184,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,    55,
     195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    79,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    -1,    -1,
      -1,   127,   128,   129,   130,    -1,    -1,    -1,   134,   135,
     136,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,
      -1,    -1,    -1,   159,   160,    27,   162,   163,   164,   165,
      -1,   167,    -1,    -1,   170,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    55,   190,    -1,   192,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    69,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    79,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    -1,    -1,    -1,   127,   128,   129,   130,    -1,
      -1,    -1,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,   153,    -1,    -1,    -1,    -1,    -1,   159,   160,    -1,
     162,   163,   164,   165,    27,   167,    29,    -1,   170,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    -1,   190,    -1,
     192,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,    67,
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
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    55,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
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
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    67,
       6,     7,    11,    12,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    27,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,    -1,    -1,
     188,   127,   128,   129,   130,    -1,    -1,    -1,   134,   135,
     136,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,
      -1,    -1,    -1,   159,   160,    27,   162,   163,   164,   165,
      -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    -1,    -1,    -1,   127,   128,   129,   130,    -1,
      -1,    -1,   134,   135,   136,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   153,    -1,    -1,    -1,    -1,    -1,   159,   160,    27,
     162,   163,   164,   165,    -1,   167,    -1,    -1,   170,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,    -1,    -1,    -1,    -1,
     128,   129,   130,    30,    -1,    -1,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    -1,    -1,   153,    -1,    54,    -1,    56,
      -1,   159,   160,    -1,   162,   163,   164,   165,    -1,   167,
      -1,    68,   170,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    36,    55,    -1,    -1,    -1,    -1,   135,    -1,
     137,   138,   139,   140,   141,    67,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    68,   162,   163,   164,    -1,    -1,
      -1,   168,    76,    77,    78,    79,    -1,    81,    -1,    -1,
     177,    -1,    -1,    -1,    88,   182,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,    48,    49,    -1,   168,    -1,    -1,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,    -1,
      -1,    68,    -1,   187,    -1,    -1,    -1,    -1,   192,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,   135,    -1,
     137,   138,   139,   140,   141,    67,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    68,   162,   163,   164,    -1,    -1,
      -1,   168,    76,    77,    78,    79,    -1,    81,    -1,    -1,
     177,    -1,    -1,    -1,    88,   182,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,   163,
     164,    68,    -1,    70,   168,    -1,    -1,    -1,    -1,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,   182,    -1,
      -1,    88,    -1,   187,    -1,    -1,    -1,    -1,   192,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,    -1,
     137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,    -1,    -1,   153,   154,   155,   156,
     157,    -1,   159,   160,    68,   162,   163,   164,    -1,    -1,
      -1,   168,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    88,   182,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,   192,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,   137,   138,   139,   140,   141,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,    -1,   159,   160,    68,   162,   163,
     164,    -1,    -1,    -1,   168,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,   182,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,    -1,   137,   138,   139,   140,
     141,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,
      -1,    -1,   153,   154,   155,   156,   157,    -1,   159,   160,
      -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      29,   192,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
     133,    29,    -1,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
      29,    -1,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    29,   133,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,   133,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    -1,    -1,
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    -1,
      67,    -1,    88,    -1,    -1,    -1,    -1,    -1,   133,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,
      -1,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,    -1,   133,   153,   154,   155,
     156,   157,    -1,   159,   160,    -1,   162,   163,   164,    -1,
      -1,    -1,   168,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    88,   182,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,    -1,   192,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,
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
     160,    -1,   162,   163,   164,    -1,    -1,    -1,   168,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   182,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      28,    29,   192,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    12,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    12,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    29,    -1,    -1,    32,    33,    34,    35,    36,
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
     187,   189,   190,   192,   193,   195,   196,   201,   204,   212,
     213,   214,   215,   216,   219,   235,   236,   240,   243,   250,
     256,   316,   317,   325,   329,   330,   331,   332,   333,   334,
     335,   336,   338,   341,   353,   354,   355,   357,   358,   360,
     370,   371,   372,   374,   379,   382,   401,   409,   411,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     436,   438,   440,   119,   120,   121,   134,   153,   163,   187,
     204,   235,   316,   335,   413,   335,   187,   335,   335,   335,
     105,   335,   335,   399,   400,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,    81,    88,   121,
     148,   187,   213,   354,   371,   374,   379,   413,   416,   413,
      36,   335,   427,   428,   335,   121,   127,   187,   213,   248,
     371,   372,   373,   375,   379,   410,   411,   412,   420,   424,
     425,   187,   326,   376,   187,   326,   345,   327,   335,   221,
     326,   187,   187,   187,   326,   189,   335,   204,   189,   335,
       3,     4,     6,     7,    10,    11,    12,    13,    27,    29,
      55,    57,    69,    70,    71,    72,    73,    74,    75,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   127,   128,   129,   130,   134,   135,   136,
     153,   157,   165,   167,   170,   177,   187,   204,   205,   206,
     215,   441,   456,   457,   459,   189,   332,   335,   190,   228,
     335,   108,   109,   156,   207,   208,   209,   212,    81,   192,
     282,   283,   120,   127,   119,   127,    81,   284,   187,   187,
     187,   187,   204,   254,   444,   187,   187,   327,    81,    87,
     149,   150,   151,   433,   434,   156,   190,   212,   212,   204,
     255,   444,   157,   187,   444,   444,    81,   184,   190,   346,
      27,   325,   329,   335,   336,   413,   417,   217,   190,   422,
      87,   377,   433,    87,   433,   433,    30,   156,   173,   445,
     187,     9,   189,    36,   234,   157,   253,   444,   121,   183,
     235,   317,   189,   189,   189,   189,   189,   189,    10,    11,
      12,    29,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    55,    67,   189,    68,    68,
     190,   152,   128,   163,   165,   178,   180,   256,   315,   316,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    65,    66,   131,   132,   403,    68,   190,
     408,   187,   187,    68,   190,   187,   234,   235,    14,   335,
     189,   133,    46,   204,   398,    87,   325,   336,   152,   413,
     133,   194,     9,   384,   249,   325,   336,   413,   445,   152,
     187,   378,   403,   408,   188,   335,    30,   219,     8,   347,
       9,   189,   219,   220,   327,   328,   335,   204,   268,   223,
     189,   189,   189,   135,   136,   459,   459,   173,   187,   108,
     459,    14,   152,   135,   136,   153,   204,   206,   189,   189,
     229,   112,   170,   189,   207,   208,   207,   208,   212,   190,
       9,   189,    99,   156,   190,   413,     9,   189,   127,   127,
      14,     9,   189,   413,   437,   327,   325,   336,   413,   416,
     417,   188,   173,   246,   134,   413,   426,   427,   189,    68,
     403,   149,   434,    80,   335,   413,    87,   149,   434,   212,
     203,   189,   190,   241,   251,   361,   363,    88,   187,   348,
     349,   351,   374,   419,   421,   438,    14,    99,   439,   342,
     343,   344,   278,   279,   401,   402,   188,   188,   188,   188,
     188,   191,   218,   219,   236,   243,   250,   401,   335,   193,
     195,   196,   204,   446,   447,   459,    36,   166,   280,   281,
     335,   441,   187,   444,   244,   234,   335,   335,   335,    30,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   375,   335,   335,   423,   423,   335,
     429,   430,   127,   190,   205,   206,   422,   254,   204,   255,
     444,   444,   253,   235,    36,   329,   332,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     157,   190,   204,   404,   405,   406,   407,   422,   423,   335,
     280,   280,   423,   335,   426,   234,   188,   335,   187,   397,
       9,   384,   188,   188,    36,   335,    36,   335,   378,   188,
     188,   188,   420,   421,   422,   280,   190,   204,   404,   405,
     422,   188,   217,   272,   190,   332,   335,   335,    91,    30,
     219,   266,   189,    28,    99,    14,     9,   188,    30,   190,
     269,   459,    29,    88,   215,   453,   454,   455,   187,     9,
      48,    49,    54,    56,    68,   135,   157,   177,   187,   213,
     215,   356,   371,   379,   380,   381,   204,   458,   217,   187,
     227,   190,   189,   190,   189,    99,   156,   108,   109,   156,
     209,   210,   211,   212,   209,   204,   335,   283,   380,    81,
       9,   188,   188,   188,   188,   188,   188,   188,   189,    48,
      49,   451,   452,   129,   259,   187,     9,   188,   188,    81,
      82,   204,   435,   204,    68,   191,   191,   200,   202,    30,
     130,   258,   172,    52,   157,   172,   365,   336,   133,     9,
     384,   188,   152,   459,   459,    14,   347,   278,   217,   185,
       9,   385,   459,   460,   403,   408,   403,   191,     9,   384,
     174,   413,   335,   188,     9,   385,    14,   339,   237,   129,
     257,   187,   444,   335,    30,   194,   194,   133,   191,     9,
     384,   335,   445,   187,   247,   242,   252,    14,   439,   245,
     234,    70,   413,   335,   445,   194,   191,   188,   188,   194,
     191,   188,    48,    49,    68,    76,    77,    78,    88,   135,
     148,   177,   204,   387,   389,   390,   393,   396,   204,   413,
     413,   133,   257,   403,   408,   188,   335,   273,    73,    74,
     274,   217,   326,   217,   328,    99,    36,   134,   263,   413,
     380,   204,    30,   219,   267,   189,   270,   189,   270,     9,
     174,    88,   133,   152,     9,   384,   188,   166,   446,   447,
     448,   446,   380,   380,   380,   380,   380,   383,   386,   187,
     152,   187,   380,   152,   190,    10,    11,    12,    29,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    67,   152,   445,   191,   371,   190,   231,   208,   208,
     204,   209,   209,   212,     9,   191,   191,    14,   413,   189,
       9,   174,   204,   260,   371,   190,   426,   134,   413,    14,
     194,   335,   191,   200,   459,   260,   190,   364,    14,   188,
     335,   348,   422,   189,   459,   185,   191,    30,   449,   402,
      36,    81,   166,   404,   405,   407,   404,   405,   459,    36,
     166,   335,   380,   278,   187,   371,   258,   340,   238,   335,
     335,   335,   191,   187,   280,   259,    30,   258,   459,    14,
     257,   444,   375,   191,   187,    14,    76,    77,    78,   204,
     388,   388,   390,   391,   392,    50,   187,    87,   149,   187,
       9,   384,   188,   397,    36,   335,   258,   191,    73,    74,
     275,   326,   219,   191,   189,    92,   189,   263,   413,   187,
     133,   262,    14,   217,   270,   102,   103,   104,   270,   191,
     459,   133,   459,   204,   453,     9,   188,   384,   133,   194,
       9,   384,   383,   205,   348,   350,   352,   188,   127,   205,
     380,   431,   432,   380,   380,   380,    30,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   458,    81,   232,   191,   191,   211,   189,   380,   452,
      99,   100,   450,     9,   288,   188,   187,   329,   332,   335,
     194,   191,   439,   288,   158,   171,   190,   360,   367,   158,
     190,   366,   133,   189,   449,   459,   347,   460,    81,   166,
      14,    81,   445,   413,   335,   188,   278,   190,   278,   187,
     133,   187,   280,   188,   190,   459,   190,   189,   459,   258,
     239,   378,   280,   133,   194,     9,   384,   389,   391,   149,
     348,   394,   395,   390,   413,   190,   326,    30,    75,   219,
     189,   328,   262,   426,   263,   188,   380,    98,   102,   189,
     335,    30,   189,   271,   191,   174,   459,   133,   166,    30,
     188,   380,   380,   188,   133,     9,   384,   188,   133,   191,
       9,   384,   380,    30,   188,   217,   189,   189,   204,   459,
     459,   371,     4,   109,   114,   120,   122,   159,   160,   162,
     191,   289,   314,   315,   316,   321,   322,   323,   324,   401,
     426,   191,   190,   191,    52,   335,   335,   335,   347,    36,
      81,   166,    14,    81,   335,   187,   449,   188,   288,   188,
     278,   335,   280,   188,   288,   439,   288,   189,   190,   187,
     188,   390,   390,   188,   133,   188,     9,   384,   288,    30,
     217,   189,   188,   188,   188,   224,   189,   189,   271,   217,
     459,   459,   133,   380,   348,   380,   380,   380,   190,   191,
     450,   129,   130,   178,   205,   442,   459,   261,   371,   109,
     324,    29,   122,   135,   136,   157,   163,   298,   299,   300,
     301,   371,   161,   306,   307,   125,   187,   204,   308,   309,
     290,   235,   459,     9,   189,     9,   189,   189,   439,   315,
     188,   285,   157,   362,   191,   191,    81,   166,    14,    81,
     335,   280,   114,   337,   449,   191,   449,   188,   188,   191,
     190,   191,   288,   278,   133,   390,   348,   191,   217,   222,
     225,    30,   219,   265,   217,   188,   380,   133,   133,   217,
     371,   371,   444,    14,   205,     9,   189,   190,   442,   439,
     301,   173,   190,     9,   189,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    55,    69,    70,    71,
      72,    73,    74,    75,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   134,   135,   137,   138,   139,
     140,   141,   153,   154,   155,   165,   167,   168,   170,   177,
     178,   180,   182,   183,   204,   368,   369,     9,   189,   157,
     161,   204,   309,   310,   311,   189,    81,   320,   234,   291,
     442,   442,    14,   235,   191,   286,   287,   442,    14,    81,
     335,   188,   187,   190,   189,   190,   312,   337,   449,   285,
     191,   188,   390,   133,    30,   219,   264,   265,   217,   380,
     380,   191,   189,   189,   380,   371,   294,   459,   302,   303,
     379,   299,    14,    30,    49,   304,   307,     9,    34,   188,
      29,    48,    51,    14,     9,   189,   206,   443,   320,    14,
     459,   234,   189,    14,   335,    36,    81,   359,   217,   217,
     190,   312,   191,   449,   390,   217,    96,   230,   191,   204,
     215,   295,   296,   297,     9,   174,     9,   384,   191,   380,
     369,   369,    57,   305,   310,   310,    29,    48,    51,   380,
      81,   173,   187,   189,   380,   444,   380,    81,     9,   385,
     191,   191,   217,   312,    94,   189,   112,   226,   152,    99,
     459,   379,   164,    14,   451,   292,   187,    36,    81,   188,
     191,   189,   187,   170,   233,   204,   315,   316,   174,   380,
     174,   276,   277,   402,   293,    81,   371,   231,   167,   204,
     189,   188,     9,   385,   116,   117,   118,   318,   319,   276,
      81,   261,   189,   449,   402,   460,   188,   188,   189,   189,
     190,   313,   318,    36,    81,   166,   449,   190,   217,   460,
      81,   166,    14,    81,   313,   217,   191,    36,    81,   166,
      14,    81,   335,   191,    81,   166,    14,    81,   335,    14,
      81,   335,   335
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
#line 877 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { _p->onUseDeclaration((yyval), (yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(1) - (1)]),
                                           &Parser::useClass);;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 895 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useFunction);;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 897 "hphp.y"
    { _p->onMixedUseDeclaration((yyval), (yyvsp[(2) - (2)]),
                                           &Parser::useConst);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 904 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 943 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 989 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 990 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 993 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1033 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1034 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1048 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { (yyval).reset();;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1062 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { (yyval).reset();;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { (yyval).reset();;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1080 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1086 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1092 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1105 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1112 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1130 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1144 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
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

  case 193:

/* Line 1455 of yacc.c  */
#line 1162 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
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

  case 195:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1196 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1199 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1230 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { (yyval).reset();;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { (yyval).reset();;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval).reset();;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval).reset();;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1274 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { (yyval).reset();;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { (yyval).reset();;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { (yyval).reset();;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { (yyval).reset();;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1333 "hphp.y"
    { (yyval).reset();;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1382 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1437 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1453 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { (yyval).reset();;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1472 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1484 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { (yyval).reset();;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval).reset();;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { (yyval).reset();;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { (yyval).reset();;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { (yyval).reset();;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyval).reset();;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval).reset();;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval).reset();;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval).reset();;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { _p->onNullCoalesce((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval).reset();;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
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

  case 514:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
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

  case 516:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
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

  case 518:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2073 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
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
#line 2117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
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

  case 553:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
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

  case 554:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval).reset();;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval).reset();;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval).reset();;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval).reset();;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval).reset();;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval).reset();;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { (yyval).reset();;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { (yyval).reset();;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { (yyval).reset();;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { (yyval).reset();;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { (yyval).reset();;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { (yyval).reset();;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval).reset();;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 807:

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

  case 808:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
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

  case 809:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
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

  case 811:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2743 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
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

  case 818:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
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
#line 2795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { (yyval).reset();;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
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
#line 2911 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2912 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
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

  case 867:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2937 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { (yyval).reset();;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2987 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
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

  case 903:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3036 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3090 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3096 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3106 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3163 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    {;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    {;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3169 "hphp.y"
    {;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3191 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3196 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3197 "hphp.y"
    { ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3202 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3223 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3230 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3237 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3240 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3250 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3253 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3262 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3268 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3277 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13707 "hphp.7.tab.cpp"
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
#line 3280 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

