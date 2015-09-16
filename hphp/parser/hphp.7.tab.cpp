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
     T_BOOLEAN_OR = 282,
     T_BOOLEAN_AND = 283,
     T_IS_NOT_IDENTICAL = 284,
     T_IS_IDENTICAL = 285,
     T_IS_NOT_EQUAL = 286,
     T_IS_EQUAL = 287,
     T_SPACESHIP = 288,
     T_IS_GREATER_OR_EQUAL = 289,
     T_IS_SMALLER_OR_EQUAL = 290,
     T_SR = 291,
     T_SL = 292,
     T_INSTANCEOF = 293,
     T_UNSET_CAST = 294,
     T_BOOL_CAST = 295,
     T_OBJECT_CAST = 296,
     T_ARRAY_CAST = 297,
     T_STRING_CAST = 298,
     T_DOUBLE_CAST = 299,
     T_INT_CAST = 300,
     T_DEC = 301,
     T_INC = 302,
     T_POW = 303,
     T_CLONE = 304,
     T_NEW = 305,
     T_EXIT = 306,
     T_IF = 307,
     T_ELSEIF = 308,
     T_ELSE = 309,
     T_ENDIF = 310,
     T_LNUMBER = 311,
     T_DNUMBER = 312,
     T_ONUMBER = 313,
     T_STRING = 314,
     T_STRING_VARNAME = 315,
     T_VARIABLE = 316,
     T_NUM_STRING = 317,
     T_INLINE_HTML = 318,
     T_HASHBANG = 319,
     T_CHARACTER = 320,
     T_BAD_CHARACTER = 321,
     T_ENCAPSED_AND_WHITESPACE = 322,
     T_CONSTANT_ENCAPSED_STRING = 323,
     T_ECHO = 324,
     T_DO = 325,
     T_WHILE = 326,
     T_ENDWHILE = 327,
     T_FOR = 328,
     T_ENDFOR = 329,
     T_FOREACH = 330,
     T_ENDFOREACH = 331,
     T_DECLARE = 332,
     T_ENDDECLARE = 333,
     T_AS = 334,
     T_SUPER = 335,
     T_SWITCH = 336,
     T_ENDSWITCH = 337,
     T_CASE = 338,
     T_DEFAULT = 339,
     T_BREAK = 340,
     T_GOTO = 341,
     T_CONTINUE = 342,
     T_FUNCTION = 343,
     T_CONST = 344,
     T_RETURN = 345,
     T_TRY = 346,
     T_CATCH = 347,
     T_THROW = 348,
     T_USE = 349,
     T_GLOBAL = 350,
     T_PUBLIC = 351,
     T_PROTECTED = 352,
     T_PRIVATE = 353,
     T_FINAL = 354,
     T_ABSTRACT = 355,
     T_STATIC = 356,
     T_VAR = 357,
     T_UNSET = 358,
     T_ISSET = 359,
     T_EMPTY = 360,
     T_HALT_COMPILER = 361,
     T_CLASS = 362,
     T_INTERFACE = 363,
     T_EXTENDS = 364,
     T_IMPLEMENTS = 365,
     T_OBJECT_OPERATOR = 366,
     T_NULLSAFE_OBJECT_OPERATOR = 367,
     T_DOUBLE_ARROW = 368,
     T_LIST = 369,
     T_ARRAY = 370,
     T_CALLABLE = 371,
     T_CLASS_C = 372,
     T_METHOD_C = 373,
     T_FUNC_C = 374,
     T_LINE = 375,
     T_FILE = 376,
     T_COMMENT = 377,
     T_DOC_COMMENT = 378,
     T_OPEN_TAG = 379,
     T_OPEN_TAG_WITH_ECHO = 380,
     T_CLOSE_TAG = 381,
     T_WHITESPACE = 382,
     T_START_HEREDOC = 383,
     T_END_HEREDOC = 384,
     T_DOLLAR_OPEN_CURLY_BRACES = 385,
     T_CURLY_OPEN = 386,
     T_DOUBLE_COLON = 387,
     T_NAMESPACE = 388,
     T_NS_C = 389,
     T_DIR = 390,
     T_NS_SEPARATOR = 391,
     T_XHP_LABEL = 392,
     T_XHP_TEXT = 393,
     T_XHP_ATTRIBUTE = 394,
     T_XHP_CATEGORY = 395,
     T_XHP_CATEGORY_LABEL = 396,
     T_XHP_CHILDREN = 397,
     T_ENUM = 398,
     T_XHP_REQUIRED = 399,
     T_TRAIT = 400,
     T_ELLIPSIS = 401,
     T_INSTEADOF = 402,
     T_TRAIT_C = 403,
     T_HH_ERROR = 404,
     T_FINALLY = 405,
     T_XHP_TAG_LT = 406,
     T_XHP_TAG_GT = 407,
     T_TYPELIST_LT = 408,
     T_TYPELIST_GT = 409,
     T_UNRESOLVED_LT = 410,
     T_COLLECTION = 411,
     T_SHAPE = 412,
     T_TYPE = 413,
     T_UNRESOLVED_TYPE = 414,
     T_NEWTYPE = 415,
     T_UNRESOLVED_NEWTYPE = 416,
     T_COMPILER_HALT_OFFSET = 417,
     T_ASYNC = 418,
     T_FROM = 419,
     T_WHERE = 420,
     T_JOIN = 421,
     T_IN = 422,
     T_ON = 423,
     T_EQUALS = 424,
     T_INTO = 425,
     T_LET = 426,
     T_ORDERBY = 427,
     T_ASCENDING = 428,
     T_DESCENDING = 429,
     T_SELECT = 430,
     T_GROUP = 431,
     T_BY = 432,
     T_LAMBDA_OP = 433,
     T_LAMBDA_CP = 434,
     T_UNRESOLVED_OP = 435
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
#line 886 "hphp.7.tab.cpp"

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
#define YYLAST   18727

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  210
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  281
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1022
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1858

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   435

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    53,   208,     2,   205,    52,    35,   209,
     200,   201,    50,    47,     9,    48,    49,    51,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   202,
      40,    14,    41,    29,    56,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    67,     2,   207,    34,     2,   206,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   203,    33,   204,    55,     2,     2,     2,
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
      27,    28,    31,    32,    36,    37,    38,    39,    42,    43,
      44,    45,    46,    54,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    68,    69,    70,    71,    72,    73,
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
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199
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
     200,   202,   204,   206,   208,   210,   212,   214,   216,   218,
     220,   222,   224,   226,   228,   230,   234,   236,   240,   242,
     246,   248,   250,   253,   257,   262,   264,   267,   271,   276,
     278,   281,   285,   290,   292,   296,   298,   302,   305,   307,
     310,   313,   319,   324,   327,   328,   330,   332,   334,   336,
     340,   346,   355,   356,   361,   362,   369,   370,   381,   382,
     387,   390,   394,   397,   401,   404,   408,   412,   416,   420,
     424,   428,   434,   436,   438,   440,   441,   451,   452,   463,
     469,   470,   484,   485,   491,   495,   499,   502,   505,   508,
     511,   514,   517,   521,   524,   527,   531,   534,   535,   540,
     550,   551,   552,   557,   560,   561,   563,   564,   566,   567,
     577,   578,   589,   590,   602,   603,   613,   614,   625,   626,
     635,   636,   646,   647,   655,   656,   665,   666,   674,   675,
     684,   686,   688,   690,   692,   694,   697,   701,   705,   708,
     711,   712,   715,   716,   719,   720,   722,   726,   728,   732,
     735,   736,   738,   741,   746,   748,   753,   755,   760,   762,
     767,   769,   774,   778,   784,   788,   793,   798,   804,   810,
     815,   816,   818,   820,   825,   826,   832,   833,   836,   837,
     841,   842,   850,   859,   866,   869,   875,   882,   887,   888,
     893,   899,   907,   914,   921,   929,   939,   948,   955,   963,
     969,   972,   977,   983,   987,   988,   992,   997,  1004,  1010,
    1016,  1023,  1032,  1040,  1043,  1044,  1046,  1049,  1052,  1056,
    1061,  1066,  1070,  1072,  1074,  1077,  1082,  1086,  1092,  1094,
    1098,  1101,  1102,  1105,  1109,  1112,  1113,  1114,  1119,  1120,
    1126,  1129,  1132,  1135,  1136,  1147,  1148,  1160,  1164,  1168,
    1172,  1177,  1182,  1186,  1192,  1195,  1198,  1199,  1206,  1212,
    1217,  1221,  1223,  1225,  1229,  1234,  1236,  1239,  1241,  1243,
    1248,  1255,  1257,  1259,  1264,  1266,  1268,  1272,  1275,  1278,
    1279,  1282,  1283,  1285,  1289,  1291,  1293,  1295,  1297,  1301,
    1306,  1311,  1316,  1318,  1320,  1323,  1326,  1329,  1333,  1337,
    1339,  1341,  1343,  1345,  1349,  1351,  1355,  1357,  1359,  1361,
    1362,  1364,  1367,  1369,  1371,  1373,  1375,  1377,  1379,  1381,
    1383,  1384,  1386,  1388,  1390,  1394,  1400,  1402,  1406,  1412,
    1417,  1421,  1425,  1429,  1434,  1438,  1442,  1446,  1449,  1451,
    1453,  1457,  1461,  1463,  1465,  1466,  1468,  1471,  1476,  1480,
    1487,  1490,  1494,  1501,  1503,  1505,  1507,  1509,  1511,  1518,
    1522,  1527,  1534,  1538,  1542,  1546,  1550,  1554,  1558,  1562,
    1566,  1570,  1574,  1578,  1582,  1585,  1588,  1591,  1594,  1598,
    1602,  1606,  1610,  1614,  1618,  1622,  1626,  1630,  1634,  1638,
    1642,  1646,  1650,  1654,  1658,  1662,  1665,  1668,  1671,  1674,
    1678,  1682,  1686,  1690,  1694,  1698,  1702,  1706,  1710,  1714,
    1718,  1724,  1729,  1731,  1734,  1737,  1740,  1743,  1746,  1749,
    1752,  1755,  1758,  1760,  1762,  1764,  1768,  1771,  1773,  1779,
    1780,  1781,  1793,  1794,  1807,  1808,  1812,  1813,  1818,  1819,
    1826,  1827,  1835,  1836,  1842,  1845,  1848,  1853,  1855,  1857,
    1863,  1867,  1873,  1877,  1880,  1881,  1884,  1885,  1890,  1895,
    1899,  1904,  1909,  1914,  1919,  1921,  1923,  1925,  1927,  1931,
    1934,  1938,  1943,  1946,  1950,  1952,  1955,  1957,  1960,  1962,
    1964,  1966,  1968,  1970,  1972,  1977,  1982,  1985,  1994,  2005,
    2008,  2010,  2014,  2016,  2019,  2021,  2023,  2025,  2027,  2030,
    2035,  2039,  2043,  2048,  2050,  2053,  2058,  2061,  2068,  2069,
    2071,  2076,  2077,  2080,  2081,  2083,  2085,  2089,  2091,  2095,
    2097,  2099,  2103,  2107,  2109,  2111,  2113,  2115,  2117,  2119,
    2121,  2123,  2125,  2127,  2129,  2131,  2133,  2135,  2137,  2139,
    2141,  2143,  2145,  2147,  2149,  2151,  2153,  2155,  2157,  2159,
    2161,  2163,  2165,  2167,  2169,  2171,  2173,  2175,  2177,  2179,
    2181,  2183,  2185,  2187,  2189,  2191,  2193,  2195,  2197,  2199,
    2201,  2203,  2205,  2207,  2209,  2211,  2213,  2215,  2217,  2219,
    2221,  2223,  2225,  2227,  2229,  2231,  2233,  2235,  2237,  2239,
    2241,  2243,  2245,  2247,  2249,  2251,  2253,  2255,  2257,  2259,
    2261,  2263,  2265,  2267,  2272,  2274,  2276,  2278,  2280,  2282,
    2284,  2288,  2292,  2294,  2296,  2298,  2301,  2303,  2304,  2305,
    2307,  2309,  2313,  2314,  2316,  2318,  2320,  2322,  2324,  2326,
    2328,  2330,  2332,  2334,  2336,  2338,  2340,  2344,  2347,  2349,
    2351,  2356,  2360,  2365,  2367,  2369,  2373,  2377,  2381,  2385,
    2389,  2393,  2397,  2401,  2405,  2409,  2413,  2417,  2421,  2425,
    2429,  2433,  2437,  2441,  2444,  2447,  2450,  2453,  2457,  2461,
    2465,  2469,  2473,  2477,  2481,  2485,  2489,  2495,  2500,  2504,
    2508,  2512,  2514,  2516,  2518,  2520,  2524,  2528,  2532,  2535,
    2536,  2538,  2539,  2541,  2542,  2548,  2552,  2556,  2558,  2560,
    2562,  2564,  2568,  2571,  2573,  2575,  2577,  2579,  2581,  2585,
    2587,  2589,  2591,  2594,  2597,  2602,  2606,  2611,  2614,  2615,
    2621,  2625,  2629,  2631,  2635,  2637,  2640,  2641,  2647,  2651,
    2654,  2655,  2659,  2660,  2665,  2668,  2669,  2673,  2677,  2679,
    2680,  2682,  2684,  2686,  2688,  2692,  2694,  2696,  2698,  2702,
    2704,  2706,  2710,  2714,  2717,  2722,  2725,  2730,  2736,  2742,
    2748,  2754,  2756,  2758,  2760,  2762,  2764,  2766,  2770,  2774,
    2779,  2784,  2788,  2790,  2792,  2794,  2796,  2800,  2802,  2807,
    2811,  2815,  2817,  2819,  2821,  2823,  2825,  2829,  2833,  2838,
    2843,  2847,  2849,  2851,  2859,  2869,  2877,  2884,  2893,  2895,
    2900,  2905,  2907,  2909,  2914,  2917,  2919,  2920,  2922,  2924,
    2926,  2930,  2934,  2938,  2939,  2941,  2943,  2947,  2951,  2954,
    2958,  2965,  2966,  2968,  2973,  2976,  2977,  2983,  2987,  2991,
    2993,  3000,  3005,  3010,  3013,  3016,  3017,  3023,  3027,  3031,
    3033,  3036,  3037,  3043,  3047,  3051,  3053,  3056,  3059,  3061,
    3064,  3066,  3071,  3075,  3079,  3086,  3090,  3092,  3094,  3096,
    3101,  3106,  3111,  3116,  3121,  3126,  3129,  3132,  3137,  3140,
    3143,  3145,  3149,  3153,  3157,  3158,  3161,  3167,  3174,  3181,
    3189,  3191,  3194,  3196,  3199,  3201,  3206,  3208,  3213,  3217,
    3218,  3220,  3224,  3227,  3231,  3233,  3235,  3236,  3237,  3240,
    3243,  3246,  3251,  3254,  3260,  3264,  3266,  3268,  3269,  3273,
    3278,  3284,  3288,  3290,  3293,  3294,  3299,  3301,  3305,  3308,
    3311,  3314,  3316,  3318,  3320,  3322,  3326,  3331,  3338,  3340,
    3349,  3356,  3358
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     211,     0,    -1,    -1,   212,   213,    -1,   213,   214,    -1,
      -1,   234,    -1,   251,    -1,   258,    -1,   255,    -1,   263,
      -1,   470,    -1,   125,   200,   201,   202,    -1,   152,   226,
     202,    -1,    -1,   152,   226,   203,   215,   213,   204,    -1,
      -1,   152,   203,   216,   213,   204,    -1,   113,   220,   202,
      -1,   113,   107,   221,   202,    -1,   113,   108,   222,   202,
      -1,   231,   202,    -1,    78,    -1,    99,    -1,   158,    -1,
     159,    -1,   161,    -1,   163,    -1,   162,    -1,   184,    -1,
     185,    -1,   187,    -1,   186,    -1,   188,    -1,   189,    -1,
     190,    -1,   191,    -1,   192,    -1,   193,    -1,   194,    -1,
     195,    -1,   196,    -1,   217,    -1,   135,    -1,   164,    -1,
     128,    -1,   129,    -1,   120,    -1,   119,    -1,   118,    -1,
     117,    -1,   116,    -1,   115,    -1,   108,    -1,    97,    -1,
      93,    -1,    95,    -1,    74,    -1,    91,    -1,    12,    -1,
     114,    -1,   105,    -1,    54,    -1,   166,    -1,   127,    -1,
     152,    -1,    69,    -1,    10,    -1,    11,    -1,   110,    -1,
     113,    -1,   121,    -1,    70,    -1,   133,    -1,    68,    -1,
       7,    -1,     6,    -1,   112,    -1,   134,    -1,    13,    -1,
      88,    -1,     4,    -1,     3,    -1,   109,    -1,    73,    -1,
      72,    -1,   103,    -1,   104,    -1,   106,    -1,   100,    -1,
      27,    -1,   107,    -1,    71,    -1,   101,    -1,   169,    -1,
      92,    -1,    94,    -1,    96,    -1,   102,    -1,    89,    -1,
      90,    -1,    98,    -1,   111,    -1,   218,    -1,   126,    -1,
     220,     9,   223,    -1,   223,    -1,   224,     9,   224,    -1,
     224,    -1,   225,     9,   225,    -1,   225,    -1,   226,    -1,
     155,   226,    -1,   226,    98,   217,    -1,   155,   226,    98,
     217,    -1,   226,    -1,   155,   226,    -1,   226,    98,   217,
      -1,   155,   226,    98,   217,    -1,   226,    -1,   155,   226,
      -1,   226,    98,   217,    -1,   155,   226,    98,   217,    -1,
     217,    -1,   226,   155,   217,    -1,   226,    -1,   152,   155,
     226,    -1,   155,   226,    -1,   227,    -1,   227,   475,    -1,
     227,   475,    -1,   231,     9,   471,    14,   410,    -1,   108,
     471,    14,   410,    -1,   232,   233,    -1,    -1,   234,    -1,
     251,    -1,   258,    -1,   263,    -1,   203,   232,   204,    -1,
      71,   339,   234,   285,   287,    -1,    71,   339,    30,   232,
     286,   288,    74,   202,    -1,    -1,    90,   339,   235,   279,
      -1,    -1,    89,   236,   234,    90,   339,   202,    -1,    -1,
      92,   200,   341,   202,   341,   202,   341,   201,   237,   277,
      -1,    -1,   100,   339,   238,   282,    -1,   104,   202,    -1,
     104,   348,   202,    -1,   106,   202,    -1,   106,   348,   202,
      -1,   109,   202,    -1,   109,   348,   202,    -1,    27,   104,
     202,    -1,   114,   295,   202,    -1,   120,   297,   202,    -1,
      88,   340,   202,    -1,   144,   340,   202,    -1,   122,   200,
     467,   201,   202,    -1,   202,    -1,    82,    -1,    83,    -1,
      -1,    94,   200,   348,    98,   276,   275,   201,   239,   278,
      -1,    -1,    94,   200,   348,    28,    98,   276,   275,   201,
     240,   278,    -1,    96,   200,   281,   201,   280,    -1,    -1,
     110,   243,   111,   200,   403,    80,   201,   203,   232,   204,
     245,   241,   248,    -1,    -1,   110,   243,   169,   242,   246,
      -1,   112,   348,   202,    -1,   105,   217,   202,    -1,   348,
     202,    -1,   342,   202,    -1,   343,   202,    -1,   344,   202,
      -1,   345,   202,    -1,   346,   202,    -1,   109,   345,   202,
      -1,   347,   202,    -1,   373,   202,    -1,   109,   372,   202,
      -1,   217,    30,    -1,    -1,   203,   244,   232,   204,    -1,
     245,   111,   200,   403,    80,   201,   203,   232,   204,    -1,
      -1,    -1,   203,   247,   232,   204,    -1,   169,   246,    -1,
      -1,    35,    -1,    -1,   107,    -1,    -1,   250,   249,   474,
     252,   200,   291,   201,   479,   325,    -1,    -1,   329,   250,
     249,   474,   253,   200,   291,   201,   479,   325,    -1,    -1,
     431,   328,   250,   249,   474,   254,   200,   291,   201,   479,
     325,    -1,    -1,   162,   217,   256,    30,   489,   469,   203,
     298,   204,    -1,    -1,   431,   162,   217,   257,    30,   489,
     469,   203,   298,   204,    -1,    -1,   269,   266,   259,   270,
     271,   203,   301,   204,    -1,    -1,   431,   269,   266,   260,
     270,   271,   203,   301,   204,    -1,    -1,   127,   267,   261,
     272,   203,   301,   204,    -1,    -1,   431,   127,   267,   262,
     272,   203,   301,   204,    -1,    -1,   164,   268,   264,   271,
     203,   301,   204,    -1,    -1,   431,   164,   268,   265,   271,
     203,   301,   204,    -1,   474,    -1,   156,    -1,   474,    -1,
     474,    -1,   126,    -1,   119,   126,    -1,   119,   118,   126,
      -1,   118,   119,   126,    -1,   118,   126,    -1,   128,   403,
      -1,    -1,   129,   273,    -1,    -1,   128,   273,    -1,    -1,
     403,    -1,   273,     9,   403,    -1,   403,    -1,   274,     9,
     403,    -1,   132,   276,    -1,    -1,   443,    -1,    35,   443,
      -1,   133,   200,   456,   201,    -1,   234,    -1,    30,   232,
      93,   202,    -1,   234,    -1,    30,   232,    95,   202,    -1,
     234,    -1,    30,   232,    91,   202,    -1,   234,    -1,    30,
     232,    97,   202,    -1,   217,    14,   410,    -1,   281,     9,
     217,    14,   410,    -1,   203,   283,   204,    -1,   203,   202,
     283,   204,    -1,    30,   283,   101,   202,    -1,    30,   202,
     283,   101,   202,    -1,   283,   102,   348,   284,   232,    -1,
     283,   103,   284,   232,    -1,    -1,    30,    -1,   202,    -1,
     285,    72,   339,   234,    -1,    -1,   286,    72,   339,    30,
     232,    -1,    -1,    73,   234,    -1,    -1,    73,    30,   232,
      -1,    -1,   290,     9,   432,   331,   490,   165,    80,    -1,
     290,     9,   432,   331,   490,    35,   165,    80,    -1,   290,
       9,   432,   331,   490,   165,    -1,   290,   415,    -1,   432,
     331,   490,   165,    80,    -1,   432,   331,   490,    35,   165,
      80,    -1,   432,   331,   490,   165,    -1,    -1,   432,   331,
     490,    80,    -1,   432,   331,   490,    35,    80,    -1,   432,
     331,   490,    35,    80,    14,   348,    -1,   432,   331,   490,
      80,    14,   348,    -1,   290,     9,   432,   331,   490,    80,
      -1,   290,     9,   432,   331,   490,    35,    80,    -1,   290,
       9,   432,   331,   490,    35,    80,    14,   348,    -1,   290,
       9,   432,   331,   490,    80,    14,   348,    -1,   292,     9,
     432,   490,   165,    80,    -1,   292,     9,   432,   490,    35,
     165,    80,    -1,   292,     9,   432,   490,   165,    -1,   292,
     415,    -1,   432,   490,   165,    80,    -1,   432,   490,    35,
     165,    80,    -1,   432,   490,   165,    -1,    -1,   432,   490,
      80,    -1,   432,   490,    35,    80,    -1,   432,   490,    35,
      80,    14,   348,    -1,   432,   490,    80,    14,   348,    -1,
     292,     9,   432,   490,    80,    -1,   292,     9,   432,   490,
      35,    80,    -1,   292,     9,   432,   490,    35,    80,    14,
     348,    -1,   292,     9,   432,   490,    80,    14,   348,    -1,
     294,   415,    -1,    -1,   348,    -1,    35,   443,    -1,   165,
     348,    -1,   294,     9,   348,    -1,   294,     9,   165,   348,
      -1,   294,     9,    35,   443,    -1,   295,     9,   296,    -1,
     296,    -1,    80,    -1,   205,   443,    -1,   205,   203,   348,
     204,    -1,   297,     9,    80,    -1,   297,     9,    80,    14,
     410,    -1,    80,    -1,    80,    14,   410,    -1,   298,   299,
      -1,    -1,   300,   202,    -1,   472,    14,   410,    -1,   301,
     302,    -1,    -1,    -1,   327,   303,   333,   202,    -1,    -1,
     329,   489,   304,   333,   202,    -1,   334,   202,    -1,   335,
     202,    -1,   336,   202,    -1,    -1,   328,   250,   249,   473,
     200,   305,   289,   201,   479,   326,    -1,    -1,   431,   328,
     250,   249,   474,   200,   306,   289,   201,   479,   326,    -1,
     158,   311,   202,    -1,   159,   319,   202,    -1,   161,   321,
     202,    -1,     4,   128,   403,   202,    -1,     4,   129,   403,
     202,    -1,   113,   274,   202,    -1,   113,   274,   203,   307,
     204,    -1,   307,   308,    -1,   307,   309,    -1,    -1,   230,
     151,   217,   166,   274,   202,    -1,   310,    98,   328,   217,
     202,    -1,   310,    98,   329,   202,    -1,   230,   151,   217,
      -1,   217,    -1,   312,    -1,   311,     9,   312,    -1,   313,
     400,   317,   318,    -1,   156,    -1,    29,   314,    -1,   314,
      -1,   134,    -1,   134,   172,   489,   173,    -1,   134,   172,
     489,     9,   489,   173,    -1,   403,    -1,   121,    -1,   162,
     203,   316,   204,    -1,   135,    -1,   409,    -1,   315,     9,
     409,    -1,   315,   414,    -1,    14,   410,    -1,    -1,    56,
     163,    -1,    -1,   320,    -1,   319,     9,   320,    -1,   160,
      -1,   322,    -1,   217,    -1,   124,    -1,   200,   323,   201,
      -1,   200,   323,   201,    50,    -1,   200,   323,   201,    29,
      -1,   200,   323,   201,    47,    -1,   322,    -1,   324,    -1,
     324,    50,    -1,   324,    29,    -1,   324,    47,    -1,   323,
       9,   323,    -1,   323,    33,   323,    -1,   217,    -1,   156,
      -1,   160,    -1,   202,    -1,   203,   232,   204,    -1,   202,
      -1,   203,   232,   204,    -1,   329,    -1,   121,    -1,   329,
      -1,    -1,   330,    -1,   329,   330,    -1,   115,    -1,   116,
      -1,   117,    -1,   120,    -1,   119,    -1,   118,    -1,   182,
      -1,   332,    -1,    -1,   115,    -1,   116,    -1,   117,    -1,
     333,     9,    80,    -1,   333,     9,    80,    14,   410,    -1,
      80,    -1,    80,    14,   410,    -1,   334,     9,   472,    14,
     410,    -1,   108,   472,    14,   410,    -1,   335,     9,   472,
      -1,   119,   108,   472,    -1,   119,   337,   469,    -1,   337,
     469,    14,   489,    -1,   108,   177,   474,    -1,   200,   338,
     201,    -1,    69,   405,   408,    -1,    68,   348,    -1,   392,
      -1,   368,    -1,   200,   348,   201,    -1,   340,     9,   348,
      -1,   348,    -1,   340,    -1,    -1,    27,    -1,    27,   348,
      -1,    27,   348,   132,   348,    -1,   443,    14,   342,    -1,
     133,   200,   456,   201,    14,   342,    -1,    28,   348,    -1,
     443,    14,   345,    -1,   133,   200,   456,   201,    14,   345,
      -1,   349,    -1,   443,    -1,   338,    -1,   447,    -1,   446,
      -1,   133,   200,   456,   201,    14,   348,    -1,   443,    14,
     348,    -1,   443,    14,    35,   443,    -1,   443,    14,    35,
      69,   405,   408,    -1,   443,    26,   348,    -1,   443,    25,
     348,    -1,   443,    24,   348,    -1,   443,    23,   348,    -1,
     443,    22,   348,    -1,   443,    21,   348,    -1,   443,    20,
     348,    -1,   443,    19,   348,    -1,   443,    18,   348,    -1,
     443,    17,   348,    -1,   443,    16,   348,    -1,   443,    15,
     348,    -1,   443,    65,    -1,    65,   443,    -1,   443,    64,
      -1,    64,   443,    -1,   348,    31,   348,    -1,   348,    32,
     348,    -1,   348,    10,   348,    -1,   348,    12,   348,    -1,
     348,    11,   348,    -1,   348,    33,   348,    -1,   348,    35,
     348,    -1,   348,    34,   348,    -1,   348,    49,   348,    -1,
     348,    47,   348,    -1,   348,    48,   348,    -1,   348,    50,
     348,    -1,   348,    51,   348,    -1,   348,    66,   348,    -1,
     348,    52,   348,    -1,   348,    46,   348,    -1,   348,    45,
     348,    -1,    47,   348,    -1,    48,   348,    -1,    53,   348,
      -1,    55,   348,    -1,   348,    37,   348,    -1,   348,    36,
     348,    -1,   348,    39,   348,    -1,   348,    38,   348,    -1,
     348,    40,   348,    -1,   348,    44,   348,    -1,   348,    41,
     348,    -1,   348,    43,   348,    -1,   348,    42,   348,    -1,
     348,    54,   405,    -1,   200,   349,   201,    -1,   348,    29,
     348,    30,   348,    -1,   348,    29,    30,   348,    -1,   466,
      -1,    63,   348,    -1,    62,   348,    -1,    61,   348,    -1,
      60,   348,    -1,    59,   348,    -1,    58,   348,    -1,    57,
     348,    -1,    70,   406,    -1,    56,   348,    -1,   412,    -1,
     367,    -1,   366,    -1,   206,   407,   206,    -1,    13,   348,
      -1,   370,    -1,   113,   200,   391,   415,   201,    -1,    -1,
      -1,   250,   249,   200,   352,   291,   201,   479,   350,   203,
     232,   204,    -1,    -1,   329,   250,   249,   200,   353,   291,
     201,   479,   350,   203,   232,   204,    -1,    -1,    80,   355,
     360,    -1,    -1,   182,    80,   356,   360,    -1,    -1,   197,
     357,   291,   198,   479,   360,    -1,    -1,   182,   197,   358,
     291,   198,   479,   360,    -1,    -1,   182,   203,   359,   232,
     204,    -1,     8,   348,    -1,     8,   345,    -1,     8,   203,
     232,   204,    -1,    87,    -1,   468,    -1,   362,     9,   361,
     132,   348,    -1,   361,   132,   348,    -1,   363,     9,   361,
     132,   410,    -1,   361,   132,   410,    -1,   362,   414,    -1,
      -1,   363,   414,    -1,    -1,   176,   200,   364,   201,    -1,
     134,   200,   457,   201,    -1,    67,   457,   207,    -1,   403,
     203,   459,   204,    -1,   403,   203,   461,   204,    -1,   370,
      67,   453,   207,    -1,   371,    67,   453,   207,    -1,   367,
      -1,   468,    -1,   446,    -1,    87,    -1,   200,   349,   201,
      -1,   374,   375,    -1,   443,    14,   372,    -1,   183,    80,
     186,   348,    -1,   376,   387,    -1,   376,   387,   390,    -1,
     387,    -1,   387,   390,    -1,   377,    -1,   376,   377,    -1,
     378,    -1,   379,    -1,   380,    -1,   381,    -1,   382,    -1,
     383,    -1,   183,    80,   186,   348,    -1,   190,    80,    14,
     348,    -1,   184,   348,    -1,   185,    80,   186,   348,   187,
     348,   188,   348,    -1,   185,    80,   186,   348,   187,   348,
     188,   348,   189,    80,    -1,   191,   384,    -1,   385,    -1,
     384,     9,   385,    -1,   348,    -1,   348,   386,    -1,   192,
      -1,   193,    -1,   388,    -1,   389,    -1,   194,   348,    -1,
     195,   348,   196,   348,    -1,   189,    80,   375,    -1,   391,
       9,    80,    -1,   391,     9,    35,    80,    -1,    80,    -1,
      35,    80,    -1,   170,   156,   393,   171,    -1,   395,    51,
      -1,   395,   171,   396,   170,    51,   394,    -1,    -1,   156,
      -1,   395,   397,    14,   398,    -1,    -1,   396,   399,    -1,
      -1,   156,    -1,   157,    -1,   203,   348,   204,    -1,   157,
      -1,   203,   348,   204,    -1,   392,    -1,   401,    -1,   400,
      30,   401,    -1,   400,    48,   401,    -1,   217,    -1,    70,
      -1,   107,    -1,   108,    -1,   109,    -1,    27,    -1,    28,
      -1,   110,    -1,   111,    -1,   169,    -1,   112,    -1,    71,
      -1,    72,    -1,    74,    -1,    73,    -1,    90,    -1,    91,
      -1,    89,    -1,    92,    -1,    93,    -1,    94,    -1,    95,
      -1,    96,    -1,    97,    -1,    54,    -1,    98,    -1,   100,
      -1,   101,    -1,   102,    -1,   103,    -1,   104,    -1,   106,
      -1,   105,    -1,    88,    -1,    13,    -1,   126,    -1,   127,
      -1,   128,    -1,   129,    -1,    69,    -1,    68,    -1,   121,
      -1,     5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,
      -1,   152,    -1,   113,    -1,   114,    -1,   123,    -1,   124,
      -1,   125,    -1,   120,    -1,   119,    -1,   118,    -1,   117,
      -1,   116,    -1,   115,    -1,   182,    -1,   122,    -1,   133,
      -1,   134,    -1,    10,    -1,    12,    -1,    11,    -1,   136,
      -1,   138,    -1,   137,    -1,   139,    -1,   140,    -1,   154,
      -1,   153,    -1,   181,    -1,   164,    -1,   167,    -1,   166,
      -1,   177,    -1,   179,    -1,   176,    -1,   229,   200,   293,
     201,    -1,   230,    -1,   156,    -1,   403,    -1,   409,    -1,
     120,    -1,   451,    -1,   200,   349,   201,    -1,   404,   151,
     452,    -1,   403,    -1,   120,    -1,   454,    -1,   200,   201,
      -1,   339,    -1,    -1,    -1,    86,    -1,   463,    -1,   200,
     293,   201,    -1,    -1,    75,    -1,    76,    -1,    77,    -1,
      87,    -1,   139,    -1,   140,    -1,   154,    -1,   136,    -1,
     167,    -1,   137,    -1,   138,    -1,   153,    -1,   181,    -1,
     147,    86,   148,    -1,   147,   148,    -1,   409,    -1,   228,
      -1,   134,   200,   413,   201,    -1,    67,   413,   207,    -1,
     176,   200,   365,   201,    -1,   411,    -1,   369,    -1,   200,
     410,   201,    -1,   410,    31,   410,    -1,   410,    32,   410,
      -1,   410,    10,   410,    -1,   410,    12,   410,    -1,   410,
      11,   410,    -1,   410,    33,   410,    -1,   410,    35,   410,
      -1,   410,    34,   410,    -1,   410,    49,   410,    -1,   410,
      47,   410,    -1,   410,    48,   410,    -1,   410,    50,   410,
      -1,   410,    51,   410,    -1,   410,    52,   410,    -1,   410,
      46,   410,    -1,   410,    45,   410,    -1,   410,    66,   410,
      -1,    53,   410,    -1,    55,   410,    -1,    47,   410,    -1,
      48,   410,    -1,   410,    37,   410,    -1,   410,    36,   410,
      -1,   410,    39,   410,    -1,   410,    38,   410,    -1,   410,
      40,   410,    -1,   410,    44,   410,    -1,   410,    41,   410,
      -1,   410,    43,   410,    -1,   410,    42,   410,    -1,   410,
      29,   410,    30,   410,    -1,   410,    29,    30,   410,    -1,
     230,   151,   218,    -1,   156,   151,   218,    -1,   230,   151,
     126,    -1,   228,    -1,    79,    -1,   468,    -1,   409,    -1,
     208,   463,   208,    -1,   209,   463,   209,    -1,   147,   463,
     148,    -1,   416,   414,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   416,     9,   410,   132,   410,    -1,   416,     9,
     410,    -1,   410,   132,   410,    -1,   410,    -1,    75,    -1,
      76,    -1,    77,    -1,   147,    86,   148,    -1,   147,   148,
      -1,    75,    -1,    76,    -1,    77,    -1,   217,    -1,    87,
      -1,    87,    49,   419,    -1,   417,    -1,   419,    -1,   217,
      -1,    47,   418,    -1,    48,   418,    -1,   134,   200,   421,
     201,    -1,    67,   421,   207,    -1,   176,   200,   424,   201,
      -1,   422,   414,    -1,    -1,   422,     9,   420,   132,   420,
      -1,   422,     9,   420,    -1,   420,   132,   420,    -1,   420,
      -1,   423,     9,   420,    -1,   420,    -1,   425,   414,    -1,
      -1,   425,     9,   361,   132,   420,    -1,   361,   132,   420,
      -1,   423,   414,    -1,    -1,   200,   426,   201,    -1,    -1,
     428,     9,   217,   427,    -1,   217,   427,    -1,    -1,   430,
     428,   414,    -1,    46,   429,    45,    -1,   431,    -1,    -1,
     130,    -1,   131,    -1,   217,    -1,   156,    -1,   203,   348,
     204,    -1,   434,    -1,   452,    -1,   217,    -1,   203,   348,
     204,    -1,   436,    -1,   452,    -1,    67,   453,   207,    -1,
     203,   348,   204,    -1,   444,   438,    -1,   200,   338,   201,
     438,    -1,   455,   438,    -1,   200,   338,   201,   438,    -1,
     200,   338,   201,   433,   435,    -1,   200,   349,   201,   433,
     435,    -1,   200,   338,   201,   433,   434,    -1,   200,   349,
     201,   433,   434,    -1,   450,    -1,   402,    -1,   448,    -1,
     449,    -1,   439,    -1,   441,    -1,   443,   433,   435,    -1,
     404,   151,   452,    -1,   445,   200,   293,   201,    -1,   446,
     200,   293,   201,    -1,   200,   443,   201,    -1,   402,    -1,
     448,    -1,   449,    -1,   439,    -1,   443,   433,   435,    -1,
     442,    -1,   445,   200,   293,   201,    -1,   200,   443,   201,
      -1,   404,   151,   452,    -1,   450,    -1,   439,    -1,   402,
      -1,   367,    -1,   409,    -1,   200,   443,   201,    -1,   200,
     349,   201,    -1,   446,   200,   293,   201,    -1,   445,   200,
     293,   201,    -1,   200,   447,   201,    -1,   351,    -1,   354,
      -1,   443,   433,   437,   475,   200,   293,   201,    -1,   200,
     338,   201,   433,   437,   475,   200,   293,   201,    -1,   404,
     151,   219,   475,   200,   293,   201,    -1,   404,   151,   452,
     200,   293,   201,    -1,   404,   151,   203,   348,   204,   200,
     293,   201,    -1,   451,    -1,   451,    67,   453,   207,    -1,
     451,   203,   348,   204,    -1,   452,    -1,    80,    -1,   205,
     203,   348,   204,    -1,   205,   452,    -1,   348,    -1,    -1,
     450,    -1,   440,    -1,   441,    -1,   454,   433,   435,    -1,
     404,   151,   450,    -1,   200,   443,   201,    -1,    -1,   440,
      -1,   442,    -1,   454,   433,   434,    -1,   200,   443,   201,
      -1,   456,     9,    -1,   456,     9,   443,    -1,   456,     9,
     133,   200,   456,   201,    -1,    -1,   443,    -1,   133,   200,
     456,   201,    -1,   458,   414,    -1,    -1,   458,     9,   348,
     132,   348,    -1,   458,     9,   348,    -1,   348,   132,   348,
      -1,   348,    -1,   458,     9,   348,   132,    35,   443,    -1,
     458,     9,    35,   443,    -1,   348,   132,    35,   443,    -1,
      35,   443,    -1,   460,   414,    -1,    -1,   460,     9,   348,
     132,   348,    -1,   460,     9,   348,    -1,   348,   132,   348,
      -1,   348,    -1,   462,   414,    -1,    -1,   462,     9,   410,
     132,   410,    -1,   462,     9,   410,    -1,   410,   132,   410,
      -1,   410,    -1,   463,   464,    -1,   463,    86,    -1,   464,
      -1,    86,   464,    -1,    80,    -1,    80,    67,   465,   207,
      -1,    80,   433,   217,    -1,   149,   348,   204,    -1,   149,
      79,    67,   348,   207,   204,    -1,   150,   443,   204,    -1,
     217,    -1,    81,    -1,    80,    -1,   123,   200,   340,   201,
      -1,   124,   200,   443,   201,    -1,   124,   200,   349,   201,
      -1,   124,   200,   447,   201,    -1,   124,   200,   446,   201,
      -1,   124,   200,   338,   201,    -1,     7,   348,    -1,     6,
     348,    -1,     5,   200,   348,   201,    -1,     4,   348,    -1,
       3,   348,    -1,   443,    -1,   467,     9,   443,    -1,   404,
     151,   218,    -1,   404,   151,   126,    -1,    -1,    98,   489,
      -1,   177,   474,    14,   489,   202,    -1,   431,   177,   474,
      14,   489,   202,    -1,   179,   474,   469,    14,   489,   202,
      -1,   431,   179,   474,   469,    14,   489,   202,    -1,   219,
      -1,   489,   219,    -1,   218,    -1,   489,   218,    -1,   219,
      -1,   219,   172,   481,   173,    -1,   217,    -1,   217,   172,
     481,   173,    -1,   172,   477,   173,    -1,    -1,   489,    -1,
     476,     9,   489,    -1,   476,   414,    -1,   476,     9,   165,
      -1,   477,    -1,   165,    -1,    -1,    -1,    30,   489,    -1,
      98,   489,    -1,    99,   489,    -1,   481,     9,   482,   217,
      -1,   482,   217,    -1,   481,     9,   482,   217,   480,    -1,
     482,   217,   480,    -1,    47,    -1,    48,    -1,    -1,    87,
     132,   489,    -1,    29,    87,   132,   489,    -1,   230,   151,
     217,   132,   489,    -1,   484,     9,   483,    -1,   483,    -1,
     484,   414,    -1,    -1,   176,   200,   485,   201,    -1,   230,
      -1,   217,   151,   488,    -1,   217,   475,    -1,    29,   489,
      -1,    56,   489,    -1,   230,    -1,   134,    -1,   135,    -1,
     486,    -1,   487,   151,   488,    -1,   134,   172,   489,   173,
      -1,   134,   172,   489,     9,   489,   173,    -1,   156,    -1,
     200,   107,   200,   478,   201,    30,   489,   201,    -1,   200,
     489,     9,   476,   414,   201,    -1,   489,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   740,   740,   740,   749,   751,   754,   755,   756,   757,
     758,   759,   760,   763,   765,   765,   767,   767,   769,   770,
     772,   774,   779,   780,   781,   782,   783,   784,   785,   786,
     787,   788,   789,   790,   791,   792,   793,   794,   795,   796,
     797,   798,   802,   803,   804,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,   815,   816,   817,   818,   819,
     820,   821,   822,   823,   824,   825,   826,   827,   828,   829,
     830,   831,   832,   833,   834,   835,   836,   837,   838,   839,
     840,   841,   842,   843,   844,   845,   846,   847,   848,   849,
     850,   851,   852,   853,   854,   855,   856,   857,   858,   859,
     860,   861,   862,   869,   870,   874,   876,   880,   882,   886,
     888,   892,   893,   894,   895,   900,   901,   902,   903,   908,
     909,   910,   911,   916,   917,   921,   922,   924,   927,   933,
     940,   947,   951,   957,   959,   962,   963,   964,   965,   968,
     969,   973,   978,   978,   984,   984,   991,   990,   996,   996,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1019,  1017,  1026,  1024,  1031,
    1039,  1033,  1043,  1041,  1045,  1046,  1050,  1051,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1068,  1068,  1073,
    1079,  1083,  1083,  1091,  1092,  1096,  1097,  1101,  1107,  1105,
    1120,  1117,  1133,  1130,  1147,  1146,  1155,  1153,  1165,  1164,
    1183,  1181,  1200,  1199,  1208,  1206,  1218,  1217,  1229,  1227,
    1240,  1241,  1245,  1248,  1251,  1252,  1253,  1256,  1257,  1260,
    1262,  1265,  1266,  1269,  1270,  1273,  1274,  1278,  1279,  1284,
    1285,  1288,  1289,  1290,  1294,  1295,  1299,  1300,  1304,  1305,
    1309,  1310,  1315,  1316,  1321,  1322,  1323,  1324,  1327,  1330,
    1332,  1335,  1336,  1340,  1342,  1345,  1348,  1351,  1352,  1355,
    1356,  1360,  1366,  1372,  1379,  1381,  1386,  1391,  1397,  1401,
    1405,  1409,  1414,  1419,  1424,  1429,  1435,  1444,  1449,  1454,
    1460,  1462,  1466,  1470,  1475,  1479,  1482,  1485,  1489,  1493,
    1497,  1501,  1506,  1514,  1516,  1519,  1520,  1521,  1522,  1524,
    1526,  1531,  1532,  1535,  1536,  1537,  1541,  1542,  1544,  1545,
    1549,  1551,  1554,  1558,  1564,  1566,  1569,  1569,  1573,  1572,
    1576,  1578,  1581,  1584,  1582,  1598,  1594,  1608,  1610,  1612,
    1614,  1616,  1618,  1620,  1624,  1625,  1626,  1629,  1635,  1639,
    1645,  1648,  1653,  1655,  1660,  1665,  1669,  1670,  1674,  1675,
    1677,  1679,  1685,  1686,  1688,  1692,  1693,  1698,  1702,  1703,
    1707,  1708,  1712,  1714,  1720,  1725,  1726,  1728,  1732,  1733,
    1734,  1735,  1739,  1740,  1741,  1742,  1743,  1744,  1746,  1751,
    1754,  1755,  1759,  1760,  1764,  1765,  1768,  1769,  1772,  1773,
    1776,  1777,  1781,  1782,  1783,  1784,  1785,  1786,  1787,  1791,
    1792,  1795,  1796,  1797,  1800,  1802,  1804,  1805,  1808,  1810,
    1814,  1816,  1820,  1824,  1828,  1833,  1834,  1836,  1837,  1838,
    1841,  1845,  1846,  1850,  1851,  1855,  1856,  1857,  1861,  1865,
    1870,  1874,  1878,  1883,  1884,  1885,  1886,  1887,  1891,  1893,
    1894,  1895,  1898,  1899,  1900,  1901,  1902,  1903,  1904,  1905,
    1906,  1907,  1908,  1909,  1910,  1911,  1912,  1913,  1914,  1915,
    1916,  1917,  1918,  1919,  1920,  1921,  1922,  1923,  1924,  1925,
    1926,  1927,  1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,
    1936,  1937,  1938,  1939,  1940,  1942,  1943,  1945,  1946,  1948,
    1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,  1957,  1958,
    1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,  1970,  1974,
    1979,  1978,  1993,  1991,  2008,  2008,  2024,  2023,  2041,  2041,
    2057,  2056,  2075,  2074,  2095,  2096,  2097,  2102,  2104,  2108,
    2112,  2118,  2122,  2128,  2130,  2134,  2136,  2140,  2144,  2145,
    2149,  2156,  2163,  2165,  2170,  2171,  2172,  2173,  2175,  2179,
    2183,  2187,  2191,  2193,  2195,  2197,  2202,  2203,  2208,  2209,
    2210,  2211,  2212,  2213,  2217,  2221,  2225,  2229,  2234,  2239,
    2243,  2244,  2248,  2249,  2253,  2254,  2258,  2259,  2263,  2267,
    2271,  2275,  2276,  2277,  2278,  2282,  2288,  2297,  2310,  2311,
    2314,  2317,  2320,  2321,  2324,  2328,  2331,  2334,  2341,  2342,
    2346,  2347,  2349,  2353,  2354,  2355,  2356,  2357,  2358,  2359,
    2360,  2361,  2362,  2363,  2364,  2365,  2366,  2367,  2368,  2369,
    2370,  2371,  2372,  2373,  2374,  2375,  2376,  2377,  2378,  2379,
    2380,  2381,  2382,  2383,  2384,  2385,  2386,  2387,  2388,  2389,
    2390,  2391,  2392,  2393,  2394,  2395,  2396,  2397,  2398,  2399,
    2400,  2401,  2402,  2403,  2404,  2405,  2406,  2407,  2408,  2409,
    2410,  2411,  2412,  2413,  2414,  2415,  2416,  2417,  2418,  2419,
    2420,  2421,  2422,  2423,  2424,  2425,  2426,  2427,  2428,  2429,
    2430,  2431,  2432,  2436,  2441,  2442,  2445,  2446,  2447,  2448,
    2450,  2452,  2463,  2464,  2465,  2469,  2470,  2471,  2475,  2476,
    2477,  2480,  2482,  2486,  2487,  2488,  2489,  2491,  2492,  2493,
    2494,  2495,  2496,  2497,  2498,  2499,  2500,  2503,  2508,  2509,
    2510,  2512,  2513,  2515,  2516,  2517,  2518,  2520,  2522,  2524,
    2526,  2528,  2529,  2530,  2531,  2532,  2533,  2534,  2535,  2536,
    2537,  2538,  2539,  2540,  2541,  2542,  2543,  2544,  2546,  2548,
    2550,  2552,  2553,  2556,  2557,  2561,  2565,  2567,  2571,  2574,
    2577,  2583,  2584,  2585,  2586,  2587,  2588,  2589,  2594,  2596,
    2600,  2601,  2604,  2605,  2609,  2612,  2614,  2616,  2620,  2621,
    2622,  2623,  2626,  2630,  2631,  2632,  2633,  2637,  2639,  2646,
    2647,  2648,  2649,  2650,  2651,  2653,  2654,  2659,  2661,  2664,
    2667,  2669,  2671,  2674,  2676,  2680,  2682,  2685,  2688,  2694,
    2696,  2699,  2700,  2705,  2708,  2712,  2712,  2717,  2720,  2721,
    2725,  2726,  2730,  2731,  2732,  2736,  2741,  2746,  2747,  2751,
    2756,  2761,  2762,  2766,  2767,  2772,  2774,  2779,  2790,  2804,
    2816,  2831,  2832,  2833,  2834,  2835,  2836,  2837,  2847,  2856,
    2858,  2860,  2864,  2865,  2866,  2867,  2868,  2884,  2885,  2887,
    2889,  2896,  2897,  2898,  2899,  2900,  2901,  2902,  2903,  2905,
    2910,  2914,  2915,  2919,  2922,  2929,  2933,  2942,  2949,  2957,
    2959,  2960,  2964,  2965,  2967,  2972,  2973,  2984,  2985,  2986,
    2987,  2998,  3001,  3004,  3005,  3006,  3007,  3018,  3022,  3023,
    3024,  3026,  3027,  3028,  3032,  3034,  3037,  3039,  3040,  3041,
    3042,  3045,  3047,  3048,  3052,  3054,  3057,  3059,  3060,  3061,
    3065,  3067,  3070,  3073,  3075,  3077,  3081,  3082,  3084,  3085,
    3091,  3092,  3094,  3104,  3106,  3108,  3111,  3112,  3113,  3117,
    3118,  3119,  3120,  3121,  3122,  3123,  3124,  3125,  3126,  3127,
    3131,  3132,  3136,  3138,  3146,  3148,  3152,  3156,  3161,  3165,
    3173,  3174,  3178,  3179,  3185,  3186,  3195,  3196,  3204,  3207,
    3211,  3214,  3219,  3224,  3226,  3227,  3228,  3232,  3233,  3237,
    3238,  3241,  3244,  3246,  3250,  3256,  3257,  3258,  3262,  3266,
    3276,  3284,  3286,  3290,  3292,  3297,  3303,  3306,  3311,  3319,
    3322,  3325,  3326,  3329,  3332,  3333,  3338,  3341,  3345,  3349,
    3355,  3365,  3366
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
  "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'", "'^'", "'&'",
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
  "T_COMPILER_HALT_OFFSET", "T_ASYNC", "T_FROM", "T_WHERE", "T_JOIN",
  "T_IN", "T_ON", "T_EQUALS", "T_INTO", "T_LET", "T_ORDERBY",
  "T_ASCENDING", "T_DESCENDING", "T_SELECT", "T_GROUP", "T_BY",
  "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP", "'('", "')'", "';'",
  "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept", "start",
  "$@1", "top_statement_list", "top_statement", "$@2", "$@3",
  "ident_no_semireserved", "ident_for_class_const", "ident",
  "use_declarations", "use_fn_declarations", "use_const_declarations",
  "use_declaration", "use_fn_declaration", "use_const_declaration",
  "namespace_name", "namespace_string_base", "namespace_string",
  "namespace_string_typeargs", "class_namespace_string_typeargs",
  "constant_declaration", "inner_statement_list", "inner_statement",
  "statement", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11",
  "try_statement_list", "$@12", "additional_catches",
  "finally_statement_list", "$@13", "optional_finally", "is_reference",
  "function_loc", "function_declaration_statement", "$@14", "$@15", "$@16",
  "enum_declaration_statement", "$@17", "$@18",
  "class_declaration_statement", "$@19", "$@20", "$@21", "$@22",
  "trait_declaration_statement", "$@23", "$@24", "class_decl_name",
  "interface_decl_name", "trait_decl_name", "class_entry_type",
  "extends_from", "implements_list", "interface_extends_list",
  "interface_list", "trait_list", "foreach_optional_arg",
  "foreach_variable", "for_statement", "foreach_statement",
  "while_statement", "declare_statement", "declare_list",
  "switch_case_list", "case_list", "case_separator", "elseif_list",
  "new_elseif_list", "else_single", "new_else_single",
  "method_parameter_list", "non_empty_method_parameter_list",
  "parameter_list", "non_empty_parameter_list",
  "function_call_parameter_list", "non_empty_fcall_parameter_list",
  "global_var_list", "global_var", "static_var_list",
  "enum_statement_list", "enum_statement", "enum_constant_declaration",
  "class_statement_list", "class_statement", "$@25", "$@26", "$@27",
  "$@28", "trait_rules", "trait_precedence_rule", "trait_alias_rule",
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
  "closure_expression", "$@29", "$@30", "lambda_expression", "$@31",
  "$@32", "$@33", "$@34", "$@35", "lambda_body", "shape_keyname",
  "non_empty_shape_pair_list", "non_empty_static_shape_pair_list",
  "shape_pair_list", "static_shape_pair_list", "shape_literal",
  "array_literal", "collection_literal", "static_collection_literal",
  "dim_expr", "dim_expr_base", "query_expr", "query_assign_expr",
  "query_head", "query_body", "query_body_clauses", "query_body_clause",
  "from_clause", "let_clause", "where_clause", "join_clause",
  "join_into_clause", "orderby_clause", "orderings", "ordering",
  "ordering_direction", "select_or_group_clause", "select_clause",
  "group_clause", "query_continuation", "lexical_var_list", "xhp_tag",
  "xhp_tag_body", "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
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
  "user_attribute_list", "$@36", "non_empty_user_attributes",
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
      58,   282,   283,   124,    94,    38,   284,   285,   286,   287,
      60,    62,   288,   289,   290,   291,   292,    43,    45,    46,
      42,    47,    37,    33,   293,   126,    64,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,    91,   304,   305,
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
     416,   417,   418,   419,   420,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
      40,    41,    59,   123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   210,   212,   211,   213,   213,   214,   214,   214,   214,
     214,   214,   214,   214,   215,   214,   216,   214,   214,   214,
     214,   214,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   219,   219,   220,   220,   221,   221,   222,
     222,   223,   223,   223,   223,   224,   224,   224,   224,   225,
     225,   225,   225,   226,   226,   227,   227,   227,   228,   229,
     230,   231,   231,   232,   232,   233,   233,   233,   233,   234,
     234,   234,   235,   234,   236,   234,   237,   234,   238,   234,
     234,   234,   234,   234,   234,   234,   234,   234,   234,   234,
     234,   234,   234,   234,   234,   239,   234,   240,   234,   234,
     241,   234,   242,   234,   234,   234,   234,   234,   234,   234,
     234,   234,   234,   234,   234,   234,   234,   244,   243,   245,
     245,   247,   246,   248,   248,   249,   249,   250,   252,   251,
     253,   251,   254,   251,   256,   255,   257,   255,   259,   258,
     260,   258,   261,   258,   262,   258,   264,   263,   265,   263,
     266,   266,   267,   268,   269,   269,   269,   269,   269,   270,
     270,   271,   271,   272,   272,   273,   273,   274,   274,   275,
     275,   276,   276,   276,   277,   277,   278,   278,   279,   279,
     280,   280,   281,   281,   282,   282,   282,   282,   283,   283,
     283,   284,   284,   285,   285,   286,   286,   287,   287,   288,
     288,   289,   289,   289,   289,   289,   289,   289,   289,   290,
     290,   290,   290,   290,   290,   290,   290,   291,   291,   291,
     291,   291,   291,   291,   291,   292,   292,   292,   292,   292,
     292,   292,   292,   293,   293,   294,   294,   294,   294,   294,
     294,   295,   295,   296,   296,   296,   297,   297,   297,   297,
     298,   298,   299,   300,   301,   301,   303,   302,   304,   302,
     302,   302,   302,   305,   302,   306,   302,   302,   302,   302,
     302,   302,   302,   302,   307,   307,   307,   308,   309,   309,
     310,   310,   311,   311,   312,   312,   313,   313,   314,   314,
     314,   314,   314,   314,   314,   315,   315,   316,   317,   317,
     318,   318,   319,   319,   320,   321,   321,   321,   322,   322,
     322,   322,   323,   323,   323,   323,   323,   323,   323,   324,
     324,   324,   325,   325,   326,   326,   327,   327,   328,   328,
     329,   329,   330,   330,   330,   330,   330,   330,   330,   331,
     331,   332,   332,   332,   333,   333,   333,   333,   334,   334,
     335,   335,   336,   336,   337,   338,   338,   338,   338,   338,
     339,   340,   340,   341,   341,   342,   342,   342,   343,   344,
     345,   346,   347,   348,   348,   348,   348,   348,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   350,   350,
     352,   351,   353,   351,   355,   354,   356,   354,   357,   354,
     358,   354,   359,   354,   360,   360,   360,   361,   361,   362,
     362,   363,   363,   364,   364,   365,   365,   366,   367,   367,
     368,   369,   370,   370,   371,   371,   371,   371,   371,   372,
     373,   374,   375,   375,   375,   375,   376,   376,   377,   377,
     377,   377,   377,   377,   378,   379,   380,   381,   382,   383,
     384,   384,   385,   385,   386,   386,   387,   387,   388,   389,
     390,   391,   391,   391,   391,   392,   393,   393,   394,   394,
     395,   395,   396,   396,   397,   398,   398,   399,   399,   399,
     400,   400,   400,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   402,   403,   403,   404,   404,   404,   404,
     404,   404,   405,   405,   405,   406,   406,   406,   407,   407,
     407,   408,   408,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   410,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   411,   411,
     411,   412,   412,   412,   412,   412,   412,   412,   413,   413,
     414,   414,   415,   415,   416,   416,   416,   416,   417,   417,
     417,   417,   417,   418,   418,   418,   418,   419,   419,   420,
     420,   420,   420,   420,   420,   420,   420,   421,   421,   422,
     422,   422,   422,   423,   423,   424,   424,   425,   425,   426,
     426,   427,   427,   428,   428,   430,   429,   431,   432,   432,
     433,   433,   434,   434,   434,   435,   435,   436,   436,   437,
     437,   438,   438,   439,   439,   440,   440,   441,   441,   442,
     442,   443,   443,   443,   443,   443,   443,   443,   443,   443,
     443,   443,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   445,   445,   445,   445,   445,   445,   445,   445,   445,
     446,   447,   447,   448,   448,   449,   449,   449,   450,   451,
     451,   451,   452,   452,   452,   453,   453,   454,   454,   454,
     454,   454,   454,   455,   455,   455,   455,   455,   456,   456,
     456,   456,   456,   456,   457,   457,   458,   458,   458,   458,
     458,   458,   458,   458,   459,   459,   460,   460,   460,   460,
     461,   461,   462,   462,   462,   462,   463,   463,   463,   463,
     464,   464,   464,   464,   464,   464,   465,   465,   465,   466,
     466,   466,   466,   466,   466,   466,   466,   466,   466,   466,
     467,   467,   468,   468,   469,   469,   470,   470,   470,   470,
     471,   471,   472,   472,   473,   473,   474,   474,   475,   475,
     476,   476,   477,   478,   478,   478,   478,   479,   479,   480,
     480,   481,   481,   481,   481,   482,   482,   482,   483,   483,
     483,   484,   484,   485,   485,   486,   487,   488,   488,   489,
     489,   489,   489,   489,   489,   489,   489,   489,   489,   489,
     489,   490,   490
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     3,
       1,     1,     2,     3,     4,     1,     2,     3,     4,     1,
       2,     3,     4,     1,     3,     1,     3,     2,     1,     2,
       2,     5,     4,     2,     0,     1,     1,     1,     1,     3,
       5,     8,     0,     4,     0,     6,     0,    10,     0,     4,
       2,     3,     2,     3,     2,     3,     3,     3,     3,     3,
       3,     5,     1,     1,     1,     0,     9,     0,    10,     5,
       0,    13,     0,     5,     3,     3,     2,     2,     2,     2,
       2,     2,     3,     2,     2,     3,     2,     0,     4,     9,
       0,     0,     4,     2,     0,     1,     0,     1,     0,     9,
       0,    10,     0,    11,     0,     9,     0,    10,     0,     8,
       0,     9,     0,     7,     0,     8,     0,     7,     0,     8,
       1,     1,     1,     1,     1,     2,     3,     3,     2,     2,
       0,     2,     0,     2,     0,     1,     3,     1,     3,     2,
       0,     1,     2,     4,     1,     4,     1,     4,     1,     4,
       1,     4,     3,     5,     3,     4,     4,     5,     5,     4,
       0,     1,     1,     4,     0,     5,     0,     2,     0,     3,
       0,     7,     8,     6,     2,     5,     6,     4,     0,     4,
       5,     7,     6,     6,     7,     9,     8,     6,     7,     5,
       2,     4,     5,     3,     0,     3,     4,     6,     5,     5,
       6,     8,     7,     2,     0,     1,     2,     2,     3,     4,
       4,     3,     1,     1,     2,     4,     3,     5,     1,     3,
       2,     0,     2,     3,     2,     0,     0,     4,     0,     5,
       2,     2,     2,     0,    10,     0,    11,     3,     3,     3,
       4,     4,     3,     5,     2,     2,     0,     6,     5,     4,
       3,     1,     1,     3,     4,     1,     2,     1,     1,     4,
       6,     1,     1,     4,     1,     1,     3,     2,     2,     0,
       2,     0,     1,     3,     1,     1,     1,     1,     3,     4,
       4,     4,     1,     1,     2,     2,     2,     3,     3,     1,
       1,     1,     1,     3,     1,     3,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     3,     5,     1,     3,     5,     4,
       3,     3,     3,     4,     3,     3,     3,     2,     1,     1,
       3,     3,     1,     1,     0,     1,     2,     4,     3,     6,
       2,     3,     6,     1,     1,     1,     1,     1,     6,     3,
       4,     6,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       5,     4,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     1,     1,     3,     2,     1,     5,     0,
       0,    11,     0,    12,     0,     3,     0,     4,     0,     6,
       0,     7,     0,     5,     2,     2,     4,     1,     1,     5,
       3,     5,     3,     2,     0,     2,     0,     4,     4,     3,
       4,     4,     4,     4,     1,     1,     1,     1,     3,     2,
       3,     4,     2,     3,     1,     2,     1,     2,     1,     1,
       1,     1,     1,     1,     4,     4,     2,     8,    10,     2,
       1,     3,     1,     2,     1,     1,     1,     1,     2,     4,
       3,     3,     4,     1,     2,     4,     2,     6,     0,     1,
       4,     0,     2,     0,     1,     1,     3,     1,     3,     1,
       1,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     1,     1,     1,     1,     1,     1,
       3,     3,     1,     1,     1,     2,     1,     0,     0,     1,
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
       0,   435,     0,   825,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   915,     0,
     903,   707,     0,   713,   714,   715,    22,   772,   892,   163,
     164,   716,     0,   144,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   197,     0,     0,     0,     0,     0,     0,
     402,   403,   404,   407,   406,   405,     0,     0,     0,     0,
     224,     0,     0,     0,   720,   722,   723,   717,   718,     0,
       0,     0,   724,   719,     0,   695,    24,    25,    26,    28,
      27,     0,   721,     0,     0,     0,     0,   725,   408,    29,
      30,    32,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,   528,     0,   162,   134,     0,   708,     0,     0,
       4,   123,   125,   128,   771,     0,   694,     0,     6,   196,
       7,     9,     8,    10,     0,     0,   400,   445,     0,     0,
       0,     0,     0,     0,     0,   443,   881,   882,   514,   513,
     429,   517,     0,     0,   428,   852,   696,     0,   774,   512,
     399,   855,   856,   867,   444,     0,     0,   447,   446,   853,
     854,   851,   888,   891,   502,   773,    11,   407,   406,   405,
       0,     0,    28,   123,   196,     0,   959,   444,   958,     0,
     956,   955,   516,     0,   436,   440,     0,     0,   485,   486,
     487,   488,   511,   509,   508,   507,   506,   505,   504,   503,
     892,   716,   698,     0,     0,   979,   874,   696,     0,   697,
     467,     0,   465,     0,   919,     0,   781,   427,   703,     0,
     979,   702,     0,   712,   697,   898,   899,   905,   897,   704,
       0,     0,   706,   510,     0,     0,     0,     0,   432,     0,
     142,   434,     0,     0,   148,   150,     0,     0,   152,     0,
      82,    81,    76,    75,    67,    68,    59,    79,    90,     0,
      62,     0,    74,    66,    72,    92,    85,    84,    57,    80,
      99,   100,    58,    95,    55,    96,    56,    97,    54,   101,
      89,    93,    98,    86,    87,    61,    88,    91,    53,    83,
      69,   102,    77,    70,    60,    52,    51,    50,    49,    48,
      47,    71,   104,    64,    45,    46,    73,  1012,  1013,    65,
    1018,    44,    63,    94,     0,     0,   123,   103,   970,  1011,
       0,  1014,     0,     0,     0,   154,     0,     0,     0,     0,
     187,     0,     0,     0,     0,     0,     0,   106,   111,   313,
       0,     0,   312,     0,   228,     0,   225,   318,     0,     0,
       0,     0,     0,   976,   212,   222,   911,   915,     0,   940,
       0,   727,     0,     0,     0,   938,     0,    16,     0,   127,
     204,   216,   223,   601,   544,     0,   964,   526,   530,   532,
     829,   445,     0,   443,   444,   446,     0,     0,   894,   709,
       0,   710,     0,     0,     0,   186,     0,     0,   130,   304,
       0,    21,   195,     0,   221,   208,   220,   405,   408,   196,
     401,   177,   178,   179,   180,   181,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   903,     0,   176,   896,   896,   184,   925,
       0,     0,     0,     0,     0,     0,     0,     0,   398,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   466,   464,   830,   831,     0,   896,     0,   843,
     304,   304,   896,     0,   911,     0,   196,     0,     0,   156,
       0,   827,   822,   781,     0,   445,   443,     0,   923,     0,
     549,   780,   914,   445,   443,   444,   130,     0,   304,   426,
       0,   845,   705,     0,   134,   264,     0,   525,     0,   159,
       0,     0,   433,     0,     0,     0,     0,     0,   151,   175,
     153,  1012,  1013,  1009,  1010,     0,  1004,     0,     0,     0,
       0,    78,    43,    65,    42,   971,     0,   182,   155,   185,
       0,     0,     0,     0,     0,     0,     0,   559,     0,   566,
     568,   569,   570,   571,   572,   573,   564,   586,   587,   134,
       0,   172,   174,     0,     0,   108,   115,     0,     0,   110,
     119,   112,     0,    18,     0,     0,   314,     0,   157,   227,
     226,     0,     0,   158,   960,     0,     0,   445,   443,   444,
     447,   446,     0,   997,   234,     0,   912,     0,     0,   160,
       0,     0,   726,   939,   772,     0,     0,   937,   777,   936,
     126,     5,    13,    14,     0,   232,     0,     0,   537,     0,
       0,   781,     0,     0,   699,   538,     0,     0,     0,     0,
     829,   134,     0,   783,   828,  1022,   425,   499,   861,   880,
     139,   133,   135,   136,   137,   138,   399,     0,   515,   775,
     776,   124,   781,     0,   980,     0,     0,     0,   783,   305,
       0,   520,   198,   230,     0,   470,   472,   471,     0,     0,
     468,   469,   473,   475,   474,   490,   489,   492,   491,   493,
     495,   497,   496,   494,   484,   483,   477,   478,   476,   479,
     480,   482,   498,   481,   895,     0,     0,   929,     0,   781,
     963,     0,   962,   979,   858,   214,   206,   218,     0,   964,
     210,   196,   435,     0,   438,   441,   449,   560,   463,   462,
     461,   460,   459,   458,   457,   456,   455,   454,   453,   452,
     833,     0,   832,   835,   857,   839,   979,   836,     0,     0,
       0,     0,     0,     0,     0,     0,   957,   437,   820,   824,
     780,   826,     0,   700,     0,   918,     0,   917,     0,   700,
     902,   901,   888,   891,     0,     0,   832,   835,   900,   836,
     430,   266,   268,   134,   535,   534,   431,     0,   134,   248,
     143,   434,     0,     0,     0,     0,     0,   260,   260,   149,
       0,     0,     0,     0,  1002,   781,     0,   986,     0,     0,
       0,     0,     0,   779,     0,   695,     0,     0,   128,   729,
     694,   734,     0,   728,   132,   733,   979,  1015,     0,     0,
     576,     0,     0,   582,   579,   580,   588,     0,   567,   562,
       0,   565,     0,     0,     0,   116,    19,     0,     0,   120,
      20,     0,     0,     0,   105,   113,     0,   311,   319,   316,
       0,     0,   949,   954,   951,   950,   953,   952,    12,   995,
     996,     0,     0,     0,     0,   911,   908,     0,   548,   948,
     947,   946,     0,   942,     0,   943,   945,     0,     5,     0,
       0,     0,   595,   596,   604,   603,     0,   443,     0,   780,
     543,   547,     0,     0,   965,     0,   527,     0,     0,   987,
     829,   290,  1021,     0,     0,   844,     0,   893,   780,   982,
     978,   306,   307,   693,   782,   303,     0,   829,     0,     0,
     232,   522,   200,   501,     0,   552,   553,     0,   550,   780,
     924,     0,     0,   304,   234,     0,   232,     0,     0,   230,
       0,   903,   450,     0,     0,   841,   842,   859,   860,   889,
     890,     0,     0,     0,   808,   788,   789,   790,   797,     0,
       0,     0,   801,   799,   800,   814,   781,     0,   822,   922,
     921,     0,     0,   846,   711,     0,   270,     0,     0,   140,
       0,     0,     0,     0,     0,     0,     0,   240,   241,   252,
       0,   134,   250,   169,   260,     0,   260,     0,     0,  1016,
       0,     0,     0,   780,  1003,  1005,   985,   781,   984,     0,
     781,   755,   756,   753,   754,   787,     0,   781,   779,     0,
     546,     0,     0,   931,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1008,   561,     0,     0,     0,   584,   585,   583,
       0,     0,   563,     0,   188,     0,   191,   173,     0,   107,
     117,     0,   109,   121,   114,   315,     0,   961,   161,   997,
     977,   992,   233,   235,   325,     0,     0,   909,     0,   941,
       0,    17,     0,   964,   231,   325,     0,     0,   700,   540,
       0,   963,   962,   701,   966,     0,   987,   533,     0,     0,
    1022,     0,   295,   293,   835,   847,   979,   835,   848,   981,
       0,     0,   308,   131,     0,   829,   229,     0,   829,     0,
     500,   928,   927,     0,   304,     0,     0,     0,     0,     0,
       0,   232,   202,   712,   834,   304,     0,   793,   794,   795,
     796,   802,   803,   812,     0,   781,     0,   808,     0,   792,
     816,   780,   819,   821,   823,     0,   916,   834,     0,     0,
       0,     0,   267,   536,   145,     0,   434,   240,   242,   911,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   254,
       0,     0,   998,     0,  1001,   780,     0,     0,     0,   731,
     780,   778,     0,   769,     0,   781,     0,   735,   770,   768,
     935,     0,   781,   738,   740,   739,     0,     0,   736,   737,
     741,   743,   742,   758,   757,   760,   759,   761,   763,   765,
     764,   762,   751,   750,   745,   746,   744,   747,   748,   749,
     752,  1007,   574,     0,   575,   581,   589,   590,     0,   134,
     118,   122,   317,     0,     0,     0,   994,     0,   399,   913,
     911,   439,   442,   448,     0,    15,     0,   399,   607,     0,
       0,   609,   602,   605,     0,   600,     0,   968,     0,   988,
     529,     0,   296,     0,     0,   291,     0,   310,   309,   987,
       0,   325,     0,   829,     0,   304,     0,   886,   325,   964,
     325,   967,     0,     0,     0,   451,     0,     0,   805,   780,
     807,   798,     0,   791,     0,     0,   781,   813,   920,     0,
     134,     0,   263,   249,     0,     0,     0,   239,   165,   253,
       0,     0,   256,     0,   261,   262,   134,   255,  1017,   999,
       0,   983,     0,  1020,   786,   785,   730,     0,   780,   545,
     732,     0,   551,   780,   930,   767,     0,     0,     0,     0,
     991,   989,   990,   236,     0,     0,     0,   406,   397,     0,
       0,     0,   213,   324,   326,     0,   396,     0,     0,     0,
     964,   399,     0,   944,   321,   217,   598,     0,     0,   539,
     531,     0,   299,   289,     0,   292,   298,   304,   519,   987,
     399,   987,     0,   926,     0,   885,   399,     0,   399,   969,
     325,   829,   883,   811,   810,   804,     0,   806,   780,   815,
     134,   269,   141,   146,   167,   243,     0,   251,   257,   134,
     259,  1000,     0,     0,   542,     0,   934,   933,   766,     0,
     134,   192,   993,     0,     0,     0,   972,     0,     0,     0,
     237,     0,   964,     0,   362,   358,   364,   695,    28,     0,
     352,     0,   357,   361,   374,     0,   372,   377,     0,   376,
       0,   375,     0,   196,   328,     0,   330,     0,   331,   332,
       0,     0,   910,     0,   599,   597,   608,   606,   300,     0,
       0,   287,   297,     0,     0,     0,     0,   209,   519,   987,
     887,   215,   321,   219,   399,     0,     0,   818,     0,   265,
       0,     0,   134,   246,   166,   258,  1019,   784,     0,     0,
       0,     0,     0,     0,   424,     0,   973,     0,   342,   346,
     421,   422,   356,     0,     0,     0,   337,   659,   658,   655,
     657,   656,   676,   678,   677,   647,   618,   619,   637,   653,
     652,   614,   624,   625,   627,   626,   646,   630,   628,   629,
     631,   632,   633,   634,   635,   636,   638,   639,   640,   641,
     642,   643,   645,   644,   615,   616,   617,   620,   621,   623,
     661,   662,   671,   670,   669,   668,   667,   666,   654,   673,
     663,   664,   665,   648,   649,   650,   651,   674,   675,   679,
     681,   680,   682,   683,   660,   685,   684,   687,   689,   688,
     622,   692,   690,   691,   686,   672,   613,   369,   610,     0,
     338,   390,   391,   389,   382,     0,   383,   339,   416,     0,
       0,     0,     0,   420,     0,   196,   205,   320,     0,     0,
       0,   288,   302,   884,     0,   134,   392,   134,   199,     0,
       0,     0,   211,   987,   809,     0,   134,   244,   147,   168,
       0,   541,   932,   577,   190,   340,   341,   419,   238,     0,
       0,   781,     0,   365,   353,     0,     0,     0,   371,   373,
       0,     0,   378,   385,   386,   384,     0,     0,   327,   974,
       0,     0,     0,   423,     0,   322,     0,   301,     0,   593,
     783,     0,     0,   134,   201,   207,     0,   817,     0,     0,
       0,   170,   343,   123,     0,   344,   345,     0,     0,   359,
     780,   367,   363,   368,   611,   612,     0,   354,   387,   388,
     380,   381,   379,   417,   414,   997,   333,   329,   418,     0,
     323,   594,   782,     0,   521,   393,     0,   203,     0,   247,
     578,     0,   194,     0,   399,     0,   366,   370,     0,     0,
     829,   335,     0,   591,   518,   523,   245,     0,     0,   171,
     350,     0,   398,   360,   415,   975,     0,   783,   410,   829,
     592,     0,   193,     0,     0,   349,   987,   829,   274,   411,
     412,   413,  1022,   409,     0,     0,     0,   348,     0,   410,
       0,   987,     0,   347,   394,   134,   334,  1022,     0,   279,
     277,     0,   134,     0,     0,   280,     0,     0,   275,   336,
       0,   395,     0,   283,   273,     0,   276,   282,   189,   284,
       0,     0,   271,   281,     0,   272,   286,   285
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   898,   631,   183,  1466,   328,
     346,   584,   588,   347,   585,   589,   122,   123,   124,   125,
     126,   127,   396,   661,   662,   531,   249,  1530,   537,  1446,
    1531,  1772,   854,   341,   579,  1731,  1087,  1269,  1789,   413,
     184,   663,   938,  1149,  1324,   131,   634,   955,   664,   683,
     959,   614,   954,   665,   635,   956,   415,   364,   381,   134,
     940,   901,   884,  1102,  1469,  1201,  1007,  1678,  1534,   800,
    1013,   536,   809,  1015,  1356,   792,   996,   999,  1190,  1796,
    1797,   652,   653,   677,   678,   351,   352,   358,  1503,  1657,
    1658,  1278,  1393,  1492,  1651,  1780,  1799,  1689,  1735,  1736,
    1737,  1479,  1480,  1481,  1482,  1691,  1692,  1698,  1747,  1485,
    1486,  1490,  1644,  1645,  1646,  1668,  1826,  1394,  1395,   185,
     136,  1812,  1813,  1649,  1397,  1398,  1399,  1400,   137,   242,
     532,   533,   138,   139,   140,   141,   142,   143,   144,   145,
    1515,   146,   937,  1148,   147,   246,   649,   390,   650,   651,
     527,   640,   641,  1225,   642,  1226,   148,   149,   150,   831,
     151,   152,   338,   153,   339,   567,   568,   569,   570,   571,
     572,   573,   574,   575,   844,   845,  1079,   576,   577,   578,
     851,  1720,   154,   636,  1505,   637,  1116,   906,  1295,  1292,
    1637,  1638,   155,   156,   157,   233,   243,   400,   519,   158,
    1035,   835,   159,  1036,   929,   921,  1037,   983,  1171,   984,
    1173,  1174,  1175,   986,  1335,  1336,   987,   769,   503,   196,
     197,   666,   655,   486,  1134,  1135,   755,   756,   925,   161,
     235,   162,   163,   187,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   715,   239,   240,   617,   225,   226,   718,
     719,  1231,  1232,   374,   375,   892,   174,   605,   175,   648,
     176,   330,  1659,  1710,   365,   408,   672,   673,  1029,  1129,
    1276,   881,   882,   814,   815,   816,   331,   332,   837,  1468,
     923
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1485
static const yytype_int16 yypact[] =
{
   -1485,   170, -1485, -1485,  5662, 13321, 13321,   -33, 13321, 13321,
   13321, 11251, 13321, -1485, 13321, 13321, 13321, 13321, 13321, 13321,
   13321, 13321, 13321, 13321, 13321, 13321, 17448, 17448, 11458, 13321,
   17574,   -28,   -17, -1485, -1485, -1485, -1485, -1485,   181, -1485,
   -1485,   114, 13321, -1485,   -17,    -7,    -4,    51, -1485,   -17,
   11665, 14606, 11872, -1485, 14554, 10216,   169, 13321,  4469,   142,
   -1485, -1485, -1485,   256,    53,   262,   178,   180,   186,   202,
   -1485, 14606,   206,   211, -1485, -1485, -1485, -1485, -1485, 13321,
     513,  4254, -1485, -1485, 14606, -1485, -1485, -1485, -1485, 14606,
   -1485, 14606, -1485,    75,   221, 14606, 14606, -1485,   163, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, 13321, -1485, -1485,    52,   667,   711,   711,
   -1485,   454,   361,   479, -1485,   305, -1485,    70, -1485,   493,
   -1485, -1485, -1485, -1485,  4305,   654, -1485, -1485,   328,   330,
     342,   346,   362,   376,  5081, -1485, -1485, -1485, -1485,    60,
   -1485,   517,   528,   378, -1485,    64,   394,   451,    -2, -1485,
     940,   150, -1485, -1485,  3810,   154,   413,    93, -1485,   156,
     160,   417,    56, -1485, -1485,   557, -1485, -1485, -1485,   478,
     431,   481, -1485, -1485,   493,   654, 18431,  3995, 18431, 13321,
   18431, 18431, 14943,   436, 16451, 14943,   613, 14606,   599,   599,
     458,   599,   599,   599,   599,   599,   599,   599,   599,   599,
   -1485, -1485, -1485,   370, 13321,   496, -1485, -1485,   521,   487,
     522,   504,   522, 17448, 16732,   515,   717, -1485,   478, 13321,
     496,   579,   592,   555, -1485,   161, -1485, -1485, -1485,   522,
     154, 12079, -1485, -1485, 13321,  4878,   752,    72, 18431, 10009,
   -1485, 13321, 13321, 14606, -1485, -1485,  5130,   574, -1485,  5175,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, 16232,
   -1485, 16232, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485,    78,    82,   481,
   -1485, -1485, -1485, -1485,   583, 16015,    86, -1485, -1485,   641,
     765, -1485,   644, 15336,   718, -1485,   604, 15790,   605,   979,
   -1485,    32, 15835, 17639, 18125, 14606,    84, -1485,    44, -1485,
   16923,    92, -1485,   675, -1485,   684, -1485,   797,    94, 17448,
   13321, 13321,   622,   642, -1485, -1485, 17054, 11458,   109,   277,
     584, -1485, 13528, 17448,   570, -1485, 14606, -1485,   197,   361,
   -1485, -1485, -1485, -1485, 17700,   799,   726, -1485, -1485, -1485,
      65,   630, 18431,   646,  1210,   647,  5869, 13321, -1485,   269,
     623,   713,   269,   520,   472, -1485, 14606, 16232,   650, 10423,
   14554, -1485, -1485, 14216, -1485, -1485, -1485, -1485, -1485,   493,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, 13321, 13321, 13321,
   12286, 13321, 13321, 13321, 13321, 13321, 13321, 13321, 13321, 13321,
   13321, 13321, 13321, 13321, 13321, 13321, 13321, 13321, 13321, 13321,
   13321, 13321, 13321, 17574, 13321, -1485, 13321, 13321, -1485, 13321,
   13735, 14606, 14606, 14606, 14606, 14606,  4305,   746,   916,  4665,
   13321, 13321, 13321, 13321, 13321, 13321, 13321, 13321, 13321, 13321,
   13321, 13321, -1485, -1485, -1485, -1485,  2750, 13321, 13321, -1485,
   10423, 10423, 13321, 13321, 17054,   656,   493, 12493, 15880, -1485,
   13321, -1485,   658,   845,   721,   673,   679, 13878,   522, 12700,
   -1485, 12907, -1485,   682,   695,  2371, -1485,   172, 10423, -1485,
    3230, -1485, -1485, 15925, -1485, -1485, 10630, -1485, 13321, -1485,
     809,  9181,   870,   698, 18341,   890,    81,    55, -1485, -1485,
   -1485,   734, -1485, -1485, -1485, 16232,  2149,   707,   899, 16663,
   14606, -1485, -1485, -1485, -1485, -1485,   738, -1485, -1485, -1485,
     832, 13321,   851,   852, 13321, 13321, 13321, -1485,   979, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485,   744, -1485, -1485, -1485,
     737, -1485, -1485, 14606,   732,   931,    71, 14606,   739,   934,
     399,   421, 18172, -1485, 14606, 13321,   522,   142, -1485, -1485,
   -1485, 16663,   865, -1485,   522,    97,   100,   745,   758,  2868,
     138,   760,   761,   629,   834,   764,   522,   127,   772, -1485,
    1920, 14606, -1485, -1485,   898,  2372,   368, -1485, -1485, -1485,
     361, -1485, -1485, -1485,   936,   846,   805,   358,   826, 13321,
     847,   972,   781,   833,   162, -1485, 16232, 16232,   973,   752,
      65, -1485,   790,   980, -1485, 16232,    27,   524,   151, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485,  1008,  2421, -1485, -1485,
   -1485, -1485,   981,   819, -1485, 17448, 13321,   792,   991, 18431,
     987, -1485, -1485,   874, 14411, 11857, 12064, 14943, 13321, 18386,
   18626, 17253, 10403, 11437, 12678, 12885, 12885, 12885, 12885,  1336,
    1336,  1336,  1336,  1336,  1105,  1105,   768,   768,   768,   458,
     458,   458, -1485,   599, 18431,   796,   798, 16862,   802,   999,
     -20, 13321,   -10,   496,   350, -1485, -1485, -1485,   996,   726,
   -1485,   493, 13321, 17186, -1485, -1485, 14943, -1485, 14943, 14943,
   14943, 14943, 14943, 14943, 14943, 14943, 14943, 14943, 14943, 14943,
   -1485, 13321,     4, -1485,   165, -1485,   496,    20,   806,  2692,
     813,   815,   810,  3099,   129,   821, -1485, 18431, 16793, -1485,
   14606, -1485,    27,   259, 17448, 18431, 17448, 17887,    27,   522,
     167, -1485,   162,   868,   824, 13321, -1485,   168, -1485, -1485,
   -1485,  8974,   628, -1485, -1485, 18431, 18431,   -17, -1485, -1485,
   -1485, 13321,   924, 16512, 16663, 14606,  9388,   825,   827, -1485,
     103,   943,   905,   888, -1485,  1034,   849, 16090, 16232, 16663,
   16663, 16663, 16663, 16663,   853,   895,   861, 16663,   572, -1485,
     900, -1485,   859, -1485, 18519, -1485,    15, -1485, 13321,   878,
   18431,   879,  1054, 11236,  1063, -1485, 18431, 15970, -1485,   744,
     998, -1485,  6076, 17513,   881,   422, -1485, 17639, 14606,   449,
   -1485, 18125, 14606, 14606, -1485, -1485,  3428, -1485, 18519,  1071,
   17448,   885, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485,   137, 14606, 17513,   889, 17054, 17317,  1074, -1485, -1485,
   -1485, -1485,   884, -1485, 13321, -1485, -1485,  5248, -1485, 16232,
   17513,   892, -1485, -1485, -1485, -1485,  1085,   906, 13321, 17700,
   -1485, -1485, 14021,   904, -1485, 16232, -1485,   902,  6283,  1079,
      67, -1485, -1485,    79,  2750, -1485,  3230, -1485, 16232, -1485,
   -1485,   522, 18431, -1485, 10837, -1485, 16663,    91,   901, 17513,
     846, -1485, -1485, 18555, 13321, -1485, -1485, 13321, -1485, 13321,
   -1485,  3618,   910, 10423,   834,  1081,   846, 16232,  1100,   874,
   14606, 17574,   522,  3757,   918, -1485, -1485,   158,   929, -1485,
   -1485,  1102, 17497, 17497, 16793, -1485, -1485, -1485,  1083,   937,
     414,   939, -1485, -1485, -1485, -1485,  1135,   944,   658,   522,
     522, 13114,  3230, -1485, -1485,  3941,   634,   -17, 10009, -1485,
    6490,   945,  6697,   946, 16512, 17448,   965,  1014,   522, 18519,
    1154, -1485, -1485, -1485, -1485,   743, -1485,    45, 16232, -1485,
    1047, 16232, 14606,  2149, -1485, -1485, -1485,  1171, -1485,   982,
     981,   789,   789,  1115,  1115, 17989,   975,  1175, 16663, 15622,
   17700,  2088, 15479, 16663, 16663, 16663, 16663, 16382, 16663, 16663,
   16663, 16663, 16663, 16663, 16663, 16663, 16663, 16663, 16663, 16663,
   16663, 16663, 16663, 16663, 16663, 16663, 16663, 16663, 16663, 16663,
   16663, 14606, -1485, 18431, 13321, 13321, 13321, -1485, -1485, -1485,
   13321, 13321, -1485,   979, -1485,  1106, -1485, -1485, 14606, -1485,
   -1485, 14606, -1485, -1485, -1485, -1485, 16663,   522, -1485,   629,
   -1485,   612,  1178, -1485, -1485,   130,   988,   522, 11044, -1485,
    2014, -1485,  5455,   726,  1178, -1485,   409,    -6, -1485, 18431,
    1057, -1485, -1485, -1485, -1485,   990,  1079, -1485, 16232,   752,
   16232,    35,  1179,  1114,   171, -1485,   496,   173, -1485, -1485,
   17448, 13321, 18431, 18519,   994,    91, -1485,   997,    91,  1002,
   18555, 18431, 17932,  1003, 10423,  1004,  1001, 16232,  1005,   995,
   16232,   846, -1485,   555,   201, 10423, 13321, -1485, -1485, -1485,
   -1485, -1485, -1485,  1078,   992,  1197,  1124, 16793,  1064, -1485,
   17700, 16793, -1485, -1485, -1485, 17448, 18431, -1485,   -17,  1183,
    1140, 10009, -1485, -1485, -1485,  1013, 13321,  1014,   522, 17054,
   16512,  1015, 16663,  6904,   775,  1017, 13321,    77,   285, -1485,
    1049, 16232, -1485,  1088, -1485, 16157,  1193,  1036, 16663, -1485,
   16663, -1485,  1037, -1485,  1109,  1233,  1043, -1485, -1485, -1485,
   18034,  1041,  1237,  2990, 13732, 15333, 16663, 18476, 18661, 17515,
   10817, 12472, 13092, 16012, 16012, 16012, 16012,  2751,  2751,  2751,
    2751,  2751,   869,   869,   789,   789,   789,  1115,  1115,  1115,
    1115, -1485, 18431, 13513, 18431, -1485, 18431, -1485,  1046, -1485,
   -1485, -1485, 18519, 14606, 16232, 16232, -1485, 17513,    95, -1485,
   17054, -1485, -1485, 14943,  1045, -1485,  1048,   187, -1485,    89,
   13321, -1485, -1485, -1485, 13321, -1485, 13321, -1485,   752, -1485,
   -1485,   375,  1239,  1176, 13321, -1485,  1055,   522, 18431,  1079,
    1056, -1485,  1058,    91, 13321, 10423,  1060, -1485, -1485,   726,
   -1485, -1485,  1061,  1062,  1066, -1485,  1069, 16793, -1485, 16793,
   -1485, -1485,  1070, -1485,  1126,  1072,  1263, -1485,   522,  1246,
   -1485,  1076, -1485, -1485,  1080,  1086,   135, -1485, -1485, 18519,
    1082,  1084, -1485,  4494, -1485, -1485, -1485, -1485, -1485, -1485,
   16232, -1485, 16232, -1485, 18519, 18091, -1485, 16663, 17700, -1485,
   -1485, 16663, -1485, 16663, -1485, 18591, 16663, 13321,  1077,  7111,
     612, -1485, -1485, -1485,   585, 14749, 17513,  1180, -1485,  4061,
    1132, 14998, -1485, -1485, -1485,   746, 15969,   110,   111,  1091,
     726,   916,   136, -1485, -1485, -1485,  1138,  4070,  4268, 18431,
   -1485,    54,  1281,  1220, 13321, -1485, 18431, 10423,  1188,  1079,
     527,  1079,  1101, 18431,  1103, -1485,   669,  1104,  1573, -1485,
   -1485,    91, -1485, -1485,  1181, -1485, 16793, -1485, 17700, -1485,
   -1485,  8974, -1485, -1485, -1485, -1485,  9595, -1485, -1485, -1485,
    8974, -1485,  1111, 16663, 18519,  1182, 18519, 18136, 18591, 13306,
   -1485, -1485, -1485, 17513, 17513, 14606, -1485,  1289, 15622,    88,
   -1485, 14749,   726, 18111, -1485,  1143, -1485,   113,  1113,   116,
   -1485, 15142, -1485, -1485, -1485,   117, -1485, -1485, 16846, -1485,
    1116, -1485,  1240,   493, -1485, 14944, -1485, 14944, -1485, -1485,
    1305,   746, -1485, 14164, -1485, -1485, -1485, -1485,  1307,  1242,
   13321, -1485, 18431,  1122,  1125,  1121,   533, -1485,  1188,  1079,
   -1485, -1485, -1485, -1485,  1599,  1128, 16793, -1485,  1194,  8974,
    9802,  9595, -1485, -1485, -1485,  8974, -1485, 18519, 16663, 16663,
   13321,  7318,  1129,  1130, -1485, 16663, -1485, 17513, -1485, -1485,
   -1485, -1485, -1485, 16232,   728,  4061, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485, -1485,   147, -1485,  1132,
   -1485, -1485, -1485, -1485, -1485,    69,   578, -1485,  1313,   120,
   15336,  1240,  1316, -1485, 16232,   493, -1485, -1485,  1131,  1321,
   13321, -1485, 18431, -1485,   502, -1485, -1485, -1485, -1485,  1133,
     533, 14359, -1485,  1079, -1485, 16793, -1485, -1485, -1485, -1485,
    7525, 18519, 18519, 11650, -1485, -1485, -1485, 18519, -1485,  3979,
     144,  1328,  1134, -1485, -1485, 16663, 15142, 15142,  1283, -1485,
   16846, 16846,   610, -1485, -1485, -1485, 16663,  1262, -1485,  1177,
    1147,   121, 16663, -1485, 14606, -1485, 16663, 18431,  1268, -1485,
    1342,  7732,  7939, -1485, -1485, -1485,   533, -1485,  8146,  1150,
    1273,  1247, -1485,  1259,  1208, -1485, -1485,  1266, 16232, -1485,
     728, -1485, -1485, 18519, -1485, -1485,  1202, -1485,  1327, -1485,
   -1485, -1485, -1485, 18519,  1352,   629, -1485, -1485, 18519,  1168,
   18519, -1485,   539,  1170, -1485, -1485,  8353, -1485,  1167, -1485,
   -1485,  1191,  1223, 14606,   916,  1225, -1485, -1485, 16663,   159,
     108, -1485,  1320, -1485, -1485, -1485, -1485, 17513,   881, -1485,
    1227, 14606,   576, -1485, 18519, -1485,  1195,  1392,   769,   108,
   -1485,  1323, -1485, 17513,  1203, -1485,  1079,   139, -1485, -1485,
   -1485, -1485, 16232, -1485,  1205,  1206,   124, -1485,   536,   769,
     380,  1079,  1211, -1485, -1485, -1485, -1485, 16232,    76,  1394,
    1333,   536, -1485,  8560,   446,  1401,  1341, 13321, -1485, -1485,
    8767, -1485,    83,  1408,  1343, 13321, -1485, 18431, -1485,  1410,
    1345, 13321, -1485, 18431, 13321, -1485, 18431, 18431
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1485, -1485, -1485,  -565, -1485, -1485, -1485,   492,     2,  -305,
   -1485, -1485, -1485,   835,   571,   568,    40,  1624,  1554, -1485,
    2691, -1485,  -469, -1485,    26, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485, -1485,  -358, -1485, -1485,  -182,
     131,    28, -1485, -1485, -1485, -1485, -1485, -1485,    29, -1485,
   -1485, -1485, -1485,    31, -1485, -1485,   966,   970,   974,   -95,
     477,  -872,   485,   541,  -361,   246,  -927, -1485,   -86, -1485,
   -1485, -1485, -1485,  -733,    96, -1485, -1485, -1485, -1485,  -352,
   -1485,  -610, -1485,  -402, -1485, -1485,   854, -1485,   -74, -1485,
   -1485, -1033, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485, -1485,  -102, -1485,   -18, -1485, -1485, -1485, -1485, -1485,
    -185, -1485,    80,  -952, -1485, -1484,  -371, -1485,  -133,    18,
    -127,  -357, -1485,  -188, -1485, -1485, -1485,    98,   -41,    -3,
      38,  -718,  -430, -1485, -1485,   -19, -1485, -1485,    -5,   -64,
     -51, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
    -590,  -846, -1485, -1485, -1485, -1485, -1485,  1067, -1485, -1485,
   -1485, -1485,  1000, -1485, -1485,   392, -1485,   914, -1485, -1485,
   -1485, -1485, -1485, -1485, -1485,   403, -1485,   919, -1485, -1485,
     637, -1485,   372, -1485, -1485, -1485, -1485, -1485, -1485, -1485,
   -1485,  -932, -1485,  2495,   724,  -391, -1485, -1485,   326,  3551,
    2148, -1485, -1485,   453,  -178,  -647, -1485, -1485,   523,   317,
    -724,   318, -1485, -1485, -1485, -1485, -1485,   509, -1485, -1485,
   -1485,    30,  -869,  -163,  -416,  -415, -1485,   575,  -112, -1485,
   -1485,    24,    37,   476, -1485, -1485,   235,   -40, -1485, -1485,
       8,  -342,    48,   210, -1485, -1485,  -437,  1136, -1485, -1485,
   -1485, -1485, -1485,   774,   678, -1485, -1485, -1485,  -323,  -660,
   -1485,  1092,  -980, -1485,   -70,  -187,   -50,   687, -1485, -1040,
     128,  -254,   408,   488, -1485, -1485, -1485, -1485,   441,   349,
   -1070
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1007
static const yytype_int16 yytable[] =
{
     186,   188,   495,   190,   191,   192,   194,   195,   420,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   382,   135,   224,   227,   385,   386,   467,   555,   245,
     128,   935,   130,   132,   160,   133,   336,   248,   238,   734,
     917,   250,   644,   516,   985,   256,   254,   259,   512,   393,
     337,  1130,   342,   489,   236,   791,   327,   764,   420,   916,
    1301,   645,   712,  1120,   416,   466,   897,   237,  1147,   958,
     753,   754,   391,   395,   248,  1017,   520,  1197,  1700,   410,
     247,   528,  1287,  1003,  1158,   807,  1298,   -78,   760,   761,
     805,   -43,   -78,   592,   487,   -42,   -43,  1547,   348,  1384,
     -42,   597,  1701,   602,   787,   788,   870,  1354,   392,   528,
     852,    13,  1018,    13,  1131,  1302,   784,   368,   528,  1495,
    1497,   378,  -355,   492,   379,  1555,  1639,  -554,   521,  1707,
    1707,  -862,   210,  1547,  1508,   129,   886,    13,   886,   886,
    1406,    13,   594,   580,   886,   886,  1099,  1206,  1207,  -697,
     506,  1293,  -104,  1738,    13,   723,  1835,   484,   485,  1132,
    -556,  1695,  -103,  1849,   398,   514,  1071,   189,  1099,   858,
       3,   355,   241,   505,   395,   782,  -837,  1696,   468,   356,
    -104,  -557,   918,   244,   498,    13,  1724,   407,   513,  -524,
    -103,  1384,  -840,   251,  1224,  1697,   252,  1294,  -875,   406,
    1303,   581,   723,  1385,  -837,  -556,   621,  -699,  1386,   392,
      60,    61,    62,   177,  1387,   417,  1388,  -865,  -869,  1509,
    -840,   487,   349,  -863,   392,  -868,   406,  -864,  -904,   492,
     488,   383,  -866,    13,  -907,  -906,   523,   684,  -849,   523,
    -850,  1836,  1767,   387,  1133,   383,   248,   534,  1850,  1209,
     545,   253,   210,  1389,  1390,   397,  1391,   116,   808,   493,
    -874,   221,   221,  -294,  -873,  -782,   419,  -862,  -782,  1418,
    1702,   525,   411,  1347,   529,   530,  1019,   418,  1420,  1355,
     -78,  1204,   806,  1208,   -43,  1426,   593,  1428,   -42,  1323,
    1548,  1549,  -294,   491,   598,  1385,   603,   608,   871,  1392,
    1386,   872,    60,    61,    62,   177,  1387,   417,  1388,  -278,
    1100,   619,  1496,  1498,   765,  -355,   496,  1739,  1556,  1640,
     607,   611,  1708,  1757,  1000,   771,  1823,  1144,   887,  1002,
     971,  1279,  1795,  1112,  1334,   327,  1445,  1502,   491,   876,
    -782,   420,   357,   682,   620,  1389,  1390,   350,  1391,   369,
    -872,  -876,   406,  -865,  -869,   248,   392,   488,  -879,  -863,
     388,  -868,   224,  -864,  -904,   493,   389,   625,  -866,   418,
    -907,  -906,   340,  -838,  -849,   353,  -850,   116,   359,  1516,
     360,  1518,   354,   586,   590,   591,   361,  1206,  1207,   484,
     485,  1405,   667,   382,   728,   729,   416,  1524,   606,   632,
     633,  -838,   362,   333,   679,  1467,   366,   484,   485,   903,
    1411,   367,   327,  -698,   135,  1828,   630,  -870,   372,   373,
     654,   384,   685,   686,   687,   689,   690,   691,   692,   693,
     694,   695,   696,   697,   698,   699,   700,   701,   702,   703,
     704,   705,   706,   707,   708,   709,   710,   711,  1105,   713,
     735,   714,   714,  1286,   717,  1412,   504,  1337,   221,  -877,
    1829,   238,   722,   910,   736,   738,   739,   740,   741,   742,
     743,   744,   745,   746,   747,   748,   749,   236,  1344,  1670,
     164,  1842,   714,   759,   405,   679,   679,   714,   763,  1357,
     237,  1550,   736,   924,   926,   767,   121,   862,   484,   485,
    1178,  -701,   220,   222,   775,   409,   777,   794,   724,   327,
    1137,  1138,   453,   679,   904,  1652,   406,  1653,   371,   863,
    1088,   795,  1455,   796,   454,   781,  1843,   129,   412,   905,
     421,  1384,   422,   467,   757,  1310,   952,  1718,  1312,  1300,
    1413,   950,  1203,   257,   423,  1830,   326,  1091,   424,   960,
     953,  1155,   369,  -870,   406,   724,   840,   799,   627,   843,
     846,   847,  1179,   363,   425,   783,  1288,   644,   789,   964,
    1163,   466,   896,    13,  1782,   907,   406,   406,   426,  1289,
     458,   380,  1719,   363,   456,   221,   645,   363,   363,   394,
     866,  -558,  1528,   369,   221,   457,   610,   459,   731,   370,
     369,   221,   460,  1433,   406,  1434,   627,  1703,   221,   924,
     926,  1844,  1290,   490,   942,   992,   926,  -871,   543,  1783,
     544,   372,   373,   855,  -555,  1704,   363,   859,  1705,  -698,
    -979,   494,   348,  1726,   392,  1385,   376,  1024,   499,  1750,
    1386,   516,    60,    61,    62,   177,  1387,   417,  1388,  1072,
     369,   407,   484,   485,   484,   485,   627,  1751,   501,  1427,
    1752,   371,   372,   373,   369,   454,   993,   716,   407,   372,
     373,   932,   507,  1384,   548,  -700,   879,   880,  1281,  -979,
     654,   670,  -979,   943,   468,  1389,  1390,  -875,  1391,   502,
     394,    60,    61,    62,   177,   178,   417,   758,   644,   508,
     997,   998,   762,  1422,   491,   515,  1188,  1189,  1410,   418,
    1274,  1275,  1527,  1463,  1464,    13,   951,   645,   628,   372,
     373,   164,   510,  -979,  -877,   164,   511,   194,   669,   221,
    -696,  1517,   622,   372,   373,  1666,  1667,   121,  1824,  1825,
    1500,   121,  1820,   517,   407,   535,   963,   369,  1748,  1749,
     218,   218,  1316,   399,   232,   518,   674,  1834,   418,   333,
     526,    53,  1346,  1326,  1744,  1745,  1818,  1027,  1030,    60,
      61,    62,   177,   178,   417,  -979,   539,  1385,  1805,   549,
     995,  1831,  1386,   546,    60,    61,    62,   177,  1387,   417,
    1388,   369, -1006,   369,  1001,   550,   248,   402,   556,   627,
    1379,   599,  1674,    33,    34,    35,   557,   559,  1182,   135,
     600,   601,  1551,   646,   613,   211,   372,   373,   450,   451,
     452,  1525,   453,   612,   647,   554,   596,  1389,  1390,   668,
    1391,   656,  1012,  1073,   454,   604,   418,   609,   644,  1067,
    1068,  1069,   616,  1402,  1205,  1206,  1207,   657,   659,   626,
    -129,   418,  1217,    53,   770,  1070,   681,   645,   768,  1221,
     372,   373,   372,   373,    74,    75,    76,    77,    78,   622,
     135,  1441,   164,  1521,   772,   213,  1351,  1206,  1207,   528,
     773,    82,    83,   778,  1809,  1810,  1811,  1450,   121,  1110,
    1162,   401,   403,   404,   810,    92,   779,   586,   671,   797,
     801,   590,   326,  1119,   804,   363,   545,   817,   818,    97,
     221,  1798,   839,  1424,  1122,   135,  1064,  1065,  1066,  1067,
    1068,  1069,   129,   128,   838,   130,   132,   160,   133,  1142,
    1798,   841,   842,   850,   856,  1070,   135,   853,  1819,  1150,
     857,   860,  1151,   861,  1152,   869,   873,   218,   679,  1306,
     654,  1727,   554,   363,   726,   363,   363,   363,   363,   874,
    1123,   877,   883,   878,   885,   894,   899,   654,   221,   238,
     616,  1529,   757,   888,   789,   900,   902,  -716,   752,   908,
    1535,   909,   911,   129,   912,   236,  1186,   915,   919,   920,
     928,  1541,   930,   933,  1191,   913,   914,  1330,   237,   554,
     934,   936,   939,   945,   922,   946,   948,   164,   949,   221,
     957,   221,   786,   965,   967,  1513,   968,   969,   135,  -701,
     135,   941,  1004,   121,  1192,   994,   644,  1014,   129,  1016,
    1020,    60,    61,    62,   177,   178,   417,  1021,   221,  1022,
     789,  1223,   836,  1023,  1229,   645,  1039,  1369,   623,   129,
    1025,  1042,   629,  1038,  1374,    60,    61,    62,    63,    64,
     417,  1040,  1043,  1680,  1074,  1075,    70,   461,  1076,  1262,
    1263,  1264,  1080,  1763,   218,   843,  1266,   623,  1083,   629,
     623,   629,   629,   218,  1086,  1096,   865,  1098,  1108,  1282,
     218,  1109,  1104,   216,   216,  1115,   644,   218,   418,  1117,
    1126,  1145,   462,  1283,   463,   221,  1124,  1118,   643,  1128,
    1154,  1157,   891,   893,  1160,   645,  1166,   464,  1165,   465,
     221,   221,   418,    60,    61,    62,    63,    64,   417,  -878,
     135,   129,  1176,   129,    70,   461,  1308,  1177,   128,  1180,
     130,   132,   160,   133,  1181,  1183,  1200,  1194,  1196,   679,
    1808,   931,   447,   448,   449,   450,   451,   452,  1439,   453,
     679,  1283,   560,   561,   562,  1199,   674,   674,  1202,   563,
     564,   454,   463,   565,   566,   654,   363,   232,   654,  1211,
    1215,  1070,  1219,  1216,  1220,  1339,  1268,  1277,  1280,  1296,
     418,   248,  1297,  1304,  1305,  1309,  1721,  1321,  1722,  1328,
    1311,  1353,  1313,  1315,  1318,  1317,  1329,  1728,  1320,   962,
    1327,   978,  1333,  1340,  1341,  1343,  1348,  1342,   218,  1352,
    1360,   135,  1358,  1362,   497,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,  1363,  1366,   221,
     221,  1367,  1368,   129,  1370,  1372,  1373,  1378,  1113,  1403,
     989,  1404,   990,  1414,  1766,  1417,  1415,  1419,  1436,  1421,
     982,  1425,   988,  1429,  1125,  1430,  1431,   164,  1501,   420,
    1432,  1435,  1438,  1437,   482,   483,  1440,  1139,  1442,  1008,
    1460,  1443,   164,   121,  1447,  1407,  1448,  1444,  1471,  1408,
     216,  1409,  1484,  1499,  1504,  1510,  1396,  1010,   121,  1416,
    1511,  1514,  1519,  1545,  1520,  1396,  1159,  1522,  1401,  1423,
     679,  1650,  1536,  1526,  1538,  1553,  1554,  1401,  1647,  1654,
    1648,  1660,  1661,  1663,  1665,  1664,  1675,  1706,   164,  1673,
    1712,  1685,  1686,  1715,   129,  1716,  1723,  1740,  1742,  1746,
     484,   485,  1754,   654,   121,  1709,  1097,  1756,  1761,  1755,
    1090,  1762,  1769,  1770,  1093,  1094,  1833,  -351,  1771,  1773,
    1701,   616,  1107,  1840,  1774,  1777,  1778,  1210,  1781,  1786,
    1212,  1784,  1459,   164,  1101,   221, -1007, -1007, -1007, -1007,
   -1007,   445,   446,   447,   448,   449,   450,   451,   452,   121,
     453,  1787,  1788,  1803,   164,  1544,  1806,   135,  1793,   218,
    1800,  1807,   454,  1815,   554,  1817,  1821,  1822,  1837,  1512,
     121,   658,   679,  1838,  1832,  1845,   752,   216,   786,   468,
     221,  1846,  1851,  1852,  1854,  1855,   216,   864,  1089,  1092,
    1802,   725,   730,   216,   221,   221,  1161,   727,  1396,  1156,
     216,  1114,  1816,  1345,  1396,  1679,  1396,  1814,  1671,  1449,
    1401,   867,   363,  1694,  1699,  1552,  1401,   218,  1401,   135,
    1839,   654,  1827,  1711,  1170,  1170,   982,  1669,   135,   737,
    1546,  1491,  1533,  1714,   164,  1267,   164,  1299,   164,   922,
    1008,  1198,   848,  1265,   786,  1472,  1082,   849,  1291,  1325,
     121,  1222,   121,  1331,   121,  1332,  1172,  1184,   218,  1136,
     218,  1779,   680,   618,  1028,  1662,  1319,  1273,  1462,  1322,
     129,  1214,  1261,  1741,  1213,   221,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1493,   218,     0,     0,
       0,   554,     0,     0,   554,  1683,     0,     0,     0,     0,
       0,     0,  1396,     0,     0,     0,     0,   135,     0,     0,
       0,     0,     0,   135,  1401,     0,  1677,  1533,     0,   135,
    1359,   216,     0,   836,  1139,     0,     0,     0,     0,     0,
       0,     0,   129,     0,     0,     0,     0,  1384,     0,     0,
    1270,   129,     0,  1271,     0,     0,     0,     0,   164,     0,
       0,     0,     0,     0,   218,     0,     0,     0,     0,     0,
       0,     0,     0,  1384,   121,     0,     0,     0,     0,   218,
     218,     0,     0,     0,     0,     0,  1307,     0,     0,    13,
       0,     0,     0,  1381,  1382,     0,     0,     0,     0,     0,
       0,     0,  1655,   643,     0,     0,     0,     0,     0,     0,
       0,  1791,     0,     0,  1759,    13,     0,     0,     0,     0,
     215,   215,   327,     0,   230,  1717,     0,     0,     0,     0,
     129,  1338,     0,     0,     0,   420,   129,   164,     0,   982,
       0,     0,   129,   982,     0,   616,  1008,     0,   230,   164,
       0,  1385,     0,   121,     0,   232,  1386,     0,    60,    61,
      62,   177,  1387,   417,  1388,   121,     0,     0,   135,     0,
       0,     0,     0,     0,     0,     0,     0,  1385,     0,  1451,
       0,  1452,  1386,     0,    60,    61,    62,   177,  1387,   417,
    1388,     0,     0,     0,     0,     0,     0,     0,   218,   218,
       0,  1389,  1390,     0,  1391,     0,     0,     0,     0,   135,
     135,     0,   216,     0,     0,  1494,   135,     0,     0,     0,
       0,     0,     0,     0,     0,   418,   616,  1389,  1390,     0,
    1391,     0,     0,     0,   643,  1380,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1523,     0,     0,
       0,   418,     0,     0,   135,     0,     0,     0,     0,     0,
       0,     0,  1792,     0,     0,     0,     0,     0,     0,     0,
     216,     0,     0,  1672,     0,     0,     0,     0,     0,     0,
     654,   129,     0,     0,     0,     0,     0,     0,     0,   982,
       0,   982,     0,     0,     0,     0,     0,     0,     0,   654,
       0,     0,  1847,     0,     0,     0,     0,   654,     0,     0,
    1853,   216,     0,   216,     0,     0,  1856,   215,     0,  1857,
       0,   135,   129,   129,     0,   164,     0,     0,   135,   129,
       0,     0,     0,     0,   218,     0,     0,     0,     0,     0,
     216,   121,     0,     0,     0,     0,     0,   326,     0,     0,
       0,     0,     0,  1489,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   230,     0,   230,     0,   129,     0,     0,
       0,     0,  1690,     0,   643,     0,     0,     0,     0,   218,
       0,     0,     0,     0,     0,     0,     0,   164,     0,     0,
       0,     0,   164,   218,   218,     0,   164,     0,   982,     0,
       0,     0,     0,   121,     0,     0,     0,   216,   121,     0,
       0,     0,   121,     0,     0,     0,     0,     0,     0,   230,
       0,     0,   216,   216,     0,     0,     0,   363,     0,     0,
     554,     0,     0,   326,   129,     0,     0,     0,     0,     0,
       0,   129,     0,  1636,   215,     0,     0,     0,     0,     0,
    1643,     0,     0,   215,     0,     0,     0,   326,     0,   326,
     215,     0,     0,     0,     0,   326,     0,   215,    36,     0,
     889,   890,     0,  1713,   218,   164,   164,   164,   230,     0,
       0,   164,     0,     0,     0,     0,     0,   164,   982,    48,
       0,   121,   121,   121,   427,   428,   429,   121,     0,     0,
       0,   230,     0,   121,   230,     0,     0,     0,     0,     0,
       0,     0,     0,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,     0,   453,     0,
       0,   216,   216,     0,     0,     0,     0,   230,    86,    87,
     454,    88,   182,    90,     0,     0,     0,  1775,     0,     0,
       0,     0,   643,     0,     0,     0,     0,     0,  1044,  1045,
    1046,     0,     0,   829,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,  1047,   215,  1048,
    1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,     0,   554,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1070,   829,   164,     0,     0,     0,
       0,   922,   643,   326,     0,     0,     0,   982,     0,   230,
     230,     0,   121,   828,     0,     0,   922,     0,   811,     0,
       0,  1733,     0,     0,     0,     0,     0,     0,  1636,  1636,
       0,     0,  1643,  1643,     0,     0,     0,   164,   164,     0,
       0,     0,     0,     0,   164,     0,   363,   216,     0,     0,
       0,     0,     0,   121,   121,     0,     0,     0,     0,     0,
     121,  1284,     0,     0,     0,   828,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,   812,     0,     0,     0,
       0,     0,   164,     0,     0,     0,     0,     0,    48,     0,
       0,     0,   216,     0,     0,     0,     0,     0,   121,     0,
       0,     0,     0,     0,     0,  1790,   216,   216,     0,     0,
     230,   230,     0,     0,     0,     0,     0,     0,     0,   230,
       0,     0,     0,  1804,     0,     0,     0,     0,     0,  1227,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   215,
       0,   181,     0,     0,    84,     0,     0,    86,    87,   164,
      88,   182,    90,     0,     0,     0,   164,     0,     0,     0,
       0,     0,     0,     0,     0,   121,     0,     0,     0,     0,
       0,     0,   121,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   216,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   215,   829,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   829,   829,   829,   829,   829,     0,     0,
       0,   829,   427,   428,   429,   497,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   215,     0,
     215,   430,     0,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,     0,   453,   215,   828,     0,
       0,   427,   428,   429,     0,   482,   483,     0,   454,     0,
       0,   230,   230,   828,   828,   828,   828,   828,     0,     0,
     430,   828,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,     0,   453,     0,   230,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   454,     0,     0,
     829,     0,     0,     0,   215,     0,     0,     0,     0,     0,
       0,   484,   485,     0,     0,     0,     0,   230,     0,   215,
     215,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   217,   217,   230,   230,   231,     0,     0,     0,     0,
       0,     0,     0,   230,     0,     0,     0,     0,     0,   230,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   230,     0,     0,     0,     0,     0,     0,     0,
     828,     0,     0,   230,     0,     0,     0,     0,     0,     0,
       0,     0,   780,     0,     0,     0,   895,     0,     0,     0,
       0,   230,     0,     0,     0,   230,     0,     0,     0,     0,
       0,     0,   829,     0,     0,     0,     0,   829,   829,   829,
     829,   829,   829,   829,   829,   829,   829,   829,   829,   829,
     829,   829,   829,   829,   829,   829,   829,   829,   829,   829,
     829,   829,   829,   829,   829,   927,     0,     0,   215,   215,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   230,     0,     0,   230,     0,   230,     0,     0,
     829,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   828,     0,   230,     0,     0,   828,   828,   828,
     828,   828,   828,   828,   828,   828,   828,   828,   828,   828,
     828,   828,   828,   828,   828,   828,   828,   828,   828,   828,
     828,   828,   828,   828,   828,     0,     0,   834,     0,     0,
       0,     0,   427,   428,   429,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   217,     0,
     828,   430,     0,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   329,   453,     0,     0,   868,
       0,     0,   230,     0,   230,     0,   829,     0,   454,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,     0,
       0,     0,   829,     0,   829,     0,     0,     0,     0,     0,
       0,   230,     0,     0,   230,     0,     0,     0,     0,     0,
     829, -1007, -1007, -1007, -1007, -1007,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,   230,     0,     0,     0,     0,   215,
       0,     0,     0,     0,     0,     0,     0,  1070,     0,     0,
       0,     0,     0,   215,   215,     0,   828,     0,    36,     0,
     210,     0,     0,     0,     0,   230,     0,     0,     0,   230,
       0,     0,   828,     0,   828,   217,     0,     0,     0,    48,
       0,     0,     0,     0,   217,     0,     0,     0,     0,     0,
     828,   217,     0,     0,     0,     0,     0,     0,   217,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   217,
       0,     0,   497,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,   481,     0,   966,     0,   230,   230,
       0,   230,     0,     0,   215,     0,   750,     0,    86,    87,
       0,    88,   182,    90,     0,     0,     0,     0,     0,     0,
       0,   829,     0,     0,     0,   829,     0,   829,     0,     0,
     829,     0,   482,   483,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,   231,     0,
       0,     0,  1009,   751,     0,   116,     0,     0,     0,     0,
     329,     0,   329,     0,     0,     0,     0,  1031,  1032,  1033,
    1034,     0,     0,     0,     0,  1041,     0,     0,     0,     0,
       0,     0,     0,     0,   230,     0,   230,     0,     0,   217,
       0,   828,   230,     0,     0,   828,     0,   828,   484,   485,
     828,  1045,  1046,     0,     0,     0,     0,   829,     0,   230,
     230,     0,     0,   230,     0,     0,   329,     0,     0,  1047,
     230,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,     0,   832,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1070,     0,     0,     0,
       0,     0,   230,     0,     0,     0,     0,     0,     0,   875,
       0,     0,     0,     0,     0,     0,     0,   828,     0,     0,
       0,     0,     0,     0,  1143,     0,     0,   230,   230,     0,
       0,     0,   829,   829,     0,   230,   832,   230,   329,   829,
       0,   329,     0,     0,     0,     0,     0,     0,     0,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,   230,
       0,   230,     0,     0,     0,     0,     0,   230,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,     0,   453,     0,     0,     0,     0,     0,     0,
       0,     0,   828,   828,     0,   454,     0,     0,     0,   828,
     217,   230,     0,     0,     0,     0,     0,   230,     0,   230,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1230,  1233,  1234,  1235,  1237,  1238,  1239,  1240,  1241,
    1242,  1243,  1244,  1245,  1246,  1247,  1248,  1249,  1250,  1251,
    1252,  1253,  1254,  1255,  1256,  1257,  1258,  1259,  1260,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   217,     0,
       0,     0,     0,     0,     0,     0,   329,   813,     0,     0,
     830,     0,     0,     0,  1272,     0,     0,     0,     0,   829,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     829,     0,     0,     0,     0,     0,   829,     0,     0,   217,
     829,   217,     0,     0,     0,     0,     0,     0,   230,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   830,     0,     0,   230,     0,     0,   217,   832,
       0,     0,     0,   970,     0,     0,     0,     0,    36,     0,
     210,     0,     0,   230,   832,   832,   832,   832,   832,   828,
       0,     0,   832,     0,     0,     0,     0,     0,     0,    48,
     828,     0,   829,     0,     0,     0,   828,   329,   329,     0,
     828,     0,     0,     0,     0,     0,   329,     0,  1085,     0,
    1349,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   230,     0,     0,   217,  1364,     0,  1365,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1103,     0,
     217,   217,     0,     0,  1375,     0,   750,     0,    86,    87,
       0,    88,   182,    90,     0,  1103,     0,     0,     0,     0,
       0,     0,   828,     0,   217,     0,     0,     0,     0,     0,
       0,   230,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   230,     0,     0,
       0,   832,     0,   785,  1146,   116,   230,     0,   427,   428,
     429,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   230,     0,     0,     0,     0,   231,   430,     0,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,     0,   453,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   830,     0,     0,     0,   217,
     217,     0,     0,     0,     0,     0,     0,     0,   329,   329,
     830,   830,   830,   830,   830,  1454,     0,     0,   830,  1456,
       0,  1457,     0,     0,  1458,     0,     0,     0,     0,     0,
       0,     0,     0,   832,     0,   217,     0,     0,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,   832,   832,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   219,   219,     0,
       0,   234,     0,     0,     0,     0,     0,     0,     0,     0,
     329,   832,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1537,     0,     0,     0,     0,   329,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   329,
       0,     0,     0,     0,     0,     0,     0,   830,   427,   428,
     429,     0,  1095,     0,     0,   217,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   430,   329,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,     0,   453,     0,     0,   217,     0,     0,     0,     0,
     217,     0,     0,     0,   454,     0,  1681,  1682,     0,     0,
       0,     0,     0,  1687,   217,   217,     0,   832,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   329,
       0,     0,   329,   832,   813,   832,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   830,
       0,   832,     0,     0,   830,   830,   830,   830,   830,   830,
     830,   830,   830,   830,   830,   830,   830,   830,   830,   830,
     830,   830,   830,   830,   830,   830,   830,   830,   830,   830,
     830,   830,     0,     0,     0,     0,     0,   427,   428,   429,
       0,     0,  1383,     0,   219,   217,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   430,   830,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,     0,     0,     0,     0,     0,     0,     0,   329,
       0,   329,  1153,   454,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,     0,     0,     0,
       0,     0,     0,  1743,     0,     0,     0,     0,   329,     0,
       0,   329,     0,     0,  1753,     0,     0,     0,     0,     0,
    1758,     0,   832,   217,  1760,     0,   832,     0,   832,     0,
       0,   832,     0,     0,   482,   483,     0,     0,     0,     0,
       0,  1470,     0,     0,  1483,     0,     0,     0,     0,     0,
       0,     0,     0,   830,     0,     0,     0,     0,     0,     0,
       0,   219,   329,     0,     0,     0,   329,     0,     0,   830,
     219,   830,     0,     0,     0,     0,     0,   219,     0,     0,
       0,     0,     0,     0,   219,     0,  1794,   830,     0,     0,
       0,     0,     0,   217,     0,   234,     0,     0,     0,     0,
     484,   485,     0,     0,     0,     0,     0,     0,   832,     0,
       0,   427,   428,   429,     0,     0,     0,     0,  1542,  1543,
       0,  1164,     0,     0,     0,   329,   329,     0,  1483,     0,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,     0,   453,     0,     0,     0,     0,
       0,     0,     0,     0,   234,     0,     0,   454,     0,   497,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,   481,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   832,   832,     0,     0,     0,     0,     0,
     832,     0,  1688,     0,     0,   219,     0,     0,     0,     0,
    1483,   329,     0,   329,     0,     0,     0,    36,   830,   482,
     483,     0,   830,     0,   830,     0,     0,   830,     0,     0,
       0,     0,     0,     0,     0,     0,   329,     0,    48,     0,
     427,   428,   429,     0,     0,     0,     0,   329,     0,     0,
    1473,     0,     0,     0,     0,     0,     0,     0,     0,   430,
     833,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,     0,   453,   484,   485,     0,     0,     0,
       0,   181,     0,     0,    84,     0,   454,    86,    87,    36,
      88,   182,    90,     0,   830,  1187,     0,     0,     0,     0,
       0,     0,   833,     0,     0,     0,     0,     0,     0,     0,
      48,     0,   329,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,     0,
       0,     0,  1474,  1732,     0,     0,   329,     0,   329,     0,
     832,     0,     0,     0,   329,  1475,  1476,     0,     0,     0,
       0,   832,     0,     0,     0,     0,     0,   832,     0,     0,
       0,   832,     0,   181,     0,     0,    84,  1477,     0,    86,
      87,     0,    88,  1478,    90,     0,   219,     0,     0,   830,
     830,     0,     0,     0,     0,     0,   830,     0,     0,     0,
       0,     0,     0,     0,   329,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   832,  1506,     0,     0,     0,   427,   428,
     429,     0,  1801,     0,   219,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   430,  1470,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,     0,   453,     0,     0,   219,     0,   219,     0,     0,
       0,     0,    36,     0,   454,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   329,     0,     0,     0,     0,
       0,     0,     0,    48,   219,   833,     0,     0,     0,     0,
       0,     0,   329,     0,     0,     0,     0,     0,     0,     0,
     833,   833,   833,   833,   833,     0,     0,     0,   833,     0,
    1734,     0,     0,    36,     0,     0,   830,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   830,     0,     0,
       0,     0,     0,   830,    48,     0,     0,   830,     0,   376,
       0,     0,    86,    87,     0,    88,   182,    90,     0,     0,
       0,   219,     0,     0,     0,     0,     0,     0,     0,   329,
       0,     0,     0,     0,     0,     0,   219,   219,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,     0,     0,     0,   377,     0,     0,
     234,   414,     0,    86,    87,     0,    88,   182,    90,   830,
       0,     0,  1507,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   833,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,   329,   427,   428,   429,     0,     0,     0,
       0,     0,   234,     0,     0,     0,     0,     0,   329,     0,
       0,     0,     0,   430,  1354,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,    36,   453,     0,
       0,     0,     0,     0,     0,   219,   219,     0,     0,     0,
     454,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,   343,   344,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   833,
       0,   234,     0,     0,   833,   833,   833,   833,   833,   833,
     833,   833,   833,   833,   833,   833,   833,   833,   833,   833,
     833,   833,   833,   833,   833,   833,   833,   833,   833,   833,
     833,   833,     0,     0,   345,     0,     0,    86,    87,     0,
      88,   182,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   833,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   219,   732,    12,     0,     0,  1355,     0,     0,     0,
     733,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,   234,    28,    29,    30,    31,   219,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
     219,   219,    41,   833,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,   833,
       0,   833,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   177,   178,   179,     0,   833,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   180,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   181,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   182,    90,     0,
       0,   219,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,   334,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,   524,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   833,   234,
       0,     0,   833,     0,   833,    14,    15,   833,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,   234,
      57,     0,    59,    60,    61,    62,   177,   178,    65,     0,
      66,    67,    68,     0,   833,     0,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     181,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     182,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     0,   833,
     833,   427,   428,   429,     0,     0,   833,     0,     0,     0,
       0,     0,     0,     0,     0,  1693,     0,     0,     0,     0,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,     0,   453,     0,     0,     0,     0,
     427,   428,   429,     0,     0,     0,     0,   454,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,     0,   453,   427,   428,   429,     0,     0,
       0,     0,     0,     0,     0,     0,   454,     0,     0,     0,
       0,     0,     0,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,     0,   453,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   454,     0,     0,     0,     0,   833,     0,     0,     0,
       0,     5,     6,     7,     8,     9,     0,   833,     0,     0,
       0,    10,     0,   833,     0,     0,     0,   833,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,   455,     0,     0,     0,     0,     0,     0,
       0,  1776,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,   833,
      39,    40,   538,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,    69,    70,    71,     0,   540,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
      81,    82,    83,    84,    85,     0,    86,    87,     0,    88,
      89,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,    95,     0,    96,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,  1111,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
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
       0,    94,    95,     0,    96,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1285,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
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
       0,    96,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
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
       0,   181,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   182,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,   660,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
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
      79,     0,     0,    80,     0,     0,     0,     0,   181,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   182,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1084,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
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
      80,     0,     0,     0,     0,   181,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   182,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1127,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
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
       0,     0,   181,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   182,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1193,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,  1195,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   181,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   182,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,  1350,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,     0,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   181,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   182,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
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
       0,     0,     0,   181,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   182,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1461,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
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
     181,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     182,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,  1684,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
    1729,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   181,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   182,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,   181,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   182,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,  1764,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
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
       0,   181,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   182,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1765,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,  1768,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,     0,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   181,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   182,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
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
      80,     0,     0,     0,     0,   181,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   182,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1785,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
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
       0,     0,   181,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   182,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1841,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   181,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   182,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1848,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
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
       0,    80,     0,     0,     0,     0,   181,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   182,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,   798,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,   177,
     178,    65,     0,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   181,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   182,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,  1011,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,   177,   178,    65,     0,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     181,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     182,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,  1532,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   177,   178,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   181,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   182,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,  1676,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
     177,   178,    65,     0,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   181,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   182,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,   177,   178,    65,
       0,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   181,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   182,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   177,   178,   179,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   180,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   181,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   182,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,   334,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   335,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,     0,   453,   675,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   454,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   177,   178,   179,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   180,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   181,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   182,    90,     0,   676,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   177,   178,
     179,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   180,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   181,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   182,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,   793,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
       0,     0,  1140,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1070,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   177,   178,   179,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     180,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   181,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   182,
      90,     0,  1141,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,     0,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   732,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   177,   178,   179,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   180,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   181,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   182,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   427,   428,   429,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,     0,
     453,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   454,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,   193,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   177,
     178,   179,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   180,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   181,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   182,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,  1077,  1078,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,     0,     0,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,     0,   223,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   454,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   177,   178,   179,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   180,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     181,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     182,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     427,   428,   429,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,     0,   453,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   454,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   177,   178,   179,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   180,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   181,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   182,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,  1730,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   255,   428,   429,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   454,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     177,   178,   179,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   180,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   181,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   182,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   258,     0,   429,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,     0,   453,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     454,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   177,   178,   179,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   180,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   181,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   182,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
     522,     0,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   688,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   177,   178,   179,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   180,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   181,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   182,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,     0,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,     0,     0,     0,   733,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1070,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   177,   178,   179,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   180,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   181,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   182,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,     0,   453,     0,     0,   774,     0,     0,     0,     0,
       0,     0,     0,     0,   454,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   177,   178,
     179,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   180,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   181,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   182,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10, -1007, -1007, -1007, -1007,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,     0,   453,
       0,     0,   776,     0,     0,     0,     0,     0,     0,     0,
       0,   454,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   177,   178,   179,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     180,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   181,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   182,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,     0,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,     0,     0,     0,     0,  1185,
       0,     0,     0,     0,     0,     0,     0,     0,  1070,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   177,   178,   179,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   180,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   181,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   182,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   427,   428,   429,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   430,     0,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,     0,
     453,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   454,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   177,
     178,   179,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   180,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   181,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   182,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,  1540,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   427,   428,   429,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   430,     0,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,     0,   453,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,   454,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,   624,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   177,   178,   179,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   180,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     181,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     182,    90,     0,     0,     0,    92,     0,     0,    93,     0,
    1377,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
       0,     0,     0,   116,   117,     0,   118,   119,   260,   261,
       0,   262,   263,     0,  1046,   264,   265,   266,   267,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1047,   268,  1048,  1049,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,     0,     0,     0,     0,   270,
       0,     0,     0,     0,     0,     0,     0,     0,  1070,     0,
       0,     0,     0,   272,   273,   274,   275,   276,   277,   278,
       0,     0,     0,    36,     0,   210,     0,     0,     0,     0,
       0,     0,     0,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,    48,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,     0,     0,     0,
       0,   720,   313,   314,   315,     0,     0,     0,   316,   551,
     552,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   260,   261,     0,   262,   263,     0,   553,   264,   265,
     266,   267,     0,    86,    87,     0,    88,   182,    90,   321,
       0,   322,     0,     0,   323,   268,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   270,     0,     0,     0,     0,     0,   721,     0,
     116,     0,     0,     0,     0,     0,   272,   273,   274,   275,
     276,   277,   278,     0,     0,     0,    36,     0,   210,     0,
       0,     0,     0,     0,     0,     0,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,    48,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
       0,     0,     0,     0,   312,   313,   314,   315,     0,     0,
       0,   316,   551,   552,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   260,   261,     0,   262,   263,     0,
     553,   264,   265,   266,   267,     0,    86,    87,     0,    88,
     182,    90,   321,     0,   322,     0,     0,   323,   268,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   270,     0,     0,     0,     0,
       0,   721,     0,   116,     0,     0,     0,     0,     0,   272,
     273,   274,   275,   276,   277,   278,     0,     0,     0,    36,
       0,   210,     0,     0,     0,     0,     0,     0,     0,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
      48,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,     0,     0,     0,     0,  1121,   313,   314,
     315,     0,     0,     0,   316,   551,   552,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   260,   261,     0,
     262,   263,     0,   553,   264,   265,   266,   267,     0,    86,
      87,     0,    88,   182,    90,   321,     0,   322,     0,     0,
     323,   268,     0,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   270,     0,
     271,     0,     0,     0,     0,     0,   116,     0,     0,     0,
       0,     0,   272,   273,   274,   275,   276,   277,   278,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,    48,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,     0,     0,     0,     0,
       0,   313,   314,   315,    36,     0,     0,   316,   317,   318,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,   319,     0,     0,    84,
     320,     0,    86,    87,     0,    88,   182,    90,   321,     0,
     322,     0,     0,   323,     0,     0,     0,     0,     0,     0,
     324,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   260,   261,   325,   262,   263,     0,  1656,   264,
     265,   266,   267,     0,    86,    87,     0,    88,   182,    90,
       0,     0,     0,     0,     0,     0,   268,     0,   269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   270,     0,   271,   681,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   272,   273,   274,
     275,   276,   277,   278,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,    48,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,     0,     0,     0,     0,     0,   313,   314,   315,    36,
       0,     0,   316,   317,   318,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,   319,     0,     0,    84,   320,     0,    86,    87,     0,
      88,   182,    90,   321,     0,   322,     0,     0,   323,     0,
       0,     0,     0,     0,     0,   324,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   260,   261,   325,
     262,   263,     0,  1725,   264,   265,   266,   267,     0,    86,
      87,     0,    88,   182,    90,     0,     0,     0,     0,     0,
       0,   268,     0,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   270,     0,
     271,   941,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   272,   273,   274,   275,   276,   277,   278,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,    48,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,     0,     0,     0,     0,
     312,   313,   314,   315,    36,     0,     0,   316,   317,   318,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,   319,     0,     0,    84,
     320,     0,    86,    87,     0,    88,   182,    90,   321,     0,
     322,     0,     0,   323,     0,     0,     0,     0,     0,     0,
     324,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   260,   261,   325,   262,   263,     0,     0,   264,
     265,   266,   267,     0,    86,    87,     0,    88,   182,    90,
       0,     0,     0,     0,     0,     0,   268,     0,   269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   270,     0,   271,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   272,   273,   274,
     275,   276,   277,   278,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,    48,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,     0,     0,     0,     0,     0,   313,   314,   315,     0,
       0,     0,   316,   317,   318,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   319,     0,     0,    84,   320,     0,    86,    87,     0,
      88,   182,    90,   321,     0,   322,     0,     0,   323,     0,
       0,     0,     0,     0,     0,   324,  1465,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   260,   261,   325,
     262,   263,     0,     0,   264,   265,   266,   267,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   268,   430,   269,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,     0,   453,   270,     0,
     271,     0,     0,     0,     0,     0,     0,     0,     0,   454,
       0,     0,   272,   273,   274,   275,   276,   277,   278,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,    48,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,     0,     0,     0,     0,
       0,   313,   314,   315,     0,     0,    36,   316,   317,   318,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   319,    48,     0,    84,
     320,     0,    86,    87,     0,    88,   182,    90,   321,     0,
     322,     0,     0,   323,     0,     0,     0,     0,     0,     0,
     324,     0,  1487,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,   325,  1557,  1558,  1559,  1560,  1561,
       0,     0,  1562,  1563,  1564,  1565,    86,    87,     0,    88,
     182,    90,     0,     0,     0,     0,     0,     0,     0,  1566,
    1567,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,  1568,     0,  1488,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1569,  1570,  1571,  1572,  1573,  1574,  1575,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1576,  1577,  1578,  1579,  1580,  1581,  1582,  1583,  1584,  1585,
    1586,    48,  1587,  1588,  1589,  1590,  1591,  1592,  1593,  1594,
    1595,  1596,  1597,  1598,  1599,  1600,  1601,  1602,  1603,  1604,
    1605,  1606,  1607,  1608,  1609,  1610,  1611,  1612,  1613,  1614,
    1615,  1616,     0,     0,     0,  1617,  1618,     0,  1619,  1620,
    1621,  1622,  1623,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1624,  1625,  1626,     0,     0,     0,
      86,    87,     0,    88,   182,    90,  1627,     0,  1628,  1629,
       0,  1630,     0,     0,     0,     0,     0,     0,  1631,  1632,
       0,  1633,     0,  1634,  1635,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   260,
     261,     0,   262,   263,     0,     0,   264,   265,   266,   267,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1047,   268,  1048,  1049,  1050,  1051,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,     0,     0,     0,     0,
     270,     0,     0,     0,     0,     0,     0,     0,     0,  1070,
       0,     0,     0,     0,   272,   273,   274,   275,   276,   277,
     278,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,    48,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,     0,     0,
       0,     0,   312,   313,   314,   315,     0,     0,     0,   316,
     551,   552,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   260,   261,     0,   262,   263,     0,   553,   264,
     265,   266,   267,     0,    86,    87,     0,    88,   182,    90,
     321,     0,   322,     0,     0,   323,   268,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   270,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   272,   273,   274,
     275,   276,   277,   278,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,    48,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,     0,     0,     0,     0,  1228,   313,   314,   315,     0,
       0,     0,   316,   551,   552,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   260,   261,     0,   262,   263,
       0,   553,   264,   265,   266,   267,     0,    86,    87,     0,
      88,   182,    90,   321,     0,   322,     0,     0,   323,   268,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   270,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     272,   273,   274,   275,   276,   277,   278,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,    48,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,     0,     0,     0,     0,     0,   313,
     314,   315,     0,     0,     0,   316,   551,   552,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   553,     0,     0,     0,     0,     0,
      86,    87,     0,    88,   182,    90,   321,     0,   322,     0,
       0,   323,     0,     0,     0,     0,     0,     0,     0,     0,
     427,   428,   429,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,     0,   453,   427,   428,   429,     0,     0,
       0,     0,     0,     0,     0,     0,   454,     0,     0,     0,
       0,     0,     0,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,     0,   453,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,   454,     0,     0,     0,     0,     0,     0,     0,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,     0,   453,   427,   428,   429,     0,     0,
       0,     0,     0,     0,     0,     0,   454,     0,     0,     0,
       0,     0,     0,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,     0,   453,
     427,   428,   429,     0,     0,     0,     0,     0,     0,     0,
       0,   454,   558,     0,     0,     0,     0,     0,   269,   430,
       0,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,     0,   453,   271,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   454,   582,     0,     0,
       0,     0,     0,     0,   269,     0,     0,    36, -1007, -1007,
   -1007, -1007,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,     0,     0,     0,    48,     0,
       0,   271,     0,     0,     0,     0,  -398,     0,  1070,     0,
       0,   766,     0,     0,    60,    61,    62,   177,   178,   417,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   541,   542,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,   269,
       0,   181,   547,     0,    84,   320,   790,    86,    87,     0,
      88,   182,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   324,   271,     0,     0,   541,
     542,   418,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,  1081,   181,    36,   325,
      84,   320,     0,    86,    87,     0,    88,   182,    90,     0,
       0,     0,     0,     0,     0,     0,   269,     0,     0,    48,
       0,   324,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,   271,     0,   325,     0,     0,     0,     0,
       0,     0,     0,     0,   541,   542,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,   181,     0,     0,    84,   320,     0,    86,    87,
       0,    88,   182,    90,     0,  1026,    48,     0,     0,     0,
       0,   269,     0,     0,     0,     0,   324,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,   271,     0,
     325,   541,   542,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      36,     0,    84,   320,     0,    86,    87,     0,    88,   182,
      90,     0,  1361,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,   324,     0,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,   325,     0,     0,
       0,     0,     0,     0,     0,     0,   541,   542,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   181,     0,     0,    84,   320,     0,
      86,    87,     0,    88,   182,    90,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   324,     0,
       0,     0,  1236,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   819,
     820,     0,   325,     0,     0,   821,     0,   822,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   823,
       0,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,   427,   428,   429,     0,     0,     0,     0,     0,   211,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     430,    48,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,     0,   453,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   824,   454,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,   181,    82,    83,    84,   825,     0,
      86,    87,     0,    88,   182,    90,     0,  1005,     0,    92,
       0,     0,     0,     0,     0,     0,     0,     0,   826,     0,
       0,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    28,
       0,     0,   827,   500,     0,     0,     0,    33,    34,    35,
      36,     0,   210,     0,     0,     0,     0,     0,     0,   211,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   212,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1006,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,   181,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   182,    90,     0,     0,     0,    92,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
     819,   820,   214,     0,     0,     0,   821,   116,   822,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     823,     0,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,   427,   428,   429,     0,     0,     0,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   430,    48,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,     0,   453,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   824,   454,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,   181,    82,    83,    84,   825,
       0,    86,    87,     0,    88,   182,    90,     0,     0,     0,
      92,     0,     0,     0,     0,     0,     0,     0,     0,   826,
     972,   973,     0,     0,    97,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     974,     0,     0,   827,   509,     0,     0,     0,   975,   976,
     977,    36,   427,   428,   429,     0,     0,     0,     0,     0,
     978,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   430,    48,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,     0,   453,     0,     0,     0,
       0,     0,     0,     0,    36,     0,     0,   979,   454,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     980,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,    86,    87,     0,    88,   182,    90,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   981,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      28,     0,     0,     0,   947,     0,     0,     0,    33,    34,
      35,    36,  1641,   210,    86,    87,  1642,    88,   182,    90,
     211,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   212,     0,     0,  1488,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,   181,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   182,    90,     0,     0,     0,
      92,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,    28,     0,   214,     0,     0,   595,     0,   116,    33,
      34,    35,    36,     0,   210,     0,     0,     0,     0,     0,
       0,   211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   615,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   213,     0,     0,     0,     0,   181,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   182,    90,     0,     0,
       0,    92,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,    28,   214,   961,     0,     0,     0,   116,
       0,    33,    34,    35,    36,     0,   210,     0,     0,     0,
       0,     0,     0,   211,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   212,   453,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   454,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   213,     0,     0,     0,     0,   181,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   182,    90,
       0,     0,     0,    92,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    97,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,    28,     0,   214,     0,     0,     0,
       0,   116,    33,    34,    35,    36,     0,   210,     0,     0,
       0,     0,     0,     0,   211,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1106,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   213,     0,     0,     0,     0,   181,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   182,
      90,     0,     0,     0,    92,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,    28,     0,   214,     0,     0,
       0,     0,   116,    33,    34,    35,    36,     0,   210,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,   212,     0,
       0,     0,  1167,  1168,  1169,    36,     0,     0,     0,     0,
       0,  1070,    73,     0,    74,    75,    76,    77,    78,     0,
       0,    36,     0,     0,     0,   213,    48,     0,     0,     0,
     181,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     182,    90,    48,     0,     0,    92,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   214,    33,
      34,    35,    36,   116,   210,    86,    87,     0,    88,   182,
      90,   211,     0,     0,     0,   181,     0,     0,    84,    85,
       0,    86,    87,    48,    88,   182,    90,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   228,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      74,    75,    76,    77,    78,     0,     0,    36,     0,     0,
       0,   213,     0,     0,     0,     0,   181,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   182,    90,    48,     0,
       0,    92,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,   229,    33,    34,    35,    36,   116,
     210,     0,     0,     0,     0,     0,     0,   638,     0,     0,
       0,     0,     0,     0,   583,     0,     0,    86,    87,    48,
      88,   182,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     212,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   213,     0,     0,
       0,     0,   181,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   182,    90,     0,     0,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    97,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   427,   428,   429,
     639,     0,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,   427,   428,   429,     0,     0,     0,     0,     0,
       0,     0,     0,   454,     0,     0,     0,     0,     0,     0,
       0,   430,     0,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,     0,   453,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   454,  1044,
    1045,  1046,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1047,   991,
    1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,     0,     0,  1044,  1045,  1046,     0,     0,     0,
       0,     0,     0,     0,     0,  1070,     0,     0,     0,     0,
       0,     0,     0,  1047,  1314,  1048,  1049,  1050,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1070,  1044,  1045,  1046,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1047,  1218,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,     0,     0,  1044,  1045,  1046,     0,
       0,     0,     0,     0,     0,     0,     0,  1070,     0,     0,
       0,     0,     0,     0,     0,  1047,  1371,  1048,  1049,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1070,    36,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1453,    48,     0,     0,     0,     0,     0,
       0,     0,  1474,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1475,  1476,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   181,     0,     0,    84,    85,  1539,    86,
      87,    48,    88,  1478,    90,     0,     0,     0,     0,     0,
     587,     0,     0,    86,    87,     0,    88,   182,    90,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,     0,     0,   345,     0,     0,
      86,    87,     0,    88,   182,    90,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   427,   428,   429,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   802,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,     0,   453,   427,   428,   429,     0,
       0,     0,     0,     0,     0,     0,     0,   454,     0,     0,
       0,     0,     0,     0,     0,   430,   944,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   803,
     453,   427,   428,   429,     0,     0,     0,     0,     0,     0,
       0,     0,   454,     0,     0,     0,     0,     0,     0,     0,
     430,     0,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,     0,   453,  1044,  1045,  1046,     0,
       0,     0,     0,     0,     0,     0,     0,   454,     0,     0,
       0,     0,     0,     0,     0,  1047,  1376,  1048,  1049,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1044,
    1045,  1046,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1070,     0,     0,     0,     0,     0,  1047,     0,
    1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1070,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,     0,   453,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   454,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1070,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,     0,
     453,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   454,  1049,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1070
};

static const yytype_int16 yycheck[] =
{
       5,     6,   184,     8,     9,    10,    11,    12,   135,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    91,     4,    28,    29,    95,    96,   160,   333,    32,
       4,   678,     4,     4,     4,     4,    55,    42,    30,   469,
     650,    44,   384,   230,   768,    50,    49,    52,   226,   113,
      55,   920,    57,   165,    30,   524,    54,   494,   185,   649,
    1130,   384,   453,   909,   134,   160,   631,    30,   940,   729,
     486,   486,   113,   113,    79,   808,   239,  1004,     9,     9,
      42,     9,  1115,   801,   956,    30,  1126,     9,   490,   491,
       9,     9,    14,     9,    67,     9,    14,     9,    58,     4,
      14,     9,    33,     9,   520,   520,     9,    30,   113,     9,
     579,    46,     9,    46,    35,    80,   518,    79,     9,     9,
       9,    81,     9,    67,    84,     9,     9,    67,   240,     9,
       9,    67,    80,     9,    80,     4,     9,    46,     9,     9,
      51,    46,    98,   111,     9,     9,     9,   102,   103,   151,
     214,   157,   172,     9,    46,   460,    80,   130,   131,    80,
      67,    14,   172,    80,   116,   229,   151,   200,     9,    98,
       0,   118,   200,   214,   214,   517,   172,    30,   160,   126,
     200,    67,   651,   200,   189,    46,  1670,   172,   229,     8,
     200,     4,   172,   200,  1040,    48,   200,   203,   200,   155,
     165,   169,   507,   108,   200,    67,   369,   151,   113,   214,
     115,   116,   117,   118,   119,   120,   121,    67,    67,   165,
     200,    67,    80,    67,   229,    67,   155,    67,    67,    67,
     203,   156,    67,    46,    67,    67,   241,   419,    67,   244,
      67,   165,  1726,    80,   165,   156,   251,   252,   165,   204,
     172,   200,    80,   158,   159,   203,   161,   205,   203,   203,
     200,    26,    27,   198,   200,   198,   135,   203,   201,  1309,
     201,   245,   202,  1200,   202,   249,   173,   182,  1311,   202,
     202,  1014,   201,  1016,   202,  1318,   202,  1320,   202,  1161,
     202,   203,   201,   200,   202,   108,   202,   361,   201,   204,
     113,   201,   115,   116,   117,   118,   119,   120,   121,   201,
     173,   202,   202,   202,   496,   202,   185,   173,   202,   202,
     361,   361,   202,   202,   793,   503,   202,   937,   201,   798,
     201,   201,   173,   898,  1180,   333,   201,   201,   200,   201,
     201,   468,    80,   413,    67,   158,   159,   205,   161,    80,
     200,   200,   155,   203,   203,   360,   361,   203,   200,   203,
     197,   203,   367,   203,   203,   203,   203,   372,   203,   182,
     203,   203,   203,   172,   203,   119,   203,   205,   200,  1419,
     200,  1421,   126,   343,   344,   345,   200,   102,   103,   130,
     131,   204,   397,   463,   464,   465,   466,  1430,   360,   202,
     203,   200,   200,    54,   409,  1385,   200,   130,   131,    51,
      35,   200,   410,   151,   396,    35,   376,    67,   149,   150,
     390,   200,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   885,   454,
     469,   456,   457,  1113,   459,    80,    86,  1181,   223,   200,
      80,   453,   460,   641,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   453,  1196,  1519,
       4,    35,   487,   488,    30,   490,   491,   492,   493,   204,
     453,  1471,   497,   656,   657,   500,     4,    98,   130,   131,
      86,   151,    26,    27,   509,   200,   511,   526,   460,   507,
     926,   926,    54,   518,   156,  1495,   155,  1497,   148,    98,
      98,   526,  1368,   528,    66,   517,    80,   396,    35,   171,
     202,     4,   202,   666,   486,  1145,   723,    35,  1148,  1129,
     165,   719,  1011,    51,   202,   165,    54,    98,   202,   731,
     200,   953,    80,   203,   155,   507,   561,   531,    86,   564,
     565,   566,   148,    71,   202,   517,   157,   909,   520,   756,
     961,   666,   204,    46,    35,   639,   155,   155,   202,   170,
     202,    89,    80,    91,    67,   350,   909,    95,    96,   113,
     595,    67,  1438,    80,   359,    67,   361,   203,   467,    86,
      80,   366,   151,  1327,   155,  1329,    86,    29,   373,   772,
     773,   165,   203,   200,   684,   778,   779,   200,   269,    80,
     271,   149,   150,   583,    67,    47,   134,   587,    50,   151,
     151,   200,   592,  1673,   639,   108,   155,   815,   202,    29,
     113,   828,   115,   116,   117,   118,   119,   120,   121,   836,
      80,   172,   130,   131,   130,   131,    86,    47,    45,  1319,
      50,   148,   149,   150,    80,    66,   778,   457,   172,   149,
     150,   676,   151,     4,   325,   151,    47,    48,  1108,   200,
     650,   209,   203,   688,   666,   158,   159,   200,   161,   197,
     214,   115,   116,   117,   118,   119,   120,   487,  1040,   223,
      72,    73,   492,  1313,   200,   229,    72,    73,  1298,   182,
      98,    99,  1436,   128,   129,    46,   721,  1040,   148,   149,
     150,   245,   207,   151,   200,   249,     9,   732,   208,   494,
     151,   204,   148,   149,   150,   202,   203,   245,   202,   203,
    1400,   249,  1812,   151,   172,   253,   751,    80,  1700,  1701,
      26,    27,  1154,    86,    30,   200,   407,  1827,   182,   410,
       8,   107,  1199,  1165,  1696,  1697,  1806,   817,   818,   115,
     116,   117,   118,   119,   120,   203,   202,   108,   202,    14,
     785,  1821,   113,   200,   115,   116,   117,   118,   119,   120,
     121,    80,   151,    80,   797,   151,   801,    86,    80,    86,
    1269,   126,  1526,    75,    76,    77,   202,   202,   986,   791,
     126,    14,  1472,    14,   172,    87,   149,   150,    50,    51,
      52,  1431,    54,   201,    98,   333,   350,   158,   159,   206,
     161,   201,   806,   838,    66,   359,   182,   361,  1180,    50,
      51,    52,   366,  1280,   101,   102,   103,   201,   201,   373,
     200,   182,  1030,   107,     9,    66,   200,  1180,   200,  1037,
     149,   150,   149,   150,   136,   137,   138,   139,   140,   148,
     852,  1340,   396,   204,   201,   147,   101,   102,   103,     9,
     201,   153,   154,   201,   115,   116,   117,  1356,   396,   894,
     960,   117,   118,   119,   545,   167,   201,   857,   406,    90,
     202,   861,   410,   908,    14,   413,   172,   200,     9,   181,
     675,  1780,    80,  1315,   912,   897,    47,    48,    49,    50,
      51,    52,   791,   897,   186,   897,   897,   897,   897,   934,
    1799,    80,    80,   189,   202,    66,   918,   200,  1807,   944,
       9,   202,   947,     9,   949,    80,   201,   223,   953,  1136,
     920,  1675,   460,   461,   462,   463,   464,   465,   466,   201,
     912,   201,   128,   202,   200,    67,    30,   937,   733,   961,
     494,  1440,   924,   201,   926,   129,   171,   151,   486,   132,
    1449,     9,   201,   852,   151,   961,   991,    14,   198,     9,
       9,  1460,   173,   201,   997,   646,   647,  1175,   961,   507,
       9,    14,   128,   207,   655,   207,   204,   531,     9,   774,
      14,   776,   520,   207,   201,  1417,   201,   207,  1000,   151,
    1002,   200,    98,   531,   998,   201,  1368,   202,   897,   202,
      87,   115,   116,   117,   118,   119,   120,   132,   803,   151,
     992,  1039,   550,     9,  1042,  1368,   151,  1225,   370,   918,
     201,   151,   374,   200,  1232,   115,   116,   117,   118,   119,
     120,   200,   203,  1532,   186,   186,   126,   127,    14,  1074,
    1075,  1076,     9,  1720,   350,  1080,  1081,   399,    80,   401,
     402,   403,   404,   359,   203,    14,   594,   202,    14,  1108,
     366,   207,   203,    26,    27,   203,  1438,   373,   182,    14,
     198,   200,   162,  1108,   164,   870,   202,   201,   384,    30,
     200,    30,   620,   621,    14,  1438,    14,   177,   200,   179,
     885,   886,   182,   115,   116,   117,   118,   119,   120,   200,
    1112,  1000,    49,  1002,   126,   127,  1141,   200,  1112,   200,
    1112,  1112,  1112,  1112,     9,   201,   132,   202,   202,  1154,
    1797,   675,    47,    48,    49,    50,    51,    52,  1336,    54,
    1165,  1166,   183,   184,   185,   200,   817,   818,    14,   190,
     191,    66,   164,   194,   195,  1145,   684,   453,  1148,   132,
       9,    66,   207,   201,     9,  1188,    80,     9,   200,   132,
     182,  1196,   202,    14,    80,   201,  1665,   202,  1667,   207,
     203,  1206,   200,   200,   203,   201,     9,  1676,   203,   733,
     132,    87,   148,    30,    74,   202,   201,  1191,   494,   202,
     132,  1203,   173,    30,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,   201,   201,  1004,
    1005,   132,     9,  1112,   201,   204,     9,   201,   899,   204,
     774,   203,   776,    14,  1723,   200,    80,   201,   132,   201,
     768,   201,   770,   202,   915,   203,   200,   791,  1401,  1396,
     201,   201,     9,   201,    64,    65,    30,   928,   202,   803,
     203,   201,   806,   791,   202,  1290,   202,   201,   108,  1294,
     223,  1296,   160,   202,   156,    14,  1278,   805,   806,  1304,
      80,   113,   201,    14,   201,  1287,   957,   203,  1278,  1314,
    1315,  1493,   201,   132,   132,   172,   203,  1287,   202,    14,
      80,    14,    80,   201,   203,   200,   132,    14,   852,   201,
      14,   202,   202,   202,  1203,    14,   203,     9,   204,    56,
     130,   131,    80,  1313,   852,  1650,   870,   200,    80,   172,
     858,     9,   202,    80,   862,   863,  1825,    98,   111,   151,
      33,   885,   886,  1832,    98,   163,    14,  1018,   200,   202,
    1021,   201,  1377,   897,   882,  1140,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,   897,
      54,   200,   169,   166,   918,  1465,   201,  1379,   173,   675,
      80,     9,    66,    80,   912,   202,   201,   201,    14,  1414,
     918,   201,  1417,    80,   203,    14,   924,   350,   926,  1401,
    1185,    80,    14,    80,    14,    80,   359,   592,   857,   861,
    1788,   461,   466,   366,  1199,  1200,   959,   463,  1420,   954,
     373,   900,  1803,  1197,  1426,  1531,  1428,  1799,  1522,  1353,
    1420,   597,   960,  1555,  1639,  1473,  1426,   733,  1428,  1441,
    1831,  1431,  1819,  1651,   972,   973,   974,  1518,  1450,   469,
    1468,  1391,  1446,  1655,   998,  1083,  1000,  1128,  1002,  1130,
    1004,  1005,   568,  1080,   992,  1387,   849,   568,  1116,  1163,
     998,  1038,  1000,  1176,  1002,  1177,   973,   988,   774,   924,
     776,  1755,   410,   367,   817,  1510,  1157,  1099,  1380,  1160,
    1379,  1023,  1071,  1691,  1022,  1280,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1395,   803,    -1,    -1,
      -1,  1039,    -1,    -1,  1042,  1540,    -1,    -1,    -1,    -1,
      -1,    -1,  1524,    -1,    -1,    -1,    -1,  1529,    -1,    -1,
      -1,    -1,    -1,  1535,  1524,    -1,  1530,  1531,    -1,  1541,
    1211,   494,    -1,  1071,  1215,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1441,    -1,    -1,    -1,    -1,     4,    -1,    -1,
    1088,  1450,    -1,  1091,    -1,    -1,    -1,    -1,  1112,    -1,
      -1,    -1,    -1,    -1,   870,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     4,  1112,    -1,    -1,    -1,    -1,   885,
     886,    -1,    -1,    -1,    -1,    -1,  1140,    -1,    -1,    46,
      -1,    -1,    -1,  1274,  1275,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1501,   909,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1774,    -1,    -1,  1714,    46,    -1,    -1,    -1,    -1,
      26,    27,  1650,    -1,    30,  1660,    -1,    -1,    -1,    -1,
    1529,  1185,    -1,    -1,    -1,  1792,  1535,  1191,    -1,  1177,
      -1,    -1,  1541,  1181,    -1,  1199,  1200,    -1,    54,  1203,
      -1,   108,    -1,  1191,    -1,   961,   113,    -1,   115,   116,
     117,   118,   119,   120,   121,  1203,    -1,    -1,  1680,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,  1360,
      -1,  1362,   113,    -1,   115,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1004,  1005,
      -1,   158,   159,    -1,   161,    -1,    -1,    -1,    -1,  1721,
    1722,    -1,   675,    -1,    -1,  1396,  1728,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,  1280,   158,   159,    -1,
     161,    -1,    -1,    -1,  1040,  1273,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   204,    -1,    -1,
      -1,   182,    -1,    -1,  1766,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1774,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     733,    -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,
    1780,  1680,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1327,
      -1,  1329,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1799,
      -1,    -1,  1837,    -1,    -1,    -1,    -1,  1807,    -1,    -1,
    1845,   774,    -1,   776,    -1,    -1,  1851,   223,    -1,  1854,
      -1,  1833,  1721,  1722,    -1,  1379,    -1,    -1,  1840,  1728,
      -1,    -1,    -1,    -1,  1140,    -1,    -1,    -1,    -1,    -1,
     803,  1379,    -1,    -1,    -1,    -1,    -1,  1385,    -1,    -1,
      -1,    -1,    -1,  1391,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   269,    -1,   271,    -1,  1766,    -1,    -1,
      -1,    -1,  1553,    -1,  1180,    -1,    -1,    -1,    -1,  1185,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1441,    -1,    -1,
      -1,    -1,  1446,  1199,  1200,    -1,  1450,    -1,  1436,    -1,
      -1,    -1,    -1,  1441,    -1,    -1,    -1,   870,  1446,    -1,
      -1,    -1,  1450,    -1,    -1,    -1,    -1,    -1,    -1,   325,
      -1,    -1,   885,   886,    -1,    -1,    -1,  1465,    -1,    -1,
    1468,    -1,    -1,  1471,  1833,    -1,    -1,    -1,    -1,    -1,
      -1,  1840,    -1,  1481,   350,    -1,    -1,    -1,    -1,    -1,
    1488,    -1,    -1,   359,    -1,    -1,    -1,  1495,    -1,  1497,
     366,    -1,    -1,    -1,    -1,  1503,    -1,   373,    78,    -1,
      80,    81,    -1,  1654,  1280,  1529,  1530,  1531,   384,    -1,
      -1,  1535,    -1,    -1,    -1,    -1,    -1,  1541,  1526,    99,
      -1,  1529,  1530,  1531,    10,    11,    12,  1535,    -1,    -1,
      -1,   407,    -1,  1541,   410,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,  1004,  1005,    -1,    -1,    -1,    -1,   453,   158,   159,
      66,   161,   162,   163,    -1,    -1,    -1,  1738,    -1,    -1,
      -1,    -1,  1368,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,   549,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    29,   494,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,  1650,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,   601,  1680,    -1,    -1,    -1,
      -1,  1812,  1438,  1671,    -1,    -1,    -1,  1675,    -1,   545,
     546,    -1,  1680,   549,    -1,    -1,  1827,    -1,    29,    -1,
      -1,  1689,    -1,    -1,    -1,    -1,    -1,    -1,  1696,  1697,
      -1,    -1,  1700,  1701,    -1,    -1,    -1,  1721,  1722,    -1,
      -1,    -1,    -1,    -1,  1728,    -1,  1714,  1140,    -1,    -1,
      -1,    -1,    -1,  1721,  1722,    -1,    -1,    -1,    -1,    -1,
    1728,   207,    -1,    -1,    -1,   601,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,  1766,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,  1185,    -1,    -1,    -1,    -1,    -1,  1766,    -1,
      -1,    -1,    -1,    -1,    -1,  1773,  1199,  1200,    -1,    -1,
     646,   647,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   655,
      -1,    -1,    -1,  1791,    -1,    -1,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   675,
      -1,   152,    -1,    -1,   155,    -1,    -1,   158,   159,  1833,
     161,   162,   163,    -1,    -1,    -1,  1840,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1833,    -1,    -1,    -1,    -1,
      -1,    -1,  1840,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,  1280,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   733,   804,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   819,   820,   821,   822,   823,    -1,    -1,
      -1,   827,    10,    11,    12,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   774,    -1,
     776,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,   803,   804,    -1,
      -1,    10,    11,    12,    -1,    64,    65,    -1,    66,    -1,
      -1,   817,   818,   819,   820,   821,   822,   823,    -1,    -1,
      29,   827,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,   853,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
     936,    -1,    -1,    -1,   870,    -1,    -1,    -1,    -1,    -1,
      -1,   130,   131,    -1,    -1,    -1,    -1,   883,    -1,   885,
     886,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    27,   899,   900,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   909,    -1,    -1,    -1,    -1,    -1,   915,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   928,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     936,    -1,    -1,   939,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   201,    -1,    -1,    -1,   204,    -1,    -1,    -1,
      -1,   957,    -1,    -1,    -1,   961,    -1,    -1,    -1,    -1,
      -1,    -1,  1038,    -1,    -1,    -1,    -1,  1043,  1044,  1045,
    1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,   204,    -1,    -1,  1004,  1005,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1018,    -1,    -1,  1021,    -1,  1023,    -1,    -1,
    1096,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1038,    -1,  1040,    -1,    -1,  1043,  1044,  1045,
    1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,    -1,    -1,   549,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   223,    -1,
    1096,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    54,    54,    -1,    -1,   601,
      -1,    -1,  1128,    -1,  1130,    -1,  1202,    -1,    66,    -1,
      -1,    -1,    -1,    -1,  1140,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1218,    -1,  1220,    -1,    -1,    -1,    -1,    -1,
      -1,  1157,    -1,    -1,  1160,    -1,    -1,    -1,    -1,    -1,
    1236,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,  1180,    -1,    -1,    -1,    -1,  1185,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,  1199,  1200,    -1,  1202,    -1,    78,    -1,
      80,    -1,    -1,    -1,    -1,  1211,    -1,    -1,    -1,  1215,
      -1,    -1,  1218,    -1,  1220,   350,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,   359,    -1,    -1,    -1,    -1,    -1,
    1236,   366,    -1,    -1,    -1,    -1,    -1,    -1,   373,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   384,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,   204,    -1,  1274,  1275,
      -1,  1277,    -1,    -1,  1280,    -1,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1367,    -1,    -1,    -1,  1371,    -1,  1373,    -1,    -1,
    1376,    -1,    64,    65,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,   453,    -1,
      -1,    -1,   804,   203,    -1,   205,    -1,    -1,    -1,    -1,
     269,    -1,   271,    -1,    -1,    -1,    -1,   819,   820,   821,
     822,    -1,    -1,    -1,    -1,   827,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1360,    -1,  1362,    -1,    -1,   494,
      -1,  1367,  1368,    -1,    -1,  1371,    -1,  1373,   130,   131,
    1376,    11,    12,    -1,    -1,    -1,    -1,  1453,    -1,  1385,
    1386,    -1,    -1,  1389,    -1,    -1,   325,    -1,    -1,    29,
    1396,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,   549,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,  1438,    -1,    -1,    -1,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1453,    -1,    -1,
      -1,    -1,    -1,    -1,   936,    -1,    -1,  1463,  1464,    -1,
      -1,    -1,  1538,  1539,    -1,  1471,   601,  1473,   407,  1545,
      -1,   410,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1495,
      -1,  1497,    -1,    -1,    -1,    -1,    -1,  1503,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1538,  1539,    -1,    66,    -1,    -1,    -1,  1545,
     675,  1547,    -1,    -1,    -1,    -1,    -1,  1553,    -1,  1555,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1043,  1044,  1045,  1046,  1047,  1048,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   733,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   545,   546,    -1,    -1,
     549,    -1,    -1,    -1,  1096,    -1,    -1,    -1,    -1,  1695,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1706,    -1,    -1,    -1,    -1,    -1,  1712,    -1,    -1,   774,
    1716,   776,    -1,    -1,    -1,    -1,    -1,    -1,  1654,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   601,    -1,    -1,  1671,    -1,    -1,   803,   804,
      -1,    -1,    -1,   204,    -1,    -1,    -1,    -1,    78,    -1,
      80,    -1,    -1,  1689,   819,   820,   821,   822,   823,  1695,
      -1,    -1,   827,    -1,    -1,    -1,    -1,    -1,    -1,    99,
    1706,    -1,  1778,    -1,    -1,    -1,  1712,   646,   647,    -1,
    1716,    -1,    -1,    -1,    -1,    -1,   655,    -1,   853,    -1,
    1202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1738,    -1,    -1,   870,  1218,    -1,  1220,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   883,    -1,
     885,   886,    -1,    -1,  1236,    -1,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,   900,    -1,    -1,    -1,    -1,
      -1,    -1,  1778,    -1,   909,    -1,    -1,    -1,    -1,    -1,
      -1,  1787,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,  1803,    -1,    -1,
      -1,   936,    -1,   203,   939,   205,  1812,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1827,    -1,    -1,    -1,    -1,   961,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,   804,    -1,    -1,    -1,  1004,
    1005,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   817,   818,
     819,   820,   821,   822,   823,  1367,    -1,    -1,   827,  1371,
      -1,  1373,    -1,    -1,  1376,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1038,    -1,  1040,    -1,    -1,  1043,  1044,
    1045,  1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    27,    -1,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     899,  1096,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1453,    -1,    -1,    -1,    -1,   915,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   928,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   936,    10,    11,
      12,    -1,   204,    -1,    -1,  1140,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   957,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,  1180,    -1,    -1,    -1,    -1,
    1185,    -1,    -1,    -1,    66,    -1,  1538,  1539,    -1,    -1,
      -1,    -1,    -1,  1545,  1199,  1200,    -1,  1202,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1018,
      -1,    -1,  1021,  1218,  1023,  1220,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1038,
      -1,  1236,    -1,    -1,  1043,  1044,  1045,  1046,  1047,  1048,
    1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,  1277,    -1,   223,  1280,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,  1096,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1128,
      -1,  1130,   204,    66,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,  1695,    -1,    -1,    -1,    -1,  1157,    -1,
      -1,  1160,    -1,    -1,  1706,    -1,    -1,    -1,    -1,    -1,
    1712,    -1,  1367,  1368,  1716,    -1,  1371,    -1,  1373,    -1,
      -1,  1376,    -1,    -1,    64,    65,    -1,    -1,    -1,    -1,
      -1,  1386,    -1,    -1,  1389,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1202,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   350,  1211,    -1,    -1,    -1,  1215,    -1,    -1,  1218,
     359,  1220,    -1,    -1,    -1,    -1,    -1,   366,    -1,    -1,
      -1,    -1,    -1,    -1,   373,    -1,  1778,  1236,    -1,    -1,
      -1,    -1,    -1,  1438,    -1,   384,    -1,    -1,    -1,    -1,
     130,   131,    -1,    -1,    -1,    -1,    -1,    -1,  1453,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,  1463,  1464,
      -1,   204,    -1,    -1,    -1,  1274,  1275,    -1,  1473,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   453,    -1,    -1,    66,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1538,  1539,    -1,    -1,    -1,    -1,    -1,
    1545,    -1,  1547,    -1,    -1,   494,    -1,    -1,    -1,    -1,
    1555,  1360,    -1,  1362,    -1,    -1,    -1,    78,  1367,    64,
      65,    -1,  1371,    -1,  1373,    -1,    -1,  1376,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1385,    -1,    99,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,  1396,    -1,    -1,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     549,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,   130,   131,    -1,    -1,    -1,
      -1,   152,    -1,    -1,   155,    -1,    66,   158,   159,    78,
     161,   162,   163,    -1,  1453,   204,    -1,    -1,    -1,    -1,
      -1,    -1,   601,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,  1471,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    -1,    -1,
      -1,    -1,   121,   204,    -1,    -1,  1495,    -1,  1497,    -1,
    1695,    -1,    -1,    -1,  1503,   134,   135,    -1,    -1,    -1,
      -1,  1706,    -1,    -1,    -1,    -1,    -1,  1712,    -1,    -1,
      -1,  1716,    -1,   152,    -1,    -1,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,   675,    -1,    -1,  1538,
    1539,    -1,    -1,    -1,    -1,    -1,  1545,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1553,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1778,   204,    -1,    -1,    -1,    10,    11,
      12,    -1,  1787,    -1,   733,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1803,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,   774,    -1,   776,    -1,    -1,
      -1,    -1,    78,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1654,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,   803,   804,    -1,    -1,    -1,    -1,
      -1,    -1,  1671,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     819,   820,   821,   822,   823,    -1,    -1,    -1,   827,    -1,
    1689,    -1,    -1,    78,    -1,    -1,  1695,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1706,    -1,    -1,
      -1,    -1,    -1,  1712,    99,    -1,    -1,  1716,    -1,   155,
      -1,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,   870,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1738,
      -1,    -1,    -1,    -1,    -1,    -1,   885,   886,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,
     909,   156,    -1,   158,   159,    -1,   161,   162,   163,  1778,
      -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   936,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,  1812,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,   961,    -1,    -1,    -1,    -1,    -1,  1827,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    78,    54,    -1,
      -1,    -1,    -1,    -1,    -1,  1004,  1005,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   107,   108,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1038,
      -1,  1040,    -1,    -1,  1043,  1044,  1045,  1046,  1047,  1048,
    1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,    -1,    -1,   155,    -1,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1096,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1140,    27,    28,    -1,    -1,   202,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,  1180,    67,    68,    69,    70,  1185,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,
    1199,  1200,    87,  1202,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,  1218,
      -1,  1220,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     115,   116,   117,   118,   119,   120,    -1,  1236,   123,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,  1280,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,
     205,   206,    -1,   208,   209,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1367,  1368,
      -1,    -1,  1371,    -1,  1373,    47,    48,  1376,    -1,    -1,
      -1,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    71,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    -1,
      82,    83,    -1,    -1,    -1,    87,    88,    89,    90,    -1,
      92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,    -1,
      -1,    -1,   104,   105,   106,   107,    -1,   109,   110,  1438,
     112,    -1,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,   123,   124,    -1,  1453,    -1,    -1,    -1,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,   167,    -1,    -1,   170,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
     202,   203,    -1,   205,   206,    -1,   208,   209,    -1,  1538,
    1539,    10,    11,    12,    -1,    -1,  1545,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1554,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,  1695,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,  1706,    -1,    -1,
      -1,    13,    -1,  1712,    -1,    -1,    -1,  1716,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1740,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,
      -1,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    71,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,  1778,
      82,    83,   202,    -1,    -1,    87,    88,    89,    90,    -1,
      92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,    -1,
      -1,    -1,   104,   105,   106,   107,   108,   109,   110,    -1,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,    -1,   202,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,   164,    -1,    -1,   167,    -1,    -1,   170,    -1,
      -1,    -1,    -1,    -1,   176,   177,    -1,   179,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
     202,   203,   204,   205,   206,    -1,   208,   209,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    71,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    80,    -1,    82,    83,    -1,
      -1,    -1,    87,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    96,    -1,    -1,    99,   100,    -1,    -1,    -1,   104,
     105,   106,   107,   108,   109,   110,    -1,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    -1,   122,   123,   124,
     125,   126,   127,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,   144,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,   164,
      -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,
      -1,   176,   177,    -1,   179,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,   202,   203,   204,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    71,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    80,    -1,    82,    83,    -1,    -1,    -1,    87,
      88,    89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,
      -1,    99,   100,    -1,    -1,    -1,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,   123,   124,   125,   126,   127,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,   164,    -1,    -1,   167,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,   177,
      -1,   179,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,   202,   203,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,    -1,
      -1,    -1,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
      -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,
      -1,    -1,    -1,   104,   105,   106,   107,    -1,   109,   110,
      -1,   112,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,   122,   123,   124,    -1,   126,   127,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,   170,
      -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,   202,   203,   204,   205,   206,    -1,   208,   209,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,    53,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    71,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    80,    -1,    82,    83,
      -1,    -1,    -1,    87,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,    -1,
     104,   105,   106,   107,    -1,   109,   110,    -1,   112,    -1,
     114,   115,   116,   117,   118,   119,   120,    -1,   122,   123,
     124,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
     144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
     164,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,   202,   203,
     204,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    71,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    80,    -1,    82,    83,    -1,    -1,    -1,
      87,    88,    89,    90,    -1,    92,    -1,    94,    -1,    96,
      -1,    -1,    99,   100,    -1,    -1,    -1,   104,   105,   106,
     107,    -1,   109,   110,    -1,   112,    -1,   114,   115,   116,
     117,   118,   119,   120,    -1,   122,   123,   124,    -1,   126,
     127,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,   164,    -1,    -1,
     167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,   202,   203,   204,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,
      -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
     100,    -1,    -1,    -1,   104,   105,   106,   107,    -1,   109,
     110,    -1,   112,    -1,   114,   115,   116,   117,   118,   119,
     120,    -1,   122,   123,   124,    -1,   126,   127,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,   202,   203,   204,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,    82,
      83,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      -1,    94,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,    -1,   112,
      -1,   114,   115,   116,   117,   118,   119,   120,    -1,   122,
     123,   124,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,   164,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,   202,
     203,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    71,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    80,    -1,    82,    83,    -1,    -1,
      -1,    87,    88,    89,    90,    -1,    92,    -1,    94,    -1,
      96,    97,    -1,    99,   100,    -1,    -1,    -1,   104,   105,
     106,   107,    -1,   109,   110,    -1,   112,    -1,   114,   115,
     116,   117,   118,   119,   120,    -1,   122,   123,   124,    -1,
     126,   127,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    -1,
      -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,   202,   203,    -1,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,
      -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    71,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,   100,    -1,    -1,    -1,   104,   105,   106,   107,    -1,
     109,   110,    -1,   112,    -1,   114,   115,   116,   117,   118,
     119,   120,    -1,   122,   123,   124,    -1,   126,   127,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,
      -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,   202,   203,   204,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,
      -1,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    71,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    -1,
      82,    83,    -1,    -1,    -1,    87,    88,    89,    90,    -1,
      92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,    -1,
      -1,    -1,   104,   105,   106,   107,    -1,   109,   110,    -1,
     112,    -1,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,   123,   124,    -1,   126,   127,    -1,    -1,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,   164,    -1,    -1,   167,    -1,    -1,   170,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
     202,   203,   204,   205,   206,    -1,   208,   209,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    71,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    80,    -1,    82,    83,    -1,
      -1,    -1,    87,    88,    89,    90,    -1,    92,    -1,    94,
      95,    96,    -1,    -1,    99,   100,    -1,    -1,    -1,   104,
     105,   106,   107,    -1,   109,   110,    -1,   112,    -1,   114,
     115,   116,   117,   118,   119,   120,    -1,   122,   123,   124,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,   144,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,   164,
      -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,   202,   203,    -1,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    71,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    80,    -1,    82,    83,    -1,    -1,    -1,    87,
      88,    89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,
      -1,    99,   100,    -1,    -1,    -1,   104,   105,   106,   107,
      -1,   109,   110,    -1,   112,    -1,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,   123,   124,    -1,   126,   127,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,   164,    -1,    -1,   167,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,   202,   203,   204,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,    -1,
      -1,    -1,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
      -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,
      -1,    -1,    -1,   104,   105,   106,   107,    -1,   109,   110,
      -1,   112,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,   122,   123,   124,    -1,   126,   127,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,   170,
      -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,   202,   203,   204,   205,   206,    -1,   208,   209,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,    53,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    71,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    80,    -1,    82,    83,
      -1,    -1,    -1,    87,    88,    89,    90,    -1,    92,    93,
      94,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,    -1,
     104,   105,   106,   107,    -1,   109,   110,    -1,   112,    -1,
     114,   115,   116,   117,   118,   119,   120,    -1,   122,   123,
     124,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
     144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
     164,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,   202,   203,
      -1,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    71,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    80,    -1,    82,    83,    -1,    -1,    -1,
      87,    88,    89,    90,    -1,    92,    -1,    94,    -1,    96,
      -1,    -1,    99,   100,    -1,    -1,    -1,   104,   105,   106,
     107,    -1,   109,   110,    -1,   112,    -1,   114,   115,   116,
     117,   118,   119,   120,    -1,   122,   123,   124,    -1,   126,
     127,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,   164,    -1,    -1,
     167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,   202,   203,   204,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,
      -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
     100,    -1,    -1,    -1,   104,   105,   106,   107,    -1,   109,
     110,    -1,   112,    -1,   114,   115,   116,   117,   118,   119,
     120,    -1,   122,   123,   124,    -1,   126,   127,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,   202,   203,   204,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,    82,
      83,    -1,    -1,    -1,    87,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,    -1,   112,
      -1,   114,   115,   116,   117,   118,   119,   120,    -1,   122,
     123,   124,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,   164,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,   202,
     203,   204,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    71,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    80,    -1,    82,    83,    -1,    -1,
      -1,    87,    88,    89,    90,    -1,    92,    -1,    94,    -1,
      96,    -1,    -1,    99,   100,    -1,    -1,    -1,   104,   105,
     106,   107,    -1,   109,   110,    -1,   112,    -1,   114,   115,
     116,   117,   118,   119,   120,    -1,   122,   123,   124,    -1,
     126,   127,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    -1,
      -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,   202,   203,    -1,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    71,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,   100,    -1,    -1,    -1,   104,   105,   106,   107,    -1,
     109,   110,    -1,   112,    -1,   114,   115,   116,   117,   118,
     119,   120,    -1,   122,   123,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,
      -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,   202,   203,    -1,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    -1,
      -1,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    71,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    -1,
      82,    83,    -1,    -1,    -1,    87,    88,    89,    90,    -1,
      92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,    -1,
      -1,    -1,   104,   105,   106,   107,    -1,   109,   110,    -1,
     112,    -1,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,   167,    -1,    -1,   170,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
     202,   203,    -1,   205,   206,    -1,   208,   209,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    71,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    80,    -1,    82,    83,    -1,
      -1,    -1,    87,    88,    89,    90,    -1,    92,    -1,    94,
      -1,    96,    -1,    -1,    99,   100,    -1,    -1,    -1,   104,
     105,   106,   107,    -1,   109,   110,    -1,   112,    -1,   114,
     115,   116,   117,   118,   119,   120,    -1,   122,   123,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,   144,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,   202,   203,    -1,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    71,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    80,    -1,    82,    83,    -1,    -1,    -1,    87,
      88,    89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,
      -1,    99,   100,    -1,    -1,    -1,   104,   105,   106,   107,
      -1,   109,   110,    -1,   112,    -1,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,   123,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,   202,   203,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    -1,
      -1,    -1,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
      -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,
      -1,    -1,    -1,   104,   105,   106,   107,    -1,   109,   110,
      -1,   112,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,   122,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,   170,
      -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,   202,   203,    -1,   205,   206,    -1,   208,   209,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,    53,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   115,   116,   117,   118,   119,   120,    -1,    -1,   123,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,   202,    -1,
      -1,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    -1,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,   116,
     117,   118,   119,   120,    -1,    -1,   123,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,   165,    -1,
     167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,
      -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   115,   116,   117,   118,   119,
     120,    -1,    -1,   123,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    -1,   203,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   115,   116,   117,   118,   119,   120,    -1,    -1,
     123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,   165,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,
      -1,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
     116,   117,   118,   119,   120,    -1,    -1,   123,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,    10,    11,    12,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      -1,    -1,    66,    -1,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,    -1,   104,    -1,    -1,   107,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   115,   116,   117,   118,
     119,   120,    -1,    -1,   123,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,
      -1,   170,    -1,    -1,    -1,    -1,    -1,   176,   192,   193,
      -1,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    47,    48,    -1,    -1,    -1,
      -1,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,   116,   117,   118,   119,   120,    -1,
      -1,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,   167,    -1,    -1,   170,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
      10,    11,    12,   205,   206,    -1,   208,   209,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    -1,    -1,    66,    -1,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     115,   116,   117,   118,   119,   120,    -1,    -1,   123,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,   189,
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,   202,    11,    12,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    -1,    -1,    66,    -1,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,   116,   117,
     118,   119,   120,    -1,    -1,   123,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,   202,    -1,    12,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    -1,
      66,    -1,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   115,   116,   117,   118,   119,   120,
      -1,    -1,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,   170,
      -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
     201,    -1,    -1,    -1,   205,   206,    -1,   208,   209,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,    53,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   115,   116,   117,   118,   119,   120,    -1,    -1,   123,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,    -1,
      -1,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    -1,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,   116,
     117,   118,   119,   120,    -1,    -1,   123,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
     167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    47,    48,    -1,
      -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   115,   116,   117,   118,   119,
     120,    -1,    -1,   123,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   115,   116,   117,   118,   119,   120,    -1,    -1,
     123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,
      -1,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
     116,   117,   118,   119,   120,    -1,    -1,   123,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,    10,    11,    12,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      -1,    -1,    66,    -1,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   115,   116,   117,   118,
     119,   120,    -1,    -1,   123,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,
      -1,   170,    -1,    -1,   188,    -1,    -1,   176,    -1,    -1,
      -1,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,    10,    11,    12,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    66,
      -1,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,   116,   117,   118,   119,   120,    -1,
      -1,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,   167,    -1,    -1,   170,    -1,
     187,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   205,   206,    -1,   208,   209,     3,     4,
      -1,     6,     7,    -1,    12,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    27,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    68,    69,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,    -1,    -1,    -1,
      -1,   126,   127,   128,   129,    -1,    -1,    -1,   133,   134,
     135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   152,    10,    11,
      12,    13,    -1,   158,   159,    -1,   161,   162,   163,   164,
      -1,   166,    -1,    -1,   169,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    54,    -1,    -1,    -1,    -1,    -1,   203,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
      -1,    -1,    -1,    -1,   126,   127,   128,   129,    -1,    -1,
      -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
     152,    10,    11,    12,    13,    -1,   158,   159,    -1,   161,
     162,   163,   164,    -1,   166,    -1,    -1,   169,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    54,    -1,    -1,    -1,    -1,
      -1,   203,    -1,   205,    -1,    -1,    -1,    -1,    -1,    68,
      69,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,    -1,    -1,    -1,    -1,   126,   127,   128,
     129,    -1,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,   152,    10,    11,    12,    13,    -1,   158,
     159,    -1,   161,   162,   163,   164,    -1,   166,    -1,    -1,
     169,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    54,    -1,
      56,    -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    68,    69,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    -1,    -1,    -1,    -1,
      -1,   127,   128,   129,    78,    -1,    -1,   133,   134,   135,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,   152,    -1,    -1,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,     3,     4,   200,     6,     7,    -1,   204,    10,
      11,    12,    13,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    54,    -1,    56,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,    -1,    -1,   127,   128,   129,    78,
      -1,    -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,   152,    -1,    -1,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,   164,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,     3,     4,   200,
       6,     7,    -1,   204,    10,    11,    12,    13,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    54,    -1,
      56,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    69,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    -1,    -1,    -1,    -1,
     126,   127,   128,   129,    78,    -1,    -1,   133,   134,   135,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,   152,    -1,    -1,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,     3,     4,   200,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,    -1,    -1,   127,   128,   129,    -1,
      -1,    -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,   164,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,    -1,   176,   177,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,     3,     4,   200,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    29,    29,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    54,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    68,    69,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,    -1,    -1,    -1,    -1,
      -1,   127,   128,   129,    -1,    -1,    78,   133,   134,   135,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   152,    99,    -1,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    -1,   124,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,   200,     3,     4,     5,     6,     7,
      -1,    -1,    10,    11,    12,    13,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    54,    -1,   200,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    69,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   152,   153,   154,    -1,    -1,    -1,
     158,   159,    -1,   161,   162,   163,   164,    -1,   166,   167,
      -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,
      -1,   179,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    27,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,    -1,    -1,
      -1,    -1,   126,   127,   128,   129,    -1,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   152,    10,
      11,    12,    13,    -1,   158,   159,    -1,   161,   162,   163,
     164,    -1,   166,    -1,    -1,   169,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,    -1,   126,   127,   128,   129,    -1,
      -1,    -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,   152,    10,    11,    12,    13,    -1,   158,   159,    -1,
     161,   162,   163,   164,    -1,   166,    -1,    -1,   169,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    69,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,   127,
     128,   129,    -1,    -1,    -1,   133,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,    -1,
     158,   159,    -1,   161,   162,   163,   164,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,   202,    -1,    -1,    -1,    -1,    -1,    29,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,   202,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    -1,    78,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    99,    -1,
      -1,    56,    -1,    -1,    -1,    -1,   107,    -1,    66,    -1,
      -1,   201,    -1,    -1,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    29,
      -1,   152,   107,    -1,   155,   156,   201,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   176,    56,    -1,    -1,   134,
     135,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   196,   152,    78,   200,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,    99,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    56,    -1,   200,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,
      -1,    -1,   152,    -1,    -1,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,   165,    99,    -1,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    56,    -1,
     200,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,
      78,    -1,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,   165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    30,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    47,
      48,    -1,   200,    -1,    -1,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    99,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,    66,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    35,    -1,   167,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    67,
      -1,    -1,   200,   132,    -1,    -1,    -1,    75,    76,    77,
      78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      47,    48,   200,    -1,    -1,    -1,    53,   205,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    78,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    99,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    66,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      47,    48,    -1,    -1,   181,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      67,    -1,    -1,   200,   132,    -1,    -1,    -1,    75,    76,
      77,    78,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    99,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    -1,   134,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      67,    -1,    -1,    -1,   132,    -1,    -1,    -1,    75,    76,
      77,    78,   156,    80,   158,   159,   160,   161,   162,   163,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   120,    -1,    -1,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    67,    -1,   200,    -1,    -1,   203,    -1,   205,    75,
      76,    77,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    67,   200,    69,    -1,    -1,    -1,   205,
      -1,    75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,   120,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    67,    -1,   200,    -1,    -1,    -1,
      -1,   205,    75,    76,    77,    78,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    67,    -1,   200,    -1,    -1,
      -1,    -1,   205,    75,    76,    77,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,   120,    -1,
      -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,    -1,
      -1,    66,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    78,    -1,    -1,    -1,   147,    99,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    99,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,    -1,   200,    75,
      76,    77,    78,   205,    80,   158,   159,    -1,   161,   162,
     163,    87,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,
      -1,   158,   159,    99,   161,   162,   163,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   120,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     136,   137,   138,   139,   140,    -1,    -1,    78,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    99,    -1,
      -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,   200,    75,    76,    77,    78,   205,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,   155,    -1,    -1,   158,   159,    99,
     161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    10,    11,    12,
     200,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   132,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   132,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   132,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   132,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,   155,   156,   132,   158,
     159,    99,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,
     155,    -1,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,    -1,    -1,   155,    -1,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    28,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    98,
      54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   211,   212,     0,   213,     3,     4,     5,     6,     7,
      13,    27,    28,    46,    47,    48,    53,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    67,    68,
      69,    70,    71,    75,    76,    77,    78,    79,    80,    82,
      83,    87,    88,    89,    90,    92,    94,    96,    99,   100,
     104,   105,   106,   107,   108,   109,   110,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   122,   123,   124,   125,
     126,   127,   133,   134,   136,   137,   138,   139,   140,   144,
     147,   152,   153,   154,   155,   156,   158,   159,   161,   162,
     163,   164,   167,   170,   176,   177,   179,   181,   182,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   200,   202,   203,   205,   206,   208,   209,
     214,   217,   226,   227,   228,   229,   230,   231,   234,   250,
     251,   255,   258,   263,   269,   329,   330,   338,   342,   343,
     344,   345,   346,   347,   348,   349,   351,   354,   366,   367,
     368,   370,   371,   373,   392,   402,   403,   404,   409,   412,
     431,   439,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   466,   468,   470,   118,   119,   120,
     133,   152,   162,   217,   250,   329,   348,   443,   348,   200,
     348,   348,   348,   104,   348,   348,   429,   430,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
      80,    87,   120,   147,   200,   227,   367,   403,   404,   409,
     443,   446,   443,    35,   348,   457,   458,   348,   120,   200,
     227,   403,   404,   405,   409,   440,   441,   442,   450,   454,
     455,   200,   339,   406,   200,   339,   355,   340,   348,   236,
     339,   200,   200,   200,   339,   202,   348,   217,   202,   348,
       3,     4,     6,     7,    10,    11,    12,    13,    27,    29,
      54,    56,    68,    69,    70,    71,    72,    73,    74,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   126,   127,   128,   129,   133,   134,   135,   152,
     156,   164,   166,   169,   176,   200,   217,   218,   219,   230,
     471,   486,   487,   489,   183,   202,   345,   348,   372,   374,
     203,   243,   348,   107,   108,   155,   220,   223,   226,    80,
     205,   295,   296,   119,   126,   118,   126,    80,   297,   200,
     200,   200,   200,   217,   267,   474,   200,   200,   340,    80,
      86,   148,   149,   150,   463,   464,   155,   203,   226,   226,
     217,   268,   474,   156,   200,   474,   474,    80,   197,   203,
     357,   338,   348,   349,   443,   447,   232,   203,   452,    86,
     407,   463,    86,   463,   463,    30,   155,   172,   475,   200,
       9,   202,    35,   249,   156,   266,   474,   120,   182,   250,
     330,   202,   202,   202,   202,   202,   202,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    54,    66,   202,    67,    67,   202,   203,
     151,   127,   162,   164,   177,   179,   269,   328,   329,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    64,    65,   130,   131,   433,    67,   203,   438,
     200,   200,    67,   203,   200,   249,   250,    14,   348,   202,
     132,    45,   217,   428,    86,   338,   349,   151,   443,   132,
     207,     9,   414,   338,   349,   443,   475,   151,   200,   408,
     433,   438,   201,   348,    30,   234,     8,   360,     9,   202,
     234,   235,   340,   341,   348,   217,   281,   238,   202,   202,
     202,   134,   135,   489,   489,   172,   200,   107,   489,    14,
     151,   134,   135,   152,   217,   219,    80,   202,   202,   202,
     183,   184,   185,   190,   191,   194,   195,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   387,   388,   389,   244,
     111,   169,   202,   155,   221,   224,   226,   155,   222,   225,
     226,   226,     9,   202,    98,   203,   443,     9,   202,   126,
     126,    14,     9,   202,   443,   467,   340,   338,   349,   443,
     446,   447,   201,   172,   261,   133,   443,   456,   457,   202,
      67,   433,   148,   464,    79,   348,   443,    86,   148,   464,
     226,   216,   202,   203,   256,   264,   393,   395,    87,   200,
     361,   362,   364,   404,   451,   468,    14,    98,   469,   356,
     358,   359,   291,   292,   431,   432,   201,   201,   201,   201,
     204,   233,   234,   251,   258,   263,   431,   348,   206,   208,
     209,   217,   476,   477,   489,    35,   165,   293,   294,   348,
     471,   200,   474,   259,   249,   348,   348,   348,    30,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   405,   348,   348,   453,   453,   348,   459,   460,
     126,   203,   218,   219,   452,   267,   217,   268,   474,   474,
     266,   250,    27,    35,   342,   345,   348,   372,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     156,   203,   217,   434,   435,   436,   437,   452,   453,   348,
     293,   293,   453,   348,   456,   249,   201,   348,   200,   427,
       9,   414,   201,   201,    35,   348,    35,   348,   201,   201,
     201,   450,   451,   452,   293,   203,   217,   434,   435,   452,
     201,   232,   285,   203,   345,   348,   348,    90,    30,   234,
     279,   202,    28,    98,    14,     9,   201,    30,   203,   282,
     489,    29,    87,   230,   483,   484,   485,   200,     9,    47,
      48,    53,    55,    67,   134,   156,   176,   200,   227,   228,
     230,   369,   403,   409,   410,   411,   217,   488,   186,    80,
     348,    80,    80,   348,   384,   385,   348,   348,   377,   387,
     189,   390,   232,   200,   242,   226,   202,     9,    98,   226,
     202,     9,    98,    98,   223,   217,   348,   296,   410,    80,
       9,   201,   201,   201,   201,   201,   201,   201,   202,    47,
      48,   481,   482,   128,   272,   200,     9,   201,   201,    80,
      81,   217,   465,   217,    67,   204,   204,   213,   215,    30,
     129,   271,   171,    51,   156,   171,   397,   349,   132,     9,
     414,   201,   151,   489,   489,    14,   360,   291,   232,   198,
       9,   415,   489,   490,   433,   438,   433,   204,     9,   414,
     173,   443,   348,   201,     9,   415,    14,   352,   252,   128,
     270,   200,   474,   348,    30,   207,   207,   132,   204,     9,
     414,   348,   475,   200,   262,   257,   265,    14,   469,   260,
     249,    69,   443,   348,   475,   207,   204,   201,   201,   207,
     204,   201,    47,    48,    67,    75,    76,    77,    87,   134,
     147,   176,   217,   417,   419,   420,   423,   426,   217,   443,
     443,   132,   433,   438,   201,   348,   286,    72,    73,   287,
     232,   339,   232,   341,    98,    35,   133,   276,   443,   410,
     217,    30,   234,   280,   202,   283,   202,   283,     9,   173,
      87,   132,   151,     9,   414,   201,   165,   476,   477,   478,
     476,   410,   410,   410,   410,   410,   413,   416,   200,   151,
     200,   410,   151,   203,    10,    11,    12,    29,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      66,   151,   475,   348,   186,   186,    14,   192,   193,   386,
       9,   196,   390,    80,   204,   403,   203,   246,    98,   224,
     217,    98,   225,   217,   217,   204,    14,   443,   202,     9,
     173,   217,   273,   403,   203,   456,   133,   443,    14,   207,
     348,   204,   213,   489,   273,   203,   396,    14,   201,   348,
     361,   126,   218,   452,   202,   489,   198,   204,    30,   479,
     432,    35,    80,   165,   434,   435,   437,   434,   435,   489,
      35,   165,   348,   410,   291,   200,   403,   271,   353,   253,
     348,   348,   348,   204,   200,   293,   272,    30,   271,   489,
      14,   270,   474,   405,   204,   200,    14,    75,    76,    77,
     217,   418,   418,   420,   421,   422,    49,   200,    86,   148,
     200,     9,   414,   201,   427,    35,   348,   204,    72,    73,
     288,   339,   234,   204,   202,    91,   202,   276,   443,   200,
     132,   275,    14,   232,   283,   101,   102,   103,   283,   204,
     489,   132,   489,   217,   483,     9,   201,   414,   132,   207,
       9,   414,   413,   218,   361,   363,   365,   201,   126,   218,
     410,   461,   462,   410,   410,   410,    30,   410,   410,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   488,   348,   348,   348,   385,   348,   375,    80,   247,
     217,   217,   410,   482,    98,    99,   480,     9,   301,   201,
     200,   342,   345,   348,   207,   204,   469,   301,   157,   170,
     203,   392,   399,   157,   203,   398,   132,   202,   479,   489,
     360,   490,    80,   165,    14,    80,   475,   443,   348,   201,
     291,   203,   291,   200,   132,   200,   293,   201,   203,   489,
     203,   202,   489,   271,   254,   408,   293,   132,   207,     9,
     414,   419,   421,   148,   361,   424,   425,   420,   443,   339,
      30,    74,   234,   202,   341,   275,   456,   276,   201,   410,
      97,   101,   202,   348,    30,   202,   284,   204,   173,   489,
     132,   165,    30,   201,   410,   410,   201,   132,     9,   414,
     201,   132,   204,     9,   414,   410,    30,   187,   201,   232,
     217,   489,   489,   403,     4,   108,   113,   119,   121,   158,
     159,   161,   204,   302,   327,   328,   329,   334,   335,   336,
     337,   431,   456,   204,   203,   204,    51,   348,   348,   348,
     360,    35,    80,   165,    14,    80,   348,   200,   479,   201,
     301,   201,   291,   348,   293,   201,   301,   469,   301,   202,
     203,   200,   201,   420,   420,   201,   132,   201,     9,   414,
      30,   232,   202,   201,   201,   201,   239,   202,   202,   284,
     232,   489,   489,   132,   410,   361,   410,   410,   410,   348,
     203,   204,   480,   128,   129,   177,   218,   472,   489,   274,
     403,   108,   337,    29,   121,   134,   135,   156,   162,   311,
     312,   313,   314,   403,   160,   319,   320,   124,   200,   217,
     321,   322,   303,   250,   489,     9,   202,     9,   202,   202,
     469,   328,   201,   298,   156,   394,   204,   204,    80,   165,
      14,    80,   348,   293,   113,   350,   479,   204,   479,   201,
     201,   204,   203,   204,   301,   291,   132,   420,   361,   232,
     237,   240,    30,   234,   278,   232,   201,   410,   132,   132,
     188,   232,   403,   403,   474,    14,   218,     9,   202,   203,
     472,   469,   314,   172,   203,     9,   202,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    54,    68,
      69,    70,    71,    72,    73,    74,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   133,   134,   136,
     137,   138,   139,   140,   152,   153,   154,   164,   166,   167,
     169,   176,   177,   179,   181,   182,   217,   400,   401,     9,
     202,   156,   160,   217,   322,   323,   324,   202,    80,   333,
     249,   304,   472,   472,    14,   250,   204,   299,   300,   472,
      14,    80,   348,   201,   200,   203,   202,   203,   325,   350,
     479,   298,   204,   201,   420,   132,    30,   234,   277,   278,
     232,   410,   410,   348,   204,   202,   202,   410,   403,   307,
     489,   315,   316,   409,   312,    14,    30,    48,   317,   320,
       9,    33,   201,    29,    47,    50,    14,     9,   202,   219,
     473,   333,    14,   489,   249,   202,    14,   348,    35,    80,
     391,   232,   232,   203,   325,   204,   479,   420,   232,    95,
     189,   245,   204,   217,   230,   308,   309,   310,     9,   173,
       9,   414,   204,   410,   401,   401,    56,   318,   323,   323,
      29,    47,    50,   410,    80,   172,   200,   202,   410,   474,
     410,    80,     9,   415,   204,   204,   232,   325,    93,   202,
      80,   111,   241,   151,    98,   489,   409,   163,    14,   481,
     305,   200,    35,    80,   201,   204,   202,   200,   169,   248,
     217,   328,   329,   173,   410,   173,   289,   290,   432,   306,
      80,   403,   246,   166,   217,   202,   201,     9,   415,   115,
     116,   117,   331,   332,   289,    80,   274,   202,   479,   432,
     490,   201,   201,   202,   202,   203,   326,   331,    35,    80,
     165,   479,   203,   232,   490,    80,   165,    14,    80,   326,
     232,   204,    35,    80,   165,    14,    80,   348,   204,    80,
     165,    14,    80,   348,    14,    80,   348,   348
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
#line 740 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 743 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 894 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 904 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 918 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 993 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1054 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { (yyval).reset();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1083 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1091 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1092 "hphp.y"
    { (yyval).reset();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1097 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1101 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1107 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1113 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1120 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1151 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1155 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 209:

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

  case 210:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 211:

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

  case 212:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval).reset();;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { (yyval).reset();;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval).reset();;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { (yyval).reset();;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { (yyval).reset();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { (yyval).reset();;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { (yyval).reset();;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { (yyval).reset();;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { (yyval).reset();;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { (yyval).reset();;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1394 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1403 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1427 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1472 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { (yyval).reset();;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1491 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { (yyval).reset();;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { (yyval).reset();;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { (yyval).reset();;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval).reset();;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval).reset();;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval).reset();;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval).reset();;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval).reset();;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval).reset();;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval).reset();;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
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

  case 528:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
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

  case 532:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
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

  case 534:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
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

  case 597:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
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

  case 598:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval).reset();;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval).reset();;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval).reset();;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { (yyval).reset();;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { (yyval).reset();;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { (yyval).reset();;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { (yyval).reset();;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { (yyval).reset();;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { (yyval).reset();;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval).reset();;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { (yyval).reset();;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { (yyval).reset();;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2717 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { (yyval).reset();;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2726 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
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

  case 848:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
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

  case 849:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
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

  case 850:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
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

  case 851:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
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
#line 2855 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
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
#line 2884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2887 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { (yyval).reset();;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
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

  case 901:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
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

  case 907:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { (yyval).reset();;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3040 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3093 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
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

  case 943:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3107 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3124 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3162 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3174 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3189 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3195 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3211 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3243 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3249 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3252 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    {;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3257 "hphp.y"
    {;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3258 "hphp.y"
    {;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3264 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3269 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3280 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3285 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3286 "hphp.y"
    { ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3291 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3292 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3298 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3308 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3312 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3319 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3322 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3326 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3329 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3335 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3339 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3342 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3345 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3351 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3357 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3365 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3366 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14441 "hphp.7.tab.cpp"
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
#line 3369 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

