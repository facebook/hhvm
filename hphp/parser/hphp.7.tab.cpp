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
#define YYLAST   18849

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  210
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  283
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1026
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1859

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
    2284,  2288,  2290,  2294,  2296,  2298,  2302,  2304,  2306,  2308,
    2311,  2313,  2314,  2315,  2317,  2319,  2323,  2324,  2326,  2328,
    2330,  2332,  2334,  2336,  2338,  2340,  2342,  2344,  2346,  2348,
    2350,  2354,  2357,  2359,  2361,  2366,  2370,  2375,  2377,  2379,
    2383,  2387,  2391,  2395,  2399,  2403,  2407,  2411,  2415,  2419,
    2423,  2427,  2431,  2435,  2439,  2443,  2447,  2451,  2454,  2457,
    2460,  2463,  2467,  2471,  2475,  2479,  2483,  2487,  2491,  2495,
    2499,  2505,  2510,  2514,  2518,  2522,  2524,  2526,  2528,  2530,
    2534,  2538,  2542,  2545,  2546,  2548,  2549,  2551,  2552,  2558,
    2562,  2566,  2568,  2570,  2572,  2574,  2578,  2581,  2583,  2585,
    2587,  2589,  2591,  2595,  2597,  2599,  2601,  2604,  2607,  2612,
    2616,  2621,  2624,  2625,  2631,  2635,  2639,  2641,  2645,  2647,
    2650,  2651,  2657,  2661,  2664,  2665,  2669,  2670,  2675,  2678,
    2679,  2683,  2687,  2689,  2690,  2692,  2694,  2696,  2698,  2702,
    2704,  2706,  2708,  2712,  2714,  2716,  2720,  2724,  2727,  2732,
    2735,  2740,  2746,  2752,  2758,  2764,  2766,  2768,  2770,  2772,
    2774,  2776,  2780,  2784,  2789,  2794,  2798,  2800,  2802,  2804,
    2806,  2810,  2812,  2817,  2821,  2825,  2827,  2829,  2831,  2833,
    2835,  2839,  2843,  2848,  2853,  2857,  2859,  2861,  2869,  2879,
    2887,  2894,  2903,  2905,  2910,  2915,  2917,  2919,  2924,  2927,
    2929,  2930,  2932,  2934,  2936,  2940,  2944,  2948,  2949,  2951,
    2953,  2957,  2961,  2964,  2968,  2975,  2976,  2978,  2983,  2986,
    2987,  2993,  2997,  3001,  3003,  3010,  3015,  3020,  3023,  3026,
    3027,  3033,  3037,  3041,  3043,  3046,  3047,  3053,  3057,  3061,
    3063,  3066,  3069,  3071,  3074,  3076,  3081,  3085,  3089,  3096,
    3100,  3102,  3104,  3106,  3111,  3116,  3121,  3126,  3131,  3136,
    3139,  3142,  3147,  3150,  3153,  3155,  3159,  3163,  3167,  3168,
    3171,  3177,  3184,  3191,  3199,  3201,  3204,  3206,  3209,  3211,
    3216,  3218,  3223,  3227,  3228,  3230,  3234,  3237,  3241,  3243,
    3245,  3246,  3247,  3250,  3253,  3256,  3261,  3264,  3270,  3274,
    3276,  3278,  3279,  3283,  3288,  3294,  3298,  3300,  3303,  3304,
    3309,  3311,  3315,  3318,  3321,  3324,  3326,  3328,  3330,  3332,
    3336,  3341,  3348,  3350,  3359,  3366,  3368
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     211,     0,    -1,    -1,   212,   213,    -1,   213,   214,    -1,
      -1,   234,    -1,   251,    -1,   258,    -1,   255,    -1,   263,
      -1,   472,    -1,   125,   200,   201,   202,    -1,   152,   226,
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
     226,    -1,   155,   226,    -1,   227,    -1,   227,   477,    -1,
     227,   477,    -1,   231,     9,   473,    14,   412,    -1,   108,
     473,    14,   412,    -1,   232,   233,    -1,    -1,   234,    -1,
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
     469,   201,   202,    -1,   202,    -1,    82,    -1,    83,    -1,
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
      -1,    35,    -1,    -1,   107,    -1,    -1,   250,   249,   476,
     252,   200,   291,   201,   481,   325,    -1,    -1,   329,   250,
     249,   476,   253,   200,   291,   201,   481,   325,    -1,    -1,
     433,   328,   250,   249,   476,   254,   200,   291,   201,   481,
     325,    -1,    -1,   162,   217,   256,    30,   491,   471,   203,
     298,   204,    -1,    -1,   433,   162,   217,   257,    30,   491,
     471,   203,   298,   204,    -1,    -1,   269,   266,   259,   270,
     271,   203,   301,   204,    -1,    -1,   433,   269,   266,   260,
     270,   271,   203,   301,   204,    -1,    -1,   127,   267,   261,
     272,   203,   301,   204,    -1,    -1,   433,   127,   267,   262,
     272,   203,   301,   204,    -1,    -1,   164,   268,   264,   271,
     203,   301,   204,    -1,    -1,   433,   164,   268,   265,   271,
     203,   301,   204,    -1,   476,    -1,   156,    -1,   476,    -1,
     476,    -1,   126,    -1,   119,   126,    -1,   119,   118,   126,
      -1,   118,   119,   126,    -1,   118,   126,    -1,   128,   403,
      -1,    -1,   129,   273,    -1,    -1,   128,   273,    -1,    -1,
     403,    -1,   273,     9,   403,    -1,   403,    -1,   274,     9,
     403,    -1,   132,   276,    -1,    -1,   445,    -1,    35,   445,
      -1,   133,   200,   458,   201,    -1,   234,    -1,    30,   232,
      93,   202,    -1,   234,    -1,    30,   232,    95,   202,    -1,
     234,    -1,    30,   232,    91,   202,    -1,   234,    -1,    30,
     232,    97,   202,    -1,   217,    14,   412,    -1,   281,     9,
     217,    14,   412,    -1,   203,   283,   204,    -1,   203,   202,
     283,   204,    -1,    30,   283,   101,   202,    -1,    30,   202,
     283,   101,   202,    -1,   283,   102,   348,   284,   232,    -1,
     283,   103,   284,   232,    -1,    -1,    30,    -1,   202,    -1,
     285,    72,   339,   234,    -1,    -1,   286,    72,   339,    30,
     232,    -1,    -1,    73,   234,    -1,    -1,    73,    30,   232,
      -1,    -1,   290,     9,   434,   331,   492,   165,    80,    -1,
     290,     9,   434,   331,   492,    35,   165,    80,    -1,   290,
       9,   434,   331,   492,   165,    -1,   290,   417,    -1,   434,
     331,   492,   165,    80,    -1,   434,   331,   492,    35,   165,
      80,    -1,   434,   331,   492,   165,    -1,    -1,   434,   331,
     492,    80,    -1,   434,   331,   492,    35,    80,    -1,   434,
     331,   492,    35,    80,    14,   348,    -1,   434,   331,   492,
      80,    14,   348,    -1,   290,     9,   434,   331,   492,    80,
      -1,   290,     9,   434,   331,   492,    35,    80,    -1,   290,
       9,   434,   331,   492,    35,    80,    14,   348,    -1,   290,
       9,   434,   331,   492,    80,    14,   348,    -1,   292,     9,
     434,   492,   165,    80,    -1,   292,     9,   434,   492,    35,
     165,    80,    -1,   292,     9,   434,   492,   165,    -1,   292,
     417,    -1,   434,   492,   165,    80,    -1,   434,   492,    35,
     165,    80,    -1,   434,   492,   165,    -1,    -1,   434,   492,
      80,    -1,   434,   492,    35,    80,    -1,   434,   492,    35,
      80,    14,   348,    -1,   434,   492,    80,    14,   348,    -1,
     292,     9,   434,   492,    80,    -1,   292,     9,   434,   492,
      35,    80,    -1,   292,     9,   434,   492,    35,    80,    14,
     348,    -1,   292,     9,   434,   492,    80,    14,   348,    -1,
     294,   417,    -1,    -1,   348,    -1,    35,   445,    -1,   165,
     348,    -1,   294,     9,   348,    -1,   294,     9,   165,   348,
      -1,   294,     9,    35,   445,    -1,   295,     9,   296,    -1,
     296,    -1,    80,    -1,   205,   445,    -1,   205,   203,   348,
     204,    -1,   297,     9,    80,    -1,   297,     9,    80,    14,
     412,    -1,    80,    -1,    80,    14,   412,    -1,   298,   299,
      -1,    -1,   300,   202,    -1,   474,    14,   412,    -1,   301,
     302,    -1,    -1,    -1,   327,   303,   333,   202,    -1,    -1,
     329,   491,   304,   333,   202,    -1,   334,   202,    -1,   335,
     202,    -1,   336,   202,    -1,    -1,   328,   250,   249,   475,
     200,   305,   289,   201,   481,   326,    -1,    -1,   433,   328,
     250,   249,   476,   200,   306,   289,   201,   481,   326,    -1,
     158,   311,   202,    -1,   159,   319,   202,    -1,   161,   321,
     202,    -1,     4,   128,   403,   202,    -1,     4,   129,   403,
     202,    -1,   113,   274,   202,    -1,   113,   274,   203,   307,
     204,    -1,   307,   308,    -1,   307,   309,    -1,    -1,   230,
     151,   217,   166,   274,   202,    -1,   310,    98,   328,   217,
     202,    -1,   310,    98,   329,   202,    -1,   230,   151,   217,
      -1,   217,    -1,   312,    -1,   311,     9,   312,    -1,   313,
     400,   317,   318,    -1,   156,    -1,    29,   314,    -1,   314,
      -1,   134,    -1,   134,   172,   491,   173,    -1,   134,   172,
     491,     9,   491,   173,    -1,   403,    -1,   121,    -1,   162,
     203,   316,   204,    -1,   135,    -1,   411,    -1,   315,     9,
     411,    -1,   315,   416,    -1,    14,   412,    -1,    -1,    56,
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
     333,     9,    80,    -1,   333,     9,    80,    14,   412,    -1,
      80,    -1,    80,    14,   412,    -1,   334,     9,   474,    14,
     412,    -1,   108,   474,    14,   412,    -1,   335,     9,   474,
      -1,   119,   108,   474,    -1,   119,   337,   471,    -1,   337,
     471,    14,   491,    -1,   108,   177,   476,    -1,   200,   338,
     201,    -1,    69,   407,   410,    -1,    68,   348,    -1,   392,
      -1,   368,    -1,   200,   348,   201,    -1,   340,     9,   348,
      -1,   348,    -1,   340,    -1,    -1,    27,    -1,    27,   348,
      -1,    27,   348,   132,   348,    -1,   445,    14,   342,    -1,
     133,   200,   458,   201,    14,   342,    -1,    28,   348,    -1,
     445,    14,   345,    -1,   133,   200,   458,   201,    14,   345,
      -1,   349,    -1,   445,    -1,   338,    -1,   449,    -1,   448,
      -1,   133,   200,   458,   201,    14,   348,    -1,   445,    14,
     348,    -1,   445,    14,    35,   445,    -1,   445,    14,    35,
      69,   407,   410,    -1,   445,    26,   348,    -1,   445,    25,
     348,    -1,   445,    24,   348,    -1,   445,    23,   348,    -1,
     445,    22,   348,    -1,   445,    21,   348,    -1,   445,    20,
     348,    -1,   445,    19,   348,    -1,   445,    18,   348,    -1,
     445,    17,   348,    -1,   445,    16,   348,    -1,   445,    15,
     348,    -1,   445,    65,    -1,    65,   445,    -1,   445,    64,
      -1,    64,   445,    -1,   348,    31,   348,    -1,   348,    32,
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
     348,    54,   407,    -1,   200,   349,   201,    -1,   348,    29,
     348,    30,   348,    -1,   348,    29,    30,   348,    -1,   468,
      -1,    63,   348,    -1,    62,   348,    -1,    61,   348,    -1,
      60,   348,    -1,    59,   348,    -1,    58,   348,    -1,    57,
     348,    -1,    70,   408,    -1,    56,   348,    -1,   414,    -1,
     367,    -1,   366,    -1,   206,   409,   206,    -1,    13,   348,
      -1,   370,    -1,   113,   200,   391,   417,   201,    -1,    -1,
      -1,   250,   249,   200,   352,   291,   201,   481,   350,   203,
     232,   204,    -1,    -1,   329,   250,   249,   200,   353,   291,
     201,   481,   350,   203,   232,   204,    -1,    -1,    80,   355,
     360,    -1,    -1,   182,    80,   356,   360,    -1,    -1,   197,
     357,   291,   198,   481,   360,    -1,    -1,   182,   197,   358,
     291,   198,   481,   360,    -1,    -1,   182,   203,   359,   232,
     204,    -1,     8,   348,    -1,     8,   345,    -1,     8,   203,
     232,   204,    -1,    87,    -1,   470,    -1,   362,     9,   361,
     132,   348,    -1,   361,   132,   348,    -1,   363,     9,   361,
     132,   412,    -1,   361,   132,   412,    -1,   362,   416,    -1,
      -1,   363,   416,    -1,    -1,   176,   200,   364,   201,    -1,
     134,   200,   459,   201,    -1,    67,   459,   207,    -1,   403,
     203,   461,   204,    -1,   403,   203,   463,   204,    -1,   370,
      67,   455,   207,    -1,   371,    67,   455,   207,    -1,   367,
      -1,   470,    -1,   448,    -1,    87,    -1,   200,   349,   201,
      -1,   374,   375,    -1,   445,    14,   372,    -1,   183,    80,
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
     201,    -1,   230,    -1,   156,    -1,   403,    -1,   411,    -1,
     120,    -1,   453,    -1,   200,   349,   201,    -1,   404,    -1,
     405,   151,   454,    -1,   404,    -1,   451,    -1,   406,   151,
     454,    -1,   403,    -1,   120,    -1,   456,    -1,   200,   201,
      -1,   339,    -1,    -1,    -1,    86,    -1,   465,    -1,   200,
     293,   201,    -1,    -1,    75,    -1,    76,    -1,    77,    -1,
      87,    -1,   139,    -1,   140,    -1,   154,    -1,   136,    -1,
     167,    -1,   137,    -1,   138,    -1,   153,    -1,   181,    -1,
     147,    86,   148,    -1,   147,   148,    -1,   411,    -1,   228,
      -1,   134,   200,   415,   201,    -1,    67,   415,   207,    -1,
     176,   200,   365,   201,    -1,   413,    -1,   369,    -1,   200,
     412,   201,    -1,   412,    31,   412,    -1,   412,    32,   412,
      -1,   412,    10,   412,    -1,   412,    12,   412,    -1,   412,
      11,   412,    -1,   412,    33,   412,    -1,   412,    35,   412,
      -1,   412,    34,   412,    -1,   412,    49,   412,    -1,   412,
      47,   412,    -1,   412,    48,   412,    -1,   412,    50,   412,
      -1,   412,    51,   412,    -1,   412,    52,   412,    -1,   412,
      46,   412,    -1,   412,    45,   412,    -1,   412,    66,   412,
      -1,    53,   412,    -1,    55,   412,    -1,    47,   412,    -1,
      48,   412,    -1,   412,    37,   412,    -1,   412,    36,   412,
      -1,   412,    39,   412,    -1,   412,    38,   412,    -1,   412,
      40,   412,    -1,   412,    44,   412,    -1,   412,    41,   412,
      -1,   412,    43,   412,    -1,   412,    42,   412,    -1,   412,
      29,   412,    30,   412,    -1,   412,    29,    30,   412,    -1,
     230,   151,   218,    -1,   156,   151,   218,    -1,   230,   151,
     126,    -1,   228,    -1,    79,    -1,   470,    -1,   411,    -1,
     208,   465,   208,    -1,   209,   465,   209,    -1,   147,   465,
     148,    -1,   418,   416,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   418,     9,   412,   132,   412,    -1,   418,     9,
     412,    -1,   412,   132,   412,    -1,   412,    -1,    75,    -1,
      76,    -1,    77,    -1,   147,    86,   148,    -1,   147,   148,
      -1,    75,    -1,    76,    -1,    77,    -1,   217,    -1,    87,
      -1,    87,    49,   421,    -1,   419,    -1,   421,    -1,   217,
      -1,    47,   420,    -1,    48,   420,    -1,   134,   200,   423,
     201,    -1,    67,   423,   207,    -1,   176,   200,   426,   201,
      -1,   424,   416,    -1,    -1,   424,     9,   422,   132,   422,
      -1,   424,     9,   422,    -1,   422,   132,   422,    -1,   422,
      -1,   425,     9,   422,    -1,   422,    -1,   427,   416,    -1,
      -1,   427,     9,   361,   132,   422,    -1,   361,   132,   422,
      -1,   425,   416,    -1,    -1,   200,   428,   201,    -1,    -1,
     430,     9,   217,   429,    -1,   217,   429,    -1,    -1,   432,
     430,   416,    -1,    46,   431,    45,    -1,   433,    -1,    -1,
     130,    -1,   131,    -1,   217,    -1,   156,    -1,   203,   348,
     204,    -1,   436,    -1,   454,    -1,   217,    -1,   203,   348,
     204,    -1,   438,    -1,   454,    -1,    67,   455,   207,    -1,
     203,   348,   204,    -1,   446,   440,    -1,   200,   338,   201,
     440,    -1,   457,   440,    -1,   200,   338,   201,   440,    -1,
     200,   338,   201,   435,   437,    -1,   200,   349,   201,   435,
     437,    -1,   200,   338,   201,   435,   436,    -1,   200,   349,
     201,   435,   436,    -1,   452,    -1,   402,    -1,   450,    -1,
     451,    -1,   441,    -1,   443,    -1,   445,   435,   437,    -1,
     406,   151,   454,    -1,   447,   200,   293,   201,    -1,   448,
     200,   293,   201,    -1,   200,   445,   201,    -1,   402,    -1,
     450,    -1,   451,    -1,   441,    -1,   445,   435,   437,    -1,
     444,    -1,   447,   200,   293,   201,    -1,   200,   445,   201,
      -1,   406,   151,   454,    -1,   452,    -1,   441,    -1,   402,
      -1,   367,    -1,   411,    -1,   200,   445,   201,    -1,   200,
     349,   201,    -1,   448,   200,   293,   201,    -1,   447,   200,
     293,   201,    -1,   200,   449,   201,    -1,   351,    -1,   354,
      -1,   445,   435,   439,   477,   200,   293,   201,    -1,   200,
     338,   201,   435,   439,   477,   200,   293,   201,    -1,   406,
     151,   219,   477,   200,   293,   201,    -1,   406,   151,   454,
     200,   293,   201,    -1,   406,   151,   203,   348,   204,   200,
     293,   201,    -1,   453,    -1,   453,    67,   455,   207,    -1,
     453,   203,   348,   204,    -1,   454,    -1,    80,    -1,   205,
     203,   348,   204,    -1,   205,   454,    -1,   348,    -1,    -1,
     452,    -1,   442,    -1,   443,    -1,   456,   435,   437,    -1,
     405,   151,   452,    -1,   200,   445,   201,    -1,    -1,   442,
      -1,   444,    -1,   456,   435,   436,    -1,   200,   445,   201,
      -1,   458,     9,    -1,   458,     9,   445,    -1,   458,     9,
     133,   200,   458,   201,    -1,    -1,   445,    -1,   133,   200,
     458,   201,    -1,   460,   416,    -1,    -1,   460,     9,   348,
     132,   348,    -1,   460,     9,   348,    -1,   348,   132,   348,
      -1,   348,    -1,   460,     9,   348,   132,    35,   445,    -1,
     460,     9,    35,   445,    -1,   348,   132,    35,   445,    -1,
      35,   445,    -1,   462,   416,    -1,    -1,   462,     9,   348,
     132,   348,    -1,   462,     9,   348,    -1,   348,   132,   348,
      -1,   348,    -1,   464,   416,    -1,    -1,   464,     9,   412,
     132,   412,    -1,   464,     9,   412,    -1,   412,   132,   412,
      -1,   412,    -1,   465,   466,    -1,   465,    86,    -1,   466,
      -1,    86,   466,    -1,    80,    -1,    80,    67,   467,   207,
      -1,    80,   435,   217,    -1,   149,   348,   204,    -1,   149,
      79,    67,   348,   207,   204,    -1,   150,   445,   204,    -1,
     217,    -1,    81,    -1,    80,    -1,   123,   200,   340,   201,
      -1,   124,   200,   445,   201,    -1,   124,   200,   349,   201,
      -1,   124,   200,   449,   201,    -1,   124,   200,   448,   201,
      -1,   124,   200,   338,   201,    -1,     7,   348,    -1,     6,
     348,    -1,     5,   200,   348,   201,    -1,     4,   348,    -1,
       3,   348,    -1,   445,    -1,   469,     9,   445,    -1,   406,
     151,   218,    -1,   406,   151,   126,    -1,    -1,    98,   491,
      -1,   177,   476,    14,   491,   202,    -1,   433,   177,   476,
      14,   491,   202,    -1,   179,   476,   471,    14,   491,   202,
      -1,   433,   179,   476,   471,    14,   491,   202,    -1,   219,
      -1,   491,   219,    -1,   218,    -1,   491,   218,    -1,   219,
      -1,   219,   172,   483,   173,    -1,   217,    -1,   217,   172,
     483,   173,    -1,   172,   479,   173,    -1,    -1,   491,    -1,
     478,     9,   491,    -1,   478,   416,    -1,   478,     9,   165,
      -1,   479,    -1,   165,    -1,    -1,    -1,    30,   491,    -1,
      98,   491,    -1,    99,   491,    -1,   483,     9,   484,   217,
      -1,   484,   217,    -1,   483,     9,   484,   217,   482,    -1,
     484,   217,   482,    -1,    47,    -1,    48,    -1,    -1,    87,
     132,   491,    -1,    29,    87,   132,   491,    -1,   230,   151,
     217,   132,   491,    -1,   486,     9,   485,    -1,   485,    -1,
     486,   416,    -1,    -1,   176,   200,   487,   201,    -1,   230,
      -1,   217,   151,   490,    -1,   217,   477,    -1,    29,   491,
      -1,    56,   491,    -1,   230,    -1,   134,    -1,   135,    -1,
     488,    -1,   489,   151,   490,    -1,   134,   172,   491,   173,
      -1,   134,   172,   491,     9,   491,   173,    -1,   156,    -1,
     200,   107,   200,   480,   201,    30,   491,   201,    -1,   200,
     491,     9,   478,   416,   201,    -1,   491,    -1,    -1
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
    2430,  2431,  2432,  2436,  2441,  2442,  2446,  2447,  2448,  2449,
    2451,  2455,  2456,  2467,  2468,  2470,  2482,  2483,  2484,  2488,
    2489,  2490,  2494,  2495,  2496,  2499,  2501,  2505,  2506,  2507,
    2508,  2510,  2511,  2512,  2513,  2514,  2515,  2516,  2517,  2518,
    2519,  2522,  2527,  2528,  2529,  2531,  2532,  2534,  2535,  2536,
    2537,  2539,  2541,  2543,  2545,  2547,  2548,  2549,  2550,  2551,
    2552,  2553,  2554,  2555,  2556,  2557,  2558,  2559,  2560,  2561,
    2562,  2563,  2565,  2567,  2569,  2571,  2572,  2575,  2576,  2580,
    2584,  2586,  2590,  2593,  2596,  2602,  2603,  2604,  2605,  2606,
    2607,  2608,  2613,  2615,  2619,  2620,  2623,  2624,  2628,  2631,
    2633,  2635,  2639,  2640,  2641,  2642,  2645,  2649,  2650,  2651,
    2652,  2656,  2658,  2665,  2666,  2667,  2668,  2669,  2670,  2672,
    2673,  2678,  2680,  2683,  2686,  2688,  2690,  2693,  2695,  2699,
    2701,  2704,  2707,  2713,  2715,  2718,  2719,  2724,  2727,  2731,
    2731,  2736,  2739,  2740,  2744,  2745,  2749,  2750,  2751,  2755,
    2760,  2765,  2766,  2770,  2775,  2780,  2781,  2785,  2786,  2791,
    2793,  2798,  2809,  2823,  2835,  2850,  2851,  2852,  2853,  2854,
    2855,  2856,  2866,  2875,  2877,  2879,  2883,  2884,  2885,  2886,
    2887,  2903,  2904,  2906,  2908,  2915,  2916,  2917,  2918,  2919,
    2920,  2921,  2922,  2924,  2929,  2933,  2934,  2938,  2941,  2948,
    2952,  2961,  2968,  2976,  2978,  2979,  2983,  2984,  2986,  2991,
    2992,  3003,  3004,  3005,  3006,  3017,  3020,  3023,  3024,  3025,
    3026,  3037,  3041,  3042,  3043,  3045,  3046,  3047,  3051,  3053,
    3056,  3058,  3059,  3060,  3061,  3064,  3066,  3067,  3071,  3073,
    3076,  3078,  3079,  3080,  3084,  3086,  3089,  3092,  3094,  3096,
    3100,  3101,  3103,  3104,  3110,  3111,  3113,  3123,  3125,  3127,
    3130,  3131,  3132,  3136,  3137,  3138,  3139,  3140,  3141,  3142,
    3143,  3144,  3145,  3146,  3150,  3151,  3155,  3157,  3165,  3167,
    3171,  3175,  3180,  3184,  3192,  3193,  3197,  3198,  3204,  3205,
    3214,  3215,  3223,  3226,  3230,  3233,  3238,  3243,  3245,  3246,
    3247,  3251,  3252,  3256,  3257,  3260,  3263,  3265,  3269,  3275,
    3276,  3277,  3281,  3285,  3295,  3303,  3305,  3309,  3311,  3316,
    3322,  3325,  3330,  3338,  3341,  3344,  3345,  3348,  3351,  3352,
    3357,  3360,  3364,  3368,  3374,  3384,  3385
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
     404,   405,   405,   406,   406,   406,   407,   407,   407,   408,
     408,   408,   409,   409,   409,   410,   410,   411,   411,   411,
     411,   411,   411,   411,   411,   411,   411,   411,   411,   411,
     411,   411,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   413,   413,   413,   414,   414,   414,   414,   414,
     414,   414,   415,   415,   416,   416,   417,   417,   418,   418,
     418,   418,   419,   419,   419,   419,   419,   420,   420,   420,
     420,   421,   421,   422,   422,   422,   422,   422,   422,   422,
     422,   423,   423,   424,   424,   424,   424,   425,   425,   426,
     426,   427,   427,   428,   428,   429,   429,   430,   430,   432,
     431,   433,   434,   434,   435,   435,   436,   436,   436,   437,
     437,   438,   438,   439,   439,   440,   440,   441,   441,   442,
     442,   443,   443,   444,   444,   445,   445,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   446,   446,   446,   446,
     446,   446,   446,   446,   446,   447,   447,   447,   447,   447,
     447,   447,   447,   447,   448,   449,   449,   450,   450,   451,
     451,   451,   452,   453,   453,   453,   454,   454,   454,   455,
     455,   456,   456,   456,   456,   456,   456,   457,   457,   457,
     457,   457,   458,   458,   458,   458,   458,   458,   459,   459,
     460,   460,   460,   460,   460,   460,   460,   460,   461,   461,
     462,   462,   462,   462,   463,   463,   464,   464,   464,   464,
     465,   465,   465,   465,   466,   466,   466,   466,   466,   466,
     467,   467,   467,   468,   468,   468,   468,   468,   468,   468,
     468,   468,   468,   468,   469,   469,   470,   470,   471,   471,
     472,   472,   472,   472,   473,   473,   474,   474,   475,   475,
     476,   476,   477,   477,   478,   478,   479,   480,   480,   480,
     480,   481,   481,   482,   482,   483,   483,   483,   483,   484,
     484,   484,   485,   485,   485,   486,   486,   487,   487,   488,
     489,   490,   490,   491,   491,   491,   491,   491,   491,   491,
     491,   491,   491,   491,   491,   492,   492
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
       3,     1,     3,     1,     1,     3,     1,     1,     1,     2,
       1,     0,     0,     1,     1,     3,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     2,     1,     1,     4,     3,     4,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       5,     4,     3,     3,     3,     1,     1,     1,     1,     3,
       3,     3,     2,     0,     1,     0,     1,     0,     5,     3,
       3,     1,     1,     1,     1,     3,     2,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     2,     2,     4,     3,
       4,     2,     0,     5,     3,     3,     1,     3,     1,     2,
       0,     5,     3,     2,     0,     3,     0,     4,     2,     0,
       3,     3,     1,     0,     1,     1,     1,     1,     3,     1,
       1,     1,     3,     1,     1,     3,     3,     2,     4,     2,
       4,     5,     5,     5,     5,     1,     1,     1,     1,     1,
       1,     3,     3,     4,     4,     3,     1,     1,     1,     1,
       3,     1,     4,     3,     3,     1,     1,     1,     1,     1,
       3,     3,     4,     4,     3,     1,     1,     7,     9,     7,
       6,     8,     1,     4,     4,     1,     1,     4,     2,     1,
       0,     1,     1,     1,     3,     3,     3,     0,     1,     1,
       3,     3,     2,     3,     6,     0,     1,     4,     2,     0,
       5,     3,     3,     1,     6,     4,     4,     2,     2,     0,
       5,     3,     3,     1,     2,     0,     5,     3,     3,     1,
       2,     2,     1,     2,     1,     4,     3,     3,     6,     3,
       1,     1,     1,     4,     4,     4,     4,     4,     4,     2,
       2,     4,     2,     2,     1,     3,     3,     3,     0,     2,
       5,     6,     6,     7,     1,     2,     1,     2,     1,     4,
       1,     4,     3,     0,     1,     3,     2,     3,     1,     1,
       0,     0,     2,     2,     2,     4,     2,     5,     3,     1,
       1,     0,     3,     4,     5,     3,     1,     2,     0,     4,
       1,     3,     2,     2,     2,     1,     1,     1,     1,     3,
       4,     6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   435,     0,   829,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   919,     0,
     907,   711,     0,   717,   718,   719,    22,   776,   896,   163,
     164,   720,     0,   144,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   197,     0,     0,     0,     0,     0,     0,
     402,   403,   404,   407,   406,   405,     0,     0,     0,     0,
     224,     0,     0,     0,   724,   726,   727,   721,   722,     0,
       0,     0,   728,   723,     0,   695,    24,    25,    26,    28,
      27,     0,   725,     0,     0,     0,     0,   729,   408,    29,
      30,    32,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,   528,     0,   162,   134,     0,   712,     0,     0,
       4,   123,   125,   128,   775,     0,   694,     0,     6,   196,
       7,     9,     8,    10,     0,     0,   400,   445,     0,     0,
       0,     0,     0,     0,     0,   443,   885,   886,   514,   513,
     429,   517,     0,     0,   428,   856,   696,   703,     0,   778,
     512,   399,   859,   860,   871,   444,     0,     0,   447,   446,
     857,   858,   855,   892,   895,   502,   777,    11,   407,   406,
     405,     0,     0,    28,   123,   196,     0,   963,   444,   962,
       0,   960,   959,   516,     0,   436,   440,     0,     0,   485,
     486,   487,   488,   511,   509,   508,   507,   506,   505,   504,
     503,   896,   720,   698,     0,     0,   983,   878,   696,     0,
     697,   467,     0,   465,     0,   923,     0,   785,   427,   707,
       0,   983,   706,   701,     0,   716,   697,   902,   903,   909,
     901,   708,     0,     0,   710,   510,     0,     0,     0,     0,
     432,     0,   142,   434,     0,     0,   148,   150,     0,     0,
     152,     0,    82,    81,    76,    75,    67,    68,    59,    79,
      90,     0,    62,     0,    74,    66,    72,    92,    85,    84,
      57,    80,    99,   100,    58,    95,    55,    96,    56,    97,
      54,   101,    89,    93,    98,    86,    87,    61,    88,    91,
      53,    83,    69,   102,    77,    70,    60,    52,    51,    50,
      49,    48,    47,    71,   104,    64,    45,    46,    73,  1016,
    1017,    65,  1022,    44,    63,    94,     0,     0,   123,   103,
     974,  1015,     0,  1018,     0,     0,     0,   154,     0,     0,
       0,     0,   187,     0,     0,     0,     0,     0,     0,   106,
     111,   313,     0,     0,   312,     0,   228,     0,   225,   318,
       0,     0,     0,     0,     0,   980,   212,   222,   915,   919,
       0,   944,     0,   731,     0,     0,     0,   942,     0,    16,
       0,   127,   204,   216,   223,   601,   544,     0,   968,   526,
     530,   532,   833,   445,     0,   443,   444,   446,     0,     0,
     898,   713,     0,   714,     0,     0,     0,   186,     0,     0,
     130,   304,     0,    21,   195,     0,   221,   208,   220,   405,
     408,   196,   401,   177,   178,   179,   180,   181,   183,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   907,     0,   176,   900,   900,
     184,   929,     0,     0,     0,     0,     0,     0,     0,     0,
     398,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   466,   464,   834,   835,     0,   900,
       0,   847,   304,   304,   900,     0,   915,     0,   196,     0,
       0,   156,     0,   831,   826,   785,     0,   445,   443,     0,
     927,     0,   549,   784,   918,   445,   443,   444,   130,     0,
     304,   426,     0,   849,   709,     0,   134,   264,     0,   525,
       0,   159,     0,     0,   433,     0,     0,     0,     0,     0,
     151,   175,   153,  1016,  1017,  1013,  1014,     0,  1008,     0,
       0,     0,     0,    78,    43,    65,    42,   975,     0,   182,
     155,   185,     0,     0,     0,     0,     0,     0,     0,   559,
       0,   566,   568,   569,   570,   571,   572,   573,   564,   586,
     587,   134,     0,   172,   174,     0,     0,   108,   115,     0,
       0,   110,   119,   112,     0,    18,     0,     0,   314,     0,
     157,   227,   226,     0,     0,   158,   964,     0,     0,   445,
     443,   444,   447,   446,     0,  1001,   234,     0,   916,     0,
       0,   160,     0,     0,   730,   943,   776,     0,     0,   941,
     781,   940,   126,     5,    13,    14,     0,   232,     0,     0,
     537,     0,     0,   785,     0,     0,   704,   699,   538,     0,
       0,     0,     0,   833,   134,     0,   787,   832,  1026,   425,
     499,   865,   884,   139,   133,   135,   136,   137,   138,   399,
       0,   515,   779,   780,   124,   785,     0,   984,     0,     0,
       0,   787,   305,     0,   520,   198,   230,     0,   470,   472,
     471,     0,     0,   468,   469,   473,   475,   474,   490,   489,
     492,   491,   493,   495,   497,   496,   494,   484,   483,   477,
     478,   476,   479,   480,   482,   498,   481,   899,     0,     0,
     933,     0,   785,   967,     0,   966,   983,   862,   214,   206,
     218,     0,   968,   210,   196,   435,     0,   438,   441,   449,
     560,   463,   462,   461,   460,   459,   458,   457,   456,   455,
     454,   453,   452,   837,     0,   836,   839,   861,   843,   983,
     840,     0,     0,     0,     0,     0,     0,     0,     0,   961,
     437,   824,   828,   784,   830,     0,   700,     0,   922,     0,
     921,     0,   700,   906,   905,   892,   895,     0,     0,   836,
     839,   904,   840,   430,   266,   268,   134,   535,   534,   431,
       0,   134,   248,   143,   434,     0,     0,     0,     0,     0,
     260,   260,   149,     0,     0,     0,     0,  1006,   785,     0,
     990,     0,     0,     0,     0,     0,   783,     0,   695,     0,
       0,   128,   733,   694,   738,     0,   732,   132,   737,   983,
    1019,     0,     0,   576,     0,     0,   582,   579,   580,   588,
       0,   567,   562,     0,   565,     0,     0,     0,   116,    19,
       0,     0,   120,    20,     0,     0,     0,   105,   113,     0,
     311,   319,   316,     0,     0,   953,   958,   955,   954,   957,
     956,    12,   999,  1000,     0,     0,     0,     0,   915,   912,
       0,   548,   952,   951,   950,     0,   946,     0,   947,   949,
       0,     5,     0,     0,     0,   595,   596,   604,   603,     0,
     443,     0,   784,   543,   547,     0,     0,   969,     0,   527,
       0,     0,   991,   833,   290,  1025,     0,     0,   848,     0,
     897,   784,   986,   982,   306,   307,   693,   786,   303,     0,
     833,     0,     0,   232,   522,   200,   501,     0,   552,   553,
       0,   550,   784,   928,     0,     0,   304,   234,     0,   232,
       0,     0,   230,     0,   907,   450,     0,     0,   845,   846,
     863,   864,   893,   894,     0,     0,     0,   812,   792,   793,
     794,   801,     0,     0,     0,   805,   803,   804,   818,   785,
       0,   826,   926,   925,     0,     0,   850,   715,     0,   270,
       0,     0,   140,     0,     0,     0,     0,     0,     0,     0,
     240,   241,   252,     0,   134,   250,   169,   260,     0,   260,
       0,     0,  1020,     0,     0,     0,   784,  1007,  1009,   989,
     785,   988,     0,   785,   759,   760,   757,   758,   791,     0,
     785,   783,     0,   546,     0,     0,   935,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1012,   561,     0,     0,     0,
     584,   585,   583,     0,     0,   563,     0,   188,     0,   191,
     173,     0,   107,   117,     0,   109,   121,   114,   315,     0,
     965,   161,  1001,   981,   996,   233,   235,   325,     0,     0,
     913,     0,   945,     0,    17,     0,   968,   231,   325,     0,
       0,   700,   540,     0,   705,   970,     0,   991,   533,     0,
       0,  1026,     0,   295,   293,   839,   851,   983,   839,   852,
     985,     0,     0,   308,   131,     0,   833,   229,     0,   833,
       0,   500,   932,   931,     0,   304,     0,     0,     0,     0,
       0,     0,   232,   202,   716,   838,   304,     0,   797,   798,
     799,   800,   806,   807,   816,     0,   785,     0,   812,     0,
     796,   820,   784,   823,   825,   827,     0,   920,   838,     0,
       0,     0,     0,   267,   536,   145,     0,   434,   240,   242,
     915,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     254,     0,     0,  1002,     0,  1005,   784,     0,     0,     0,
     735,   784,   782,     0,   773,     0,   785,     0,   739,   774,
     772,   939,     0,   785,   742,   744,   743,     0,     0,   740,
     741,   745,   747,   746,   762,   761,   764,   763,   765,   767,
     769,   768,   766,   755,   754,   749,   750,   748,   751,   752,
     753,   756,  1011,   574,     0,   575,   581,   589,   590,     0,
     134,   118,   122,   317,     0,     0,     0,   998,     0,   399,
     917,   915,   439,   442,   448,     0,    15,     0,   399,   607,
       0,     0,   609,   602,   605,     0,   600,     0,   972,     0,
     992,   529,     0,   296,     0,     0,   291,     0,   310,   309,
     991,     0,   325,     0,   833,     0,   304,     0,   890,   325,
     968,   325,   971,     0,     0,     0,   451,     0,     0,   809,
     784,   811,   802,     0,   795,     0,     0,   785,   817,   924,
       0,   134,     0,   263,   249,     0,     0,     0,   239,   165,
     253,     0,     0,   256,     0,   261,   262,   134,   255,  1021,
    1003,     0,   987,     0,  1024,   790,   789,   734,     0,   784,
     545,   736,     0,   551,   784,   934,   771,     0,     0,     0,
       0,   995,   993,   994,   236,     0,     0,     0,   406,   397,
       0,     0,     0,   213,   324,   326,     0,   396,     0,     0,
       0,   968,   399,     0,   948,   321,   217,   598,     0,     0,
     539,   531,     0,   299,   289,     0,   292,   298,   304,   519,
     991,   399,   991,     0,   930,     0,   889,   399,     0,   399,
     973,   325,   833,   887,   815,   814,   808,     0,   810,   784,
     819,   134,   269,   141,   146,   167,   243,     0,   251,   257,
     134,   259,  1004,     0,     0,   542,     0,   938,   937,   770,
       0,   134,   192,   997,     0,     0,     0,   976,     0,     0,
       0,   237,     0,   968,     0,   362,   358,   364,   695,    28,
       0,   352,     0,   357,   361,   374,     0,   372,   377,     0,
     376,     0,   375,     0,   196,   328,     0,   330,     0,   331,
     332,     0,     0,   914,     0,   599,   597,   608,   606,   300,
       0,     0,   287,   297,     0,     0,     0,     0,   209,   519,
     991,   891,   215,   321,   219,   399,     0,     0,   822,     0,
     265,     0,     0,   134,   246,   166,   258,  1023,   788,     0,
       0,     0,     0,     0,     0,   424,     0,   977,     0,   342,
     346,   421,   422,   356,     0,     0,     0,   337,   659,   658,
     655,   657,   656,   676,   678,   677,   647,   618,   619,   637,
     653,   652,   614,   624,   625,   627,   626,   646,   630,   628,
     629,   631,   632,   633,   634,   635,   636,   638,   639,   640,
     641,   642,   643,   645,   644,   615,   616,   617,   620,   621,
     623,   661,   662,   671,   670,   669,   668,   667,   666,   654,
     673,   663,   664,   665,   648,   649,   650,   651,   674,   675,
     679,   681,   680,   682,   683,   660,   685,   684,   687,   689,
     688,   622,   692,   690,   691,   686,   672,   613,   369,   610,
       0,   338,   390,   391,   389,   382,     0,   383,   339,   416,
       0,     0,     0,     0,   420,     0,   196,   205,   320,     0,
       0,     0,   288,   302,   888,     0,   134,   392,   134,   199,
       0,     0,     0,   211,   991,   813,     0,   134,   244,   147,
     168,     0,   541,   936,   577,   190,   340,   341,   419,   238,
       0,     0,   785,     0,   365,   353,     0,     0,     0,   371,
     373,     0,     0,   378,   385,   386,   384,     0,     0,   327,
     978,     0,     0,     0,   423,     0,   322,     0,   301,     0,
     593,   787,     0,     0,   134,   201,   207,     0,   821,     0,
       0,     0,   170,   343,   123,     0,   344,   345,     0,     0,
     359,   784,   367,   363,   368,   611,   612,     0,   354,   387,
     388,   380,   381,   379,   417,   414,  1001,   333,   329,   418,
       0,   323,   594,   786,     0,   521,   393,     0,   203,     0,
     247,   578,     0,   194,     0,   399,     0,   366,   370,     0,
       0,   833,   335,     0,   591,   518,   523,   245,     0,     0,
     171,   350,     0,   398,   360,   415,   979,     0,   787,   410,
     833,   592,     0,   193,     0,     0,   349,   991,   833,   274,
     411,   412,   413,  1026,   409,     0,     0,     0,   348,     0,
     410,     0,   991,     0,   347,   394,   134,   334,  1026,     0,
     279,   277,     0,   134,     0,     0,   280,     0,     0,   275,
     336,     0,   395,     0,   283,   273,     0,   276,   282,   189,
     284,     0,     0,   271,   281,     0,   272,   286,   285
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   901,   633,   184,  1467,   726,
     348,   586,   590,   349,   587,   591,   122,   123,   124,   125,
     126,   127,   398,   664,   665,   533,   251,  1531,   539,  1447,
    1532,  1773,   857,   343,   581,  1732,  1090,  1270,  1790,   415,
     185,   666,   941,  1150,  1325,   131,   636,   958,   667,   686,
     962,   616,   957,   668,   637,   959,   417,   366,   383,   134,
     943,   904,   887,  1105,  1470,  1202,  1010,  1679,  1535,   803,
    1016,   538,   812,  1018,  1357,   795,   999,  1002,  1191,  1797,
    1798,   655,   656,   680,   681,   353,   354,   360,  1504,  1658,
    1659,  1279,  1394,  1493,  1652,  1781,  1800,  1690,  1736,  1737,
    1738,  1480,  1481,  1482,  1483,  1692,  1693,  1699,  1748,  1486,
    1487,  1491,  1645,  1646,  1647,  1669,  1827,  1395,  1396,   186,
     136,  1813,  1814,  1650,  1398,  1399,  1400,  1401,   137,   244,
     534,   535,   138,   139,   140,   141,   142,   143,   144,   145,
    1516,   146,   940,  1149,   147,   248,   652,   392,   653,   654,
     529,   642,   643,  1226,   644,  1227,   148,   149,   150,   834,
     151,   152,   340,   153,   341,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   847,   848,  1082,   578,   579,   580,
     854,  1721,   154,   638,  1506,   639,  1119,   909,  1296,  1293,
    1638,  1639,   155,   156,   157,   234,   158,   235,   245,   402,
     521,   159,  1038,   838,   160,  1039,   932,   924,  1040,   986,
    1172,   987,  1174,  1175,  1176,   989,  1336,  1337,   990,   772,
     505,   197,   198,   669,   658,   488,  1135,  1136,   758,   759,
     928,   162,   237,   163,   164,   188,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   718,   241,   242,   619,   226,
     227,   721,   722,  1232,  1233,   376,   377,   895,   175,   607,
     176,   651,   177,   332,  1660,  1711,   367,   410,   675,   676,
    1032,  1130,  1277,   884,   885,   817,   818,   819,   333,   334,
     840,  1469,   926
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1476
static const yytype_int16 yypact[] =
{
   -1476,   154, -1476, -1476,  5545, 13204, 13204,   -43, 13204, 13204,
   13204, 11134, 13204, -1476, 13204, 13204, 13204, 13204, 13204, 13204,
   13204, 13204, 13204, 13204, 13204, 13204, 17262, 17262, 11341, 13204,
   17388,   -38,   -32, -1476, -1476, -1476, -1476, -1476,   168, -1476,
   -1476,   136, 13204, -1476,   -32,   -12,    46,   152, -1476,   -32,
   11548, 14346, 11755, -1476, 14294, 10099,   179, 13204, 17453,    51,
   -1476, -1476, -1476,   262,    55,    84,   155,   184,   191,   193,
   -1476, 14346,   208,   211, -1476, -1476, -1476, -1476, -1476, 13204,
     519, 17327, -1476, -1476, 14346, -1476, -1476, -1476, -1476, 14346,
   -1476, 14346, -1476,   233,   223, 14346, 14346, -1476,   218, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, 13204, -1476, -1476,    50,   515,   522,   522,
   -1476,   431,   326,   424, -1476,   312, -1476,    70, -1476,   489,
   -1476, -1476, -1476, -1476, 18201,   575, -1476, -1476,   337,   358,
     375,   387,   398,   411,  4780, -1476, -1476, -1476, -1476,   161,
   -1476,   526,   553,   423, -1476,   146,   426, -1476,   504,     2,
   -1476,   671,   148, -1476, -1476,  2386,   131,   452,   177, -1476,
     133,    91,   457,   100, -1476, -1476,   620, -1476, -1476, -1476,
     545,   473,   543, -1476, -1476,   489,   575, 18113,  2513, 18113,
   13204, 18113, 18113, 14683,   508, 16378, 14683,   661, 14346,   646,
     646,   118,   646,   646,   646,   646,   646,   646,   646,   646,
     646, -1476, -1476, -1476,    47, 13204,   542, -1476, -1476,   565,
     518,   476,   520,   476, 17262, 17701,   527,   731, -1476,   545,
   13204,   542,   612, -1476,   614,   561, -1476,   138, -1476, -1476,
   -1476,   476,   131, 11962, -1476, -1476, 13204,  4847,   773,    90,
   18113,  9892, -1476, 13204, 13204, 14346, -1476, -1476, 15844,   582,
   -1476, 15889, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, 16159, -1476, 16159, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,    78,
      82,   543, -1476, -1476, -1476, -1476,   592,  2018,    89, -1476,
   -1476,   650,   802, -1476,   667, 15345,   744, -1476,   624, 15934,
     632,   637, -1476,    35, 15979, 18245, 18306, 14346,    93, -1476,
      79, -1476, 16737,    98, -1476,   714, -1476,   716, -1476,   830,
     102, 17262, 13204, 13204,   651,   682, -1476, -1476, 16868, 11341,
     105,    77,   264, -1476, 13411, 17262,   619, -1476, 14346, -1476,
     -10,   326, -1476, -1476, -1476, -1476, 17514,   833,   757, -1476,
   -1476, -1476,    66,   656, 18113,   660,  1729,   668,  5752, 13204,
   -1476,   429,   664,   622,   429,   501,   454, -1476, 14346, 16159,
     676, 10306, 14294, -1476, -1476, 13956, -1476, -1476, -1476, -1476,
   -1476,   489, -1476, -1476, -1476, -1476, -1476, -1476, -1476, 13204,
   13204, 13204, 12169, 13204, 13204, 13204, 13204, 13204, 13204, 13204,
   13204, 13204, 13204, 13204, 13204, 13204, 13204, 13204, 13204, 13204,
   13204, 13204, 13204, 13204, 13204, 17388, 13204, -1476, 13204, 13204,
   -1476, 13204, 13618, 14346, 14346, 14346, 14346, 14346, 18201,   764,
     691,  4548, 13204, 13204, 13204, 13204, 13204, 13204, 13204, 13204,
   13204, 13204, 13204, 13204, -1476, -1476, -1476, -1476,  3291, 13204,
   13204, -1476, 10306, 10306, 13204, 13204, 16868,   677,   489, 12376,
   16024, -1476, 13204, -1476,   683,   870,   734,   685,   686, 13761,
     476, 12583, -1476, 12790, -1476,   689,   700,  2291, -1476,    60,
   10306, -1476,  4354, -1476, -1476, 16069, -1476, -1476, 10513, -1476,
   13204, -1476,   794,  9064,   893,   701, 18507,   890,    95,    63,
   -1476, -1476, -1476,   733, -1476, -1476, -1476, 16159,  2474,   709,
     903, 16607, 14346, -1476, -1476, -1476, -1476, -1476,   730, -1476,
   -1476, -1476,   837, 13204,   838,   839, 13204, 13204, 13204, -1476,
     637, -1476, -1476, -1476, -1476, -1476, -1476, -1476,   724, -1476,
   -1476, -1476,   720, -1476, -1476, 14346,   719,   922,    92, 14346,
     736,   924,   255,   302, 18320, -1476, 14346, 13204,   476,    51,
   -1476, -1476, -1476, 16607,   855, -1476,   476,   119,   123,   738,
     739,  2318,   173,   740,   741,   585,   816,   749,   476,   125,
     753, -1476, 18187, 14346, -1476, -1476,   883,  2225,   414, -1476,
   -1476, -1476,   326, -1476, -1476, -1476,   926,   828,   788,   194,
     809, 13204,   829,   953,   762,   814, -1476,   139, -1476, 16159,
   16159,   957,   773,    66, -1476,   769,   963, -1476, 16159,    99,
     391,   156, -1476, -1476, -1476, -1476, -1476, -1476, -1476,  1016,
    4182, -1476, -1476, -1476, -1476,   964,   803, -1476, 17262, 13204,
     774,   968, 18113,   965, -1476, -1476,   850, 14151, 11740, 13615,
   14683, 13204, 18552, 18783, 16804, 10286,  2670,  3931, 11319, 11319,
   11319, 11319,  1173,  1173,  1173,  1173,  1173,   675,   675,   594,
     594,   594,   118,   118,   118, -1476,   646, 18113,   776,   778,
   17746,   777,   977,    -3, 13204,    -1,   542,   187, -1476, -1476,
   -1476,   973,   757, -1476,   489, 13204, 17000, -1476, -1476, 14683,
   -1476, 14683, 14683, 14683, 14683, 14683, 14683, 14683, 14683, 14683,
   14683, 14683, 14683, -1476, 13204,   220, -1476,   159, -1476,   542,
     369,   781,  4367,   789,   793,   791,  5048,   127,   796, -1476,
   18113,  3444, -1476, 14346, -1476,    99,    12, 17262, 18113, 17262,
   17803,    99,   476,   164, -1476,   139,   853,   800, 13204, -1476,
     165, -1476, -1476, -1476,  8857,   569, -1476, -1476, 18113, 18113,
     -32, -1476, -1476, -1476, 13204,   908, 16439, 16607, 14346,  9271,
     805,   806, -1476,   104,   923,   881,   863, -1476,  1006,   815,
    2675, 16159, 16607, 16607, 16607, 16607, 16607,   817,   868,   820,
   16607,   408, -1476,   872, -1476,   818, -1476, 18640, -1476,    19,
   -1476, 13204,   840, 18113,   841,  1010, 11119,  1025, -1476, 18113,
   16114, -1476,   724,   945, -1476,  5959, 18146,   834,   393, -1476,
   18245, 14346,   415, -1476, 18306, 14346, 14346, -1476, -1476, 14854,
   -1476, 18640,  1026, 17262,   842, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476,   120, 14346, 18146,   843, 16868, 17131,
    1027, -1476, -1476, -1476, -1476,   836, -1476, 13204, -1476, -1476,
    5131, -1476, 16159, 18146,   844, -1476, -1476, -1476, -1476,  1038,
     852, 13204, 17514, -1476, -1476, 13618,   856, -1476, 16159, -1476,
     857,  6166,  1024,   134, -1476, -1476,   106,  3291, -1476,  4354,
   -1476, 16159, -1476, -1476,   476, 18113, -1476, 10720, -1476, 16607,
      69,   859, 18146,   828, -1476, -1476, 18712, 13204, -1476, -1476,
   13204, -1476, 13204, -1476, 14899,   860, 10306,   816,  1031,   828,
   16159,  1043,   850, 14346, 17388,   476, 14944,   862, -1476, -1476,
     160,   864, -1476, -1476,  1052, 17311, 17311,  3444, -1476, -1476,
   -1476,  1018,   871,   261,   875, -1476, -1476, -1476, -1476,  1059,
     869,   683,   476,   476, 12997,  4354, -1476, -1476, 14989,   628,
     -32,  9892, -1476,  6373,   874,  6580,   882, 16439, 17262,   877,
     954,   476, 18640,  1073, -1476, -1476, -1476, -1476,   636, -1476,
      53, 16159, -1476,   959, 16159, 14346,  2474, -1476, -1476, -1476,
    1080, -1476,   892,   964,   681,   681,  1028,  1028, 17905,   889,
    1088, 16607, 15631, 17514,  3713, 15488, 16607, 16607, 16607, 16607,
   16309, 16607, 16607, 16607, 16607, 16607, 16607, 16607, 16607, 16607,
   16607, 16607, 16607, 16607, 16607, 16607, 16607, 16607, 16607, 16607,
   16607, 16607, 16607, 16607, 14346, -1476, 18113, 13204, 13204, 13204,
   -1476, -1476, -1476, 13204, 13204, -1476,   637, -1476,  1021, -1476,
   -1476, 14346, -1476, -1476, 14346, -1476, -1476, -1476, -1476, 16607,
     476, -1476,   585, -1476,   679,  1089, -1476, -1476,   128,   904,
     476, 10927, -1476,  3899, -1476,  5338,   757,  1089, -1476,   428,
     347, -1476, 18113,   971,   905, -1476,   910,  1024, -1476, 16159,
     773, 16159,    68,  1093,  1030,   172, -1476,   542,   176, -1476,
   -1476, 17262, 13204, 18113, 18640,   913,    69, -1476,   912,    69,
     917, 18712, 18113, 17848,   927, 10306,   928,   925, 16159,   935,
     937, 16159,   828, -1476,   561,   412, 10306, 13204, -1476, -1476,
   -1476, -1476, -1476, -1476,   998,   933,  1139,  1067,  3444,  1007,
   -1476, 17514,  3444, -1476, -1476, -1476, 17262, 18113, -1476,   -32,
    1126,  1083,  9892, -1476, -1476, -1476,   956, 13204,   954,   476,
   16868, 16439,   966, 16607,  6787,   643,   958, 13204,    59,   299,
   -1476,   992, 16159, -1476,  1036, -1476, 16113,  1140,   976, 16607,
   -1476, 16607, -1476,   980, -1476,  1037,  1163,   982, -1476, -1476,
   -1476, 17950,   974,  1175, 11947, 15342, 18676, 16607, 18597,  3025,
    3112, 10700,  2982,  4737, 12354, 12354, 12354, 12354,  1663,  1663,
    1663,  1663,  1663,   848,   848,   681,   681,   681,  1028,  1028,
    1028,  1028, -1476, 18113, 13396, 18113, -1476, 18113, -1476,   984,
   -1476, -1476, -1476, 18640, 14346, 16159, 16159, -1476, 18146,   101,
   -1476, 16868, -1476, -1476, 14683,   986, -1476,   985,   635, -1476,
     112, 13204, -1476, -1476, -1476, 13204, -1476, 13204, -1476,   773,
   -1476, -1476,   189,  1177,  1114, 13204, -1476,   996,   476, 18113,
    1024,   999, -1476,  1000,    69, 13204, 10306,  1002, -1476, -1476,
     757, -1476, -1476,   995,  1001,  1008, -1476,  1004,  3444, -1476,
    3444, -1476, -1476,  1009, -1476,  1074,  1029,  1190, -1476,   476,
    1181, -1476,  1032, -1476, -1476,  1034,  1039,   129, -1476, -1476,
   18640,  1035,  1040, -1476, 15799, -1476, -1476, -1476, -1476, -1476,
   -1476, 16159, -1476, 16159, -1476, 18640, 18007, -1476, 16607, 17514,
   -1476, -1476, 16607, -1476, 16607, -1476, 18748, 16607, 13204,  1041,
    6994,   679, -1476, -1476, -1476,   645, 14489, 18146,  1120, -1476,
   16537,  1072, 18044, -1476, -1476, -1476,   764,  3967,   107,   108,
    1044,   757,   691,   130, -1476, -1476, -1476,  1085, 15034, 15079,
   18113, -1476,    71,  1229,  1167, 13204, -1476, 18113, 10306,  1135,
    1024,  1005,  1024,  1048, 18113,  1049, -1476,  1234,  1050,  1560,
   -1476, -1476,    69, -1476, -1476,  1119, -1476,  3444, -1476, 17514,
   -1476, -1476,  8857, -1476, -1476, -1476, -1476,  9478, -1476, -1476,
   -1476,  8857, -1476,  1054, 16607, 18640,  1124, 18640, 18052, 18748,
   13189, -1476, -1476, -1476, 18146, 18146, 14346, -1476,  1243, 15631,
      76, -1476, 14489,   757, 18126, -1476,  1086, -1476,   109,  1057,
     113, -1476, 15151, -1476, -1476, -1476,   114, -1476, -1476, 18027,
   -1476,  1060, -1476,  1185,   489, -1476, 14684, -1476, 14684, -1476,
   -1476,  1247,   764, -1476, 13904, -1476, -1476, -1476, -1476,  1257,
    1192, 13204, -1476, 18113,  1075,  1077,  1070,   577, -1476,  1135,
    1024, -1476, -1476, -1476, -1476,  1580,  1078,  3444, -1476,  1142,
    8857,  9685,  9478, -1476, -1476, -1476,  8857, -1476, 18640, 16607,
   16607, 13204,  7201,  1076,  1079, -1476, 16607, -1476, 18146, -1476,
   -1476, -1476, -1476, -1476, 16159,   727, 16537, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,   135, -1476,
    1072, -1476, -1476, -1476, -1476, -1476,    86,   524, -1476,  1261,
     115, 15345,  1185,  1268, -1476, 16159,   489, -1476, -1476,  1081,
    1270, 13204, -1476, 18113, -1476,   383, -1476, -1476, -1476, -1476,
    1082,   577, 14099, -1476,  1024, -1476,  3444, -1476, -1476, -1476,
   -1476,  7408, 18640, 18640, 11533, -1476, -1476, -1476, 18640, -1476,
    3014,   126,  1279,  1091, -1476, -1476, 16607, 15151, 15151,  1241,
   -1476, 18027, 18027,   606, -1476, -1476, -1476, 16607,  1216, -1476,
    1127,  1098,   116, 16607, -1476, 14346, -1476, 16607, 18113,  1222,
   -1476,  1294,  7615,  7822, -1476, -1476, -1476,   577, -1476,  8029,
    1102,  1226,  1196, -1476,  1210,  1161, -1476, -1476,  1215, 16159,
   -1476,   727, -1476, -1476, 18640, -1476, -1476,  1151, -1476,  1283,
   -1476, -1476, -1476, -1476, 18640,  1303,   585, -1476, -1476, 18640,
    1122, 18640, -1476,   463,  1118, -1476, -1476,  8236, -1476,  1121,
   -1476, -1476,  1128,  1155, 14346,   691,  1147, -1476, -1476, 16607,
     141,    74, -1476,  1249, -1476, -1476, -1476, -1476, 18146,   834,
   -1476,  1164, 14346,   560, -1476, 18640, -1476,  1131,  1318,   449,
      74, -1476,  1253, -1476, 18146,  1132, -1476,  1024,   132, -1476,
   -1476, -1476, -1476, 16159, -1476,  1134,  1136,   117, -1476,   610,
     449,   370,  1024,  1137, -1476, -1476, -1476, -1476, 16159,   300,
    1325,  1263,   610, -1476,  8443,   382,  1327,  1266, 13204, -1476,
   -1476,  8650, -1476,   327,  1334,  1276, 13204, -1476, 18113, -1476,
    1343,  1278, 13204, -1476, 18113, 13204, -1476, 18113, 18113
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1476, -1476, -1476,  -558, -1476, -1476, -1476,   460,    43,   -46,
   -1476, -1476, -1476,   765,   502,   497,    25,  1542,  2358, -1476,
    2644, -1476,  -495, -1476,    34, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476, -1476,  -426, -1476, -1476,  -158,
     190,    29, -1476, -1476, -1476, -1476, -1476, -1476,    36, -1476,
   -1476, -1476, -1476,    37, -1476, -1476,   896,   902,   901,   -96,
     405,  -865,   413,   465,  -435,   174,  -919, -1476,  -161, -1476,
   -1476, -1476, -1476,  -731,    20, -1476, -1476, -1476, -1476,  -421,
   -1476,  -617, -1476,  -438, -1476, -1476,   779, -1476,  -146, -1476,
   -1476, -1045, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476, -1476,  -174, -1476,   -91, -1476, -1476, -1476, -1476, -1476,
    -256, -1476,    -6,  -872, -1476, -1475,  -444, -1476,  -139,    97,
    -133,  -430, -1476,  -261, -1476, -1476, -1476,     9,   -41,     0,
      42,  -737,  -423, -1476, -1476,     8, -1476, -1476,    -5,   -55,
    -121, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
    -610,  -842, -1476, -1476, -1476, -1476, -1476,  1547, -1476, -1476,
   -1476, -1476,   929, -1476, -1476,   313, -1476,   832, -1476, -1476,
   -1476, -1476, -1476, -1476, -1476,   320, -1476,   835, -1476, -1476,
     552, -1476,   287, -1476, -1476, -1476, -1476, -1476, -1476, -1476,
   -1476,  -860, -1476,  2071,    31, -1476,   884,  -396, -1476, -1476,
     243,  3433,  3225, -1476, -1476,   367,  -184,  -652, -1476, -1476,
     433,   234,  -702,   236, -1476, -1476, -1476, -1476, -1476,   421,
   -1476, -1476, -1476,    30,  -893,  -160,  -422,  -412, -1476,   488,
    -115, -1476, -1476,    38,    41,   286, -1476, -1476,  1240,   -36,
   -1476,  -340,    27,  -358,   121,  -307, -1476, -1476,  -436,  1051,
   -1476, -1476, -1476, -1476, -1476,   641,   627, -1476, -1476, -1476,
    -330,  -697, -1476,  1012,  -970, -1476,   -70,  -169,    39,   597,
   -1476, -1037,    40,  -338,   317,   396, -1476, -1476, -1476, -1476,
     351,  1833, -1092
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1011
static const yytype_int16 yytable[] =
{
     187,   189,   422,   191,   192,   193,   195,   196,   330,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   384,   469,   225,   228,   387,   388,   497,   647,   938,
    1131,   794,   247,   130,   161,   961,   920,   250,   128,  1302,
     132,   133,   919,   514,   252,   258,   646,   261,   737,   256,
     339,   491,   344,   422,   763,   764,   648,   240,   395,   715,
     767,   233,   518,   338,   418,   468,   756,  1006,   238,   988,
    1123,   239,   393,  1288,   250,   900,   757,   397,  1148,   412,
    1020,   522,   787,   350,   249,  1548,   855,   -78,  1198,  1355,
    1299,   -43,   -78,   810,  1159,  1701,   -43,   329,   -42,   530,
     790,   135,   594,   -42,   808,  1385,   380,   599,   394,   381,
     791,   604,    13,  1021,   530,    13,  1496,  1498,  -355,  1702,
      13,   370,  1556,  1640,  1708,  1708,  1548,   523,   873,  1102,
     211,   351,   530,   506,   889,  1739,   889,   889,   889,   889,
     211,  1132,   486,   487,   622,   408,   582,    13,  1303,  1696,
    1102,  1509,   719,  -697,     3,  1207,  1208,   190,  -868,   921,
     508,   785,   243,  1407,   359,  1697,   489,   494,   246,  -104,
    1074,  -103,   455,   357,   507,   516,  -524,   596,    13,   397,
      13,   358,   761,  1698,   456,   500,  1133,   765,   253,   515,
     861,   409,   634,   635,   129,   373,  1725,  -104,   489,  -103,
    -867,  1225,  -879,  -557,   583,  -908,   494,   486,   487,  1386,
     394,   623,  -881,  -866,  1387,  -869,    60,    61,    62,   178,
    1388,   419,  1389,  -873,  1412,   394,  -870,  -872,  -554,   486,
     487,  -911,  -910,  1304,   408,  -698,  1510,   400,   525,  -853,
    -556,   525,  -704,  -854,  -556,   906,   254,   408,   250,   536,
     547,  -699,  1768,   399,  -874,   116,   352,  1210,   470,  1390,
    1391,  1356,  1392,   687,  -294,   116,   811,  1421,   385,  1413,
    -294,  1134,   413,  1419,  1427,  -278,  1429,  1022,  1549,  1550,
     -78,   527,  1348,   420,   -43,   532,  1205,  1703,  1209,   557,
     165,   -42,   531,  1103,  -868,   595,   809,  1324,   389,  1740,
     600,  1003,   490,   495,   605,  1393,  1005,   621,   610,  1497,
    1499,  -355,   221,   223,  1796,  1557,  1641,  1709,  1758,  1824,
     874,   774,   609,  1145,   875,   421,   890,   613,   974,  1280,
    1446,  1503,  -786,  -786,   490,  -786,  -867,   422,  -705,  1335,
     768,  -908,   495,  1115,   371,   685,  -877,  1179,  -876,  -866,
     907,  -869,   255,   865,  1414,   361,  -880,   250,   394,  -873,
    -883,  -878,  -870,  -872,   225,   908,   330,  -911,  -910,   627,
     588,   592,   593,   493,   879,  -853,   498,   493,   329,  -854,
    1836,   355,   342,  1517,   362,  1519,  1525,   956,   356,   385,
    -874,   363,  -841,   364,   670,   384,   731,   732,   418,   396,
     866,  1207,  1208,   632,   608,  1829,   682,  1850,   368,  1180,
     408,   369,   624,   374,   375,   390,  1468,  1843,  1719,  1287,
    -841,   391,   657,   386,   688,   689,   690,   692,   693,   694,
     695,   696,   697,   698,   699,   700,   701,   702,   703,   704,
     705,   706,   707,   708,   709,   710,   711,   712,   713,   714,
    1830,   716,  1108,   717,   717,   329,   720,   408,  -558,   913,
    1345,   407,  1844,  1720,   121,  1837,   739,   741,   742,   743,
     744,   745,   746,   747,   748,   749,   750,   751,   752,   738,
    1338,   408,   240,  1671,   717,   762,   233,   682,   682,   717,
     766,  1091,  1851,   238,   739,   135,   239,   770,  1783,   927,
     929,   396,  1551,  1358,  1294,   725,   778,  1138,   780,   371,
     510,   259,   411,  1094,   328,   682,   517,  1139,  1156,  1204,
    1301,   486,   487,   798,   414,   799,  1653,  1456,  1654,  1311,
     469,   365,  1313,   165,   371,  1831,   797,   165,   953,   423,
     629,  -844,  -700,  1784,   486,   487,   784,  1845,   408,   382,
    1295,   365,   329,  1704,   647,   365,   365,   955,   843,  -983,
     424,   846,   849,   850,  1810,  1811,  1812,   802,  1164,  -844,
     408,  1705,   646,   468,  1706,  -983,   963,   425,   374,   375,
     409,   371,   648,   727,  -842,  1289,   910,   629,   129,   426,
     967,  -881,   869,   458,   365,   371,   409,  1529,  1290,   371,
     427,   401,   371,   374,   375,   372,   486,   487,   404,   760,
     858,  -983,  -842,   428,   862,   927,   929,   945,   899,   350,
     459,   995,   929,  1428,  -983,   460,  1434,  -983,  1435,   461,
     727,  1291,   882,   883,  1027,  1751,   394,  1727,   598,  1385,
     786,  1000,  1001,   792,   452,   453,   454,   606,   455,   611,
     374,   375,   492,  1752,   618,   462,  1753,  -875,   504,   734,
     456,   628,   518,   673,   374,   375,   996,   373,   374,   375,
    1075,   374,   375,   496,   935,    60,    61,    62,   178,   179,
     419,    13,    53,   657,   165,   647,   946,  -555,  1282,  1411,
      60,    61,    62,   178,   179,   419,  -698,  1423,   378,   371,
    1189,  1190,   371,   646,  1501,   629,   503,   121,   629,   672,
     501,   121,   456,   648,   409,   537,   509,  1317,  -879,   954,
     493,  1821,   449,   450,   451,   452,   453,   454,  1327,   455,
     195,  1070,  1071,  1072,   512,  1528,  1835,  1206,  1207,  1208,
     513,   456,   420,  1386,  1352,  1207,  1208,  1073,  1387,   966,
      60,    61,    62,   178,  1388,   419,  1389,   420,   403,   405,
     406,   520,  1806,  -696,  1347,   519,   470,   630,   374,   375,
    1819,   374,   375,  1464,  1465,  1380,  1552,  1275,  1276,  1667,
    1668,   528,   618,   998,   541,  1832,    60,    61,    62,    63,
      64,   419,   548,  1390,  1391,   556,  1392,    70,   463,   250,
    1004, -1010,    33,    34,    35,  1183,    60,    61,    62,   178,
     179,   419,  1825,  1826,   212,  1526,   551,   420,   552,   165,
     562,   563,   564,   647,   558,  1675,   559,   565,   566,  1749,
    1750,   567,   568,   464,   561,   465,  1076,  1745,  1746,  1406,
     601,   646,   602,  1015,   603,  1403,  1442,   649,   466,  1218,
     467,   648,   614,   420,   615,   650,  1222,   659,   121,  1030,
    1033,   660,  1451,    74,    75,    76,    77,    78,   674,   662,
     671,    53,   328,   420,   214,   365,  -129,   684,  1425,   773,
      82,    83,   624,   771,   800,   588,   775,   776,  1799,   592,
     781,   135,  1113,  1163,    92,  1067,  1068,  1069,  1070,  1071,
    1072,   782,   530,   804,   807,   547,  1122,  1799,    97,   820,
     219,   219,   821,   853,  1073,  1820,   841,   842,   844,   845,
     856,   859,   556,   365,   729,   365,   365,   365,   365,   130,
     161,   860,  1143,   864,   128,   872,   132,   133,   863,   876,
     877,   880,  1151,   881,   886,  1152,  1530,  1153,   755,   888,
     897,   682,   135,   657,   891,  1536,   902,   903,   725,   905,
    -720,   911,   912,   914,   934,   915,  1542,   922,  1307,   556,
     657,   918,   923,   931,  1728,   936,   933,   937,   942,   939,
    1514,   951,   789,   948,   129,   949,   952,   960,   968,  1187,
     970,   240,  1331,   121,   971,   233,   944,   135,   972,   625,
    1192,   997,   238,   631,  -702,   239,  1007,  1017,  1019,  1385,
    1023,   647,   839,  1024,  1025,  1026,  1028,  1041,   135,  1042,
    1043,  1046,   965,  1045,  1079,  1086,  1077,  1078,   625,   646,
     631,   625,   631,   631,  1083,  1193,  1124,  1089,  1681,   648,
    1099,  1111,  1370,  1112,  1101,   129,  1107,  1118,   760,  1375,
     792,    13,  1120,  1121,  1129,  1127,   868,  1161,  1125,  1146,
    1155,  1158,  1166,   992,  -882,   993,  1167,  1177,  1182,  1764,
    1184,  1178,  1263,  1264,  1265,  1181,  1195,  1200,   846,  1267,
     165,   647,   894,   896,  1197,  1224,  1201,  1203,  1230,  1216,
     129,  1212,  1011,  1217,  1073,   165,  1220,  1221,  1278,   646,
     135,  1269,   135,  1297,  1281,   956,  1284,  1305,   219,   648,
    1306,   129,  1298,  1386,  1310,  1312,   792,  1314,  1387,  1283,
      60,    61,    62,   178,  1388,   419,  1389,  1316,  1319,  1318,
    1328,    60,    61,    62,    63,    64,   419,  1309,  1321,  1322,
    1329,   165,    70,   463,   130,   161,  1809,   365,  1330,   128,
     682,   132,   133,  1440,   981,  1334,  1341,  1342,  1344,  1100,
    1353,   682,  1284,  1390,  1391,  1359,  1392,  1349,  1361,  1368,
    1363,  1722,  1369,  1723,   618,  1110,   657,  1364,  1373,   657,
     465,  1367,  1729,  1371,  1374,  1379,   165,   420,  1405,  1340,
    1404,  1415,   250,   129,  1416,   129,  1418,  1430,   420,  1439,
    1420,  1422,  1354,  1426,  1431,  1433,  1437,   165,  1432,  1518,
    1436,  1441,   135, -1011, -1011, -1011, -1011, -1011,   447,   448,
     449,   450,   451,   452,   453,   454,  1343,   455,  1472,  1767,
    1438,   985,  1485,   991,  1443,  1444,   219,  1448,  1385,   456,
    1445,  1505,  1449,  1511,  1461,   219,  1500,  1512,  1515,  1520,
    1521,  1527,   219,  1523,   121,  1537,  1539,  1546,  1554,   219,
    1555,  1655,  1648,  1502,   422,  1649,   222,   222,  1013,   121,
     645,  1661,  1662,  1666,  1676,  1707,  1664,  1665,  1686,  1674,
      13,  1687,  1713,  1716,  1717,  1724,  1408,   165,  1741,   165,
    1409,   165,  1410,  1011,  1199,  1743,  1755,  1747,  1757,  1756,
    1417,   135,  1762,  1763,  1770,   129,  1771,  1772,  -351,  1402,
    1424,   682,  1774,  1775,  1778,   121,  1702,  1779,  1402,  1785,
    1794,  1093,  1782,  1787,  1789,  1096,  1097,  1808,  1788,  1801,
    1804,  1834,  1807,  1816,  1818,  1822,  1651,  1823,  1841,  1838,
    1833,  1846,  1386,  1839,   657,  1104,  1847,  1387,  1852,    60,
      61,    62,   178,  1388,   419,  1389,  1853,  1855,  1856,   867,
     121,  1095,  1092,  1803,   733,   728,   730,  1162,  1117,  1817,
    1157,  1680,  1346,  1460,  1450,   556,  1397,  1672,   870,  1815,
     219,   121,  1695,  1553,  1700,  1397,  1492,   755,  1840,   789,
    1828,  1712,  1390,  1391,   129,  1392,  1545,  1473,  1670,  1268,
     740,   165,   851,  1266,  1085,   852,  1292,  1326,  1223,  1173,
    1513,  1332,  1185,   682,  1333,  1137,   420,  1031,  1780,  1274,
     620,  1463,  1215,   365,   683,  1262,     0,  1308,     0,     0,
       0,     0,     0,     0,     0,  1171,  1171,   985,  1522,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1402,     0,     0,     0,   789,     0,  1402,     0,  1402,
       0,   121,   657,   121,   222,   121,     0,     0,     0,     0,
       0,     0,  1339,     0,     0,     0,     0,   135,   165,     0,
       0,  1534,     0,     0,     0,  1214,   618,  1011,     0,     0,
     165,     0,     0,     0,     0,     0,     0,     0,  1715,   470,
       0,     0,   556,     0,     0,   556,  1663,     0,  1742,     0,
       0,     0,  1547,     0,     0,     0,     0,     0,  1397,     0,
       0,     0,     0,     0,  1397,     0,  1397,     0,     0,     0,
       0,     0,     0,     0,   839,     0,  1684,     0,     0,   135,
       0,     0,     0,     0,     0,     0,     0,     0,   135,     0,
       0,  1271,     0,     0,  1272,  1402,     0,     0,     0,     0,
       0,     0,   219,     0,  1385,  1678,  1534,   618,   216,   216,
     129,     0,   231,   217,   217,   121,     0,     0,     0,     0,
       0,     0,     0,     0,  1385,     0,  1494,     0,     0,     0,
       0,     0,   222,     0,     0,     0,   231,     0,     0,     0,
       0,   222,     0,   612,     0,  1710,    13,     0,   222,     0,
       0,     0,     0,     0,     0,   222,     0,     0,     0,     0,
     219,     0,  1397,     0,     0,     0,    13,   135,     0,     0,
       0,     0,   129,   135,     0,     0,  1792,     0,   985,   135,
       0,   129,   985,     0,     0,  1760,     0,     0,     0,     0,
       0,     0,   121,     0,     0,     0,  1718,     0,     0,     0,
     422,   219,     0,   219,   121,     0,   165,     0,  1386,     0,
       0,     0,     0,  1387,     0,    60,    61,    62,   178,  1388,
     419,  1389,     0,     0,     0,     0,     0,     0,  1386,     0,
     219,     0,  1656,  1387,   329,    60,    61,    62,   178,  1388,
     419,  1389,     0, -1011, -1011, -1011, -1011, -1011,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,     0,     0,  1390,  1391,
     129,  1392,     0,     0,     0,     0,   129,     0,   165,  1073,
       0,     0,   129,   165,  1381,     0,   222,   165,  1390,  1391,
       0,  1392,   420,   499,   472,   473,   474,   475,   476,   477,
     478,   479,   480,   481,   482,   483,     0,   219,     0,     0,
       0,     0,   420,     0,  1524,     0,   216,     0,     0,     0,
       0,   217,   219,   219,     0,     0,     0,     0,   135,     0,
       0,     0,     0,     0,  1673,     0,     0,     0,   985,     0,
     985,     0,     0,   484,   485,     0,   645,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   657,     0,   231,     0,   231,   165,   165,   165,   135,
     135,     0,   165,     0,     0,     0,   135,     0,   165,     0,
     657,     0,     0,  1848,     0,     0,     0,     0,   657,     0,
     121,  1854,     0,     0,     0,     0,   328,  1857,     0,     0,
    1858,     0,  1490,     0,     0,     0,     0,     0,     0,   486,
     487,     0,     0,     0,   135,     0,     0,     0,     0,   231,
       0,   129,  1793,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   335,     0,     0,
       0,   219,   219,     0,   216,     0,     0,   985,     0,   217,
       0,     0,   121,   216,     0,     0,     0,   121,   217,     0,
     216,   121,   129,   129,     0,   217,     0,   216,   222,   129,
       0,     0,   217,     0,     0,     0,   365,   645,   231,   556,
     661,   135,   328,     0,     0,     0,     0,     0,   135,     0,
       0,     0,  1637,     0,     0,     0,     0,     0,     0,  1644,
       0,   231,     0,     0,   231,     0,   328,   129,   328,     0,
       0,     0,     0,     0,   328,     0,     0,   165,     0,     0,
       0,     0,     0,     0,     0,     0,   222,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   985,     0,     0,
     121,   121,   121,     0,     0,     0,   121,   231,     0,     0,
       0,     0,   121,     0,     0,     0,     0,     0,   165,   165,
       0,     0,     0,     0,     0,   165,     0,   222,     0,   222,
       0,     0,     0,     0,   129,   219,     0,     0,     0,     0,
       0,   129,     0,     0,     0,     0,     0,     0,   216,     0,
       0,     0,     0,   217,     0,     0,   222,   271,     0,     0,
       0,     0,     0,   165,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   645,     0,     0,     0,     0,
     219,     0,     0,     0,   273,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   219,   219,     0,     0,     0,   231,
     231,     0,     0,   831,     0,     0,    36,   218,   218,     0,
       0,   232,     0,     0,   545,     0,   546,     0,     0,     0,
       0,   556,     0,   222,     0,     0,     0,    48,     0,     0,
     165,     0,     0,     0,     0,   549,     0,   165,   222,   222,
       0,     0,   328,     0,     0,     0,   985,     0,     0,     0,
       0,   121,     0,     0,     0,   831,     0,     0,     0,     0,
    1734,     0,   543,   544,     0,     0,     0,  1637,  1637,     0,
     550,  1644,  1644,     0,     0,   219,     0,     0,     0,     0,
     182,     0,     0,    84,   322,   365,    86,    87,     0,    88,
     183,    90,   121,   121,     0,     0,     0,     0,     0,   121,
       0,   231,   231,     0,   326,     0,     0,     0,     0,     0,
     231,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   327,     0,
     216,     0,     0,     0,     0,   217,     0,   121,     0,     0,
       0,     0,     0,     0,  1791,   429,   430,   431,     0,     0,
       0,     0,   677,     0,     0,   335,     0,   222,   222,     0,
       0,     0,  1805,   645,   432,     0,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   216,   455,
       0,     0,     0,   217,     0,     0,     0,     0,     0,     0,
       0,   456,     0,     0,   121,   218,     0,     0,     0,     0,
       0,   121,     0,     0,     0,   499,   472,   473,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,     0,   216,
       0,   216,     0,   645,   217,     0,   217,     0,     0,     0,
       0,     0,   499,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,     0,     0,     0,   216,   831,
       0,     0,     0,   217,     0,   484,   485,     0,     0,     0,
       0,     0,   231,   231,   831,   831,   831,   831,   831,     0,
       0,     0,   831,     0,     0,     0,     0,     0,     0,     0,
     813,   222,   484,   485,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   231,     0,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,     0,     0,   216,     0,     0,     0,     0,
     217,   486,   487,   218,     0,     0,   222,     0,   231,   898,
     216,   216,   218,     0,     0,   217,   217,     0,     0,   218,
     222,   222,     0,     0,   231,   231,   218,     0,   486,   487,
     484,   485,     0,     0,   231,     0,     0,   218,     0,     0,
     231,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   231,     0,     0,     0,     0,     0,     0,
       0,   831,   916,   917,   231,     0,     0,     0,     0,     0,
       0,   925,   783,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   231,   814,     0,     0,   231,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   486,   487,     0,   878,
       0,   222,     0,     0,     0,     0,   232,   499,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   216,
     216,     0,    36,     0,   217,   217,     0,     0,     0,     0,
       0,   815,     0,   231,     0,     0,   231,   218,   231,     0,
       0,     0,     0,    48,     0,     0,     0,   484,   485,     0,
       0,     0,     0,   831,     0,   231,     0,     0,   831,   831,
     831,   831,   831,   831,   831,   831,   831,   831,   831,   831,
     831,   831,   831,   831,   831,   831,   831,   831,   831,   831,
     831,   831,   831,   831,   831,   831,     0,     0,     0,     0,
       0,     0,   835,     0,     0,     0,   182,     0,     0,    84,
       0,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,   831,     0,   486,   487,     0,     0,     0,     0,     0,
       0,     0,     0,   677,   677,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   231,     0,   231,   835,     0,     0,     0,     0,     0,
       0,     0,     0,   216,     0,     0,     0,     0,   217,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   331,     0,
     231,     0,     0,   231,   271,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   231,   455,     0,     0,     0,   216,     0,
       0,   273,     0,   217,     0,  1116,   456,     0,     0,     0,
       0,     0,   216,   216,     0,   831,     0,   217,   217,   218,
       0,  1126,     0,    36,   231,     0,     0,     0,   231,     0,
       0,   831,     0,   831,  1140,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,   831,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1160,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   218,     0,   543,
     544,     0,     0,     0,     0,     0,     0,   231,   231,     0,
     231,     0,     0,   216,     0,     0,     0,   182,   217,     0,
      84,   322,     0,    86,    87,     0,    88,   183,    90,     0,
    1029,     0,     0,     0,     0,     0,     0,     0,   218,     0,
     218,   326,     0,     0,  1211,     0,     0,  1213,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   327,     0,   218,   835,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   835,   835,   835,   835,   835,     0,     0,
       0,   835,     0,   231,     0,   231,     0,     0,     0,   832,
     831,   231,     0,     0,   831,   331,   831,   331,     0,   831,
       0,     0,     0,     0,     0,     0,     0,  1088,   231,   231,
       0,     0,   231,     0,     0,     0,     0,     0,     0,   231,
       0,     0,     0,     0,   218,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1106,     0,   218,
     218,   832,  1300,     0,   925,     0,     0,     0,     0,     0,
       0,   331,     0,     0,  1106,     0,     0,     0,     0,     0,
       0,   231,     0,   218,     0,     0,     0,     0,     0,     0,
       0,  1320,     0,     0,  1323,     0,   831,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   231,   231,     0,     0,
     835,     0,     0,  1147,   231,     0,   231,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,  1072,   232,     0,     0,   231,     0,
     231,     0,     0,     0,     0,  1360,   231,     0,  1073,  1140,
       0,     0,     0,   331,     0,     0,   331,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,   218,   218,
       0,   831,   831,     0,     0,     0,     0,     0,   831,     0,
     231,  1073,    36,     0,     0,     0,   231,     0,   231,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1382,  1383,
       0,     0,   835,    48,   218,     0,     0,   835,   835,   835,
     835,   835,   835,   835,   835,   835,   835,   835,   835,   835,
     835,   835,   835,   835,   835,   835,   835,   835,   835,   835,
     835,   835,   835,   835,   835,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,  1072,   832,   182,     0,     0,    84,
     835,     0,    86,    87,     0,    88,   183,    90,  1073,     0,
     832,   832,   832,   832,   832,     0,     0,     0,   832,     0,
       0,   331,   816,     0,  1452,   833,  1453,   231,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   218,     0,   231,     0,     0,     0,  1733,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1495,     0,   231,     0,     0,     0,     0,     0,   831,     0,
       0,     0,     0,     0,     0,     0,     0,   833,     0,   831,
       0,     0,   218,     0,     0,   831,     0,   218,     0,   831,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   218,   218,     0,   835,     0,     0,     0,     0,     0,
       0,   231,     0,     0,     0,     0,     0,     0,     0,     0,
     835,     0,   835,   331,   331,     0,     0,   832,     0,     0,
       0,     0,   331,     0,     0,     0,     0,     0,   835,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   831,     0,     0,     0,     0,     0,     0,     0,     0,
     231,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   231,     0,     0,  1384,
       0,     0,   218,     0,     0,   231,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
     231,   211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1691,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,   832,
       0,     0,     0,     0,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,   832,     0,     0,     0,     0,     0,     0,     0,   835,
     218,     0,     0,   835,     0,   835,     0,   753,   835,    86,
      87,   833,    88,   183,    90,     0,     0,   832,  1471,   220,
     220,  1484,     0,   236,   331,   331,   833,   833,   833,   833,
     833,     0,     0,     0,   833,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,  1714,     0,
       0,   975,   976,     0,   754,     0,   116,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     218,   977,     0,     0,     0,     0,     0,     0,     0,   978,
     979,   980,    36,     0,     0,   835,     0,     0,     0,     0,
       0,   981,     0,     0,     0,  1543,  1544,     0,     0,     0,
       0,     0,     0,    48,     0,  1484,   331,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   832,   331,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1776,     0,     0,   331,     0,   832,   982,   832,
       0,     0,     0,   833,     0,     0,     0,     0,     0,     0,
       0,   983,     0,     0,     0,   832,     0,     0,     0,     0,
       0,     0,    86,    87,   331,    88,   183,    90,     0,     0,
     835,   835,     0,     0,     0,     0,     0,   835,     0,  1689,
     984,     0,     0,     0,     0,     0,     0,  1484,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,     0,     0,   925,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   220,     0,     0,
       0,   925,     0,     0,     0,   331,     0,     0,   331,     0,
     816,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   833,     0,     0,     0,     0,
     833,   833,   833,   833,   833,   833,   833,   833,   833,   833,
     833,   833,   833,   833,   833,   833,   833,   833,   833,   833,
     833,   833,   833,   833,   833,   833,   833,   833,     0,     0,
       0,     0,     0,  1047,  1048,  1049,   832,     0,     0,     0,
     832,     0,   832,     0,     0,   832,     0,     0,     0,     0,
       0,     0,  1050,   833,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,     0,   835,     0,     0,
       0,     0,     0,   331,     0,   331,   837,     0,   835,  1073,
       0,     0,     0,     0,   835,   220,     0,     0,   835,     0,
       0,     0,     0,     0,   220,     0,     0,     0,     0,     0,
       0,   220,   331,     0,     0,   331,     0,     0,   220,     0,
       0,     0,   832,     0,     0,     0,     0,     0,     0,   236,
       0,     0,     0,     0,     0,     0,     0,     0,   871,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   833,     0,     0,
     835,     0,     0,     0,     0,     0,   331,     0,     0,  1802,
     331,     0,     0,   833,     0,   833,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1471,     0,     0,     0,     0,
       0,   833,     0,     0,     0,     0,     0,     0,   236,     0,
       0,     0,     0,     0,     0,     0,     0,   832,   832,     0,
       0,     0,     0,     0,   832,     0,     0,     0,     0,   429,
     430,   431,     0,     0,  1228,     0,     0,     0,     0,   331,
     331,     0,     0,     0,     0,     0,     0,     0,   432,   220,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   456,     0,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   836,   455,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   271,   456,     0,     0,
       0,     0,     0,     0,     0,   331,     0,   331,     0,     0,
       0,     0,   833,     0,     0,     0,   833,     0,   833,     0,
       0,   833,     0,   273,     0,     0,     0,     0,     0,     0,
     331,     0,  1012,     0,     0,     0,   836,     0,     0,     0,
       0,   331,     0,     0,     0,    36,     0,  1034,  1035,  1036,
    1037,     0,     0,     0,   832,  1044,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   832,    48,     0,     0,     0,
       0,   832,     0,     0,  -398,   832,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   178,   179,   419,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   833,     0,
       0,   543,   544,     0,     0,     0,  1285,     0,     0,     0,
       0,   220,     0,     0,     0,     0,   331,     0,     0,   182,
       0,     0,    84,   322,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,     0,     0,     0,   832,     0,     0,
     331,     0,   331,   326,     0,     0,     0,     0,   331,   420,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,  1144,     0,     0,   327,     0,   220,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   833,   833,     0,     0,     0,     0,     0,
     833,     0,   429,   430,   431,     0,     0,     0,   331,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     220,   432,   220,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,     0,   455,     0,     0,   220,
     836,     0,     0,     0,     0,     0,     0,     0,   456,     0,
       0,     0,     0,     0,     0,   836,   836,   836,   836,   836,
       0,     0,     0,   836,     0,     0,     0,     0,     0,     0,
       0,  1231,  1234,  1235,  1236,  1238,  1239,  1240,  1241,  1242,
    1243,  1244,  1245,  1246,  1247,  1248,  1249,  1250,  1251,  1252,
    1253,  1254,  1255,  1256,  1257,  1258,  1259,  1260,  1261,   331,
       0,     0,     0,     0,     0,     0,   220,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   331,     0,     0,     0,
       0,   220,   220,     0,  1273,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1735,     0,     0,     0,     0,     0,
     833,     0,     0,     0,     0,   236,     0,     0,     0,     0,
       0,   833,     0,     0,     0,     0,     0,   833,     0,     0,
       0,   833,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   836,     0,     0,     0,     0,   429,   430,   431,
       0,     0,     0,   331,     0,     0,   930,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   432,   236,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
       0,   455,     0,   833,     0,     0,     0,     0,  1350,     0,
       0,     0,    36,   456,   211,     0,     0,     0,     0,     0,
     220,   220,     0,     0,  1365,     0,  1366,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,   331,     0,     0,
       0,     0,  1376,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   331,     0,   836,     0,   236,     0,     0,   836,
     836,   836,   836,   836,   836,   836,   836,   836,   836,   836,
     836,   836,   836,   836,   836,   836,   836,   836,   836,   836,
     836,   836,   836,   836,   836,   836,   836,     0,     0,     0,
     753,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   836,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     5,     6,     7,     8,     9,     0,   788,     0,   116,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   969,     0,     0,   220,   735,    12,     0,     0,     0,
       0,     0,     0,   736,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1455,     0,    14,    15,  1457,     0,  1458,
       0,    16,  1459,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,   236,    28,    29,    30,    31,   220,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,   220,   220,    41,   836,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,   836,     0,   836,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   178,   179,   180,     0,
     836,    67,    68,     0,     0,     0,     0,     0,     0,  1538,
       0,   181,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,   220,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,   336,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
       0,     0,     0,   116,   117,     0,   118,   119,     0,     0,
       0,     0,     0,     0,  1682,  1683,     0,     0,     0,     0,
       0,  1688,     0,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
     429,   430,   431,     0,     0,     0,     0,     0,     0,     0,
       0,   836,   236,  1073,     0,   836,     0,   836,     0,   432,
     836,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,     0,   455,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   456,     0,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   236,     0,    11,    12,     0,   526,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   836,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,  1744,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,  1754,     0,    41,    42,    43,    44,  1759,    45,
       0,    46,  1761,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,   178,   179,    65,     0,    66,
      67,    68,   836,   836,     0,     0,     0,     0,     0,   836,
      72,    73,   457,    74,    75,    76,    77,    78,  1694,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,  1795,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     0,   429,   430,
     431,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   432,     0,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,     0,   455,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   456,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   836,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
     836,     0,     0,     0,    10,     0,   836,     0,     0,     0,
     836,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1777,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,   836,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,    60,    61,    62,    63,
      64,    65,   973,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,    81,    82,    83,    84,    85,     0,    86,
      87,     0,    88,    89,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,    95,     0,
      96,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1114,   116,   117,     0,   118,
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
       0,     0,    50,    51,    52,    53,    54,    55,    56,     0,
      57,    58,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,    69,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
      81,    82,    83,    84,    85,     0,    86,    87,     0,    88,
      89,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,    95,     0,    96,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,  1286,   116,   117,     0,   118,   119,     5,     6,
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
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,   663,   116,   117,     0,
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
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1087,   116,   117,     0,   118,   119,     5,
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
      79,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1128,   116,   117,     0,   118,   119,     5,     6,     7,     8,
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
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1194,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,  1196,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,     0,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,  1351,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
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
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,     0,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1462,   116,
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
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1685,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,  1730,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
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
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1765,
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
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,  1766,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,  1769,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,     0,   116,   117,     0,   118,   119,     5,
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
      79,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1786,   116,   117,     0,   118,   119,     5,     6,     7,     8,
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
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1842,   116,   117,
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
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1849,   116,   117,     0,   118,   119,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,   801,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,   178,   179,    65,     0,    66,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,  1014,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,   178,
     179,    65,     0,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,  1533,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,   178,   179,    65,     0,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,  1677,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   178,   179,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
     178,   179,    65,     0,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   178,   179,   180,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,   336,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   337,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,     0,
     455,   678,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   456,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   178,   179,   180,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
       0,   679,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,     0,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   178,   179,   180,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,   796,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,     0,     0,  1141,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1073,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   178,   179,
     180,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,  1142,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   735,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   178,   179,   180,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   429,
     430,   431,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   432,     0,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,   456,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,   194,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   178,   179,   180,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,  1080,  1081,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,     0,     0,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10, -1011, -1011, -1011, -1011,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,     0,     0,   224,     0,     0,     0,
       0,     0,     0,     0,     0,   456,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   178,
     179,   180,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   429,   430,   431,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   432,     0,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,     0,   455,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,   456,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   178,   179,   180,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   181,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,  1731,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     257,   430,   431,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   432,
       0,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,     0,   455,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   456,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   178,   179,   180,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   260,  1048,  1049,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1050,     0,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,  1073,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     178,   179,   180,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   181,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,   524,     0,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   691,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   178,   179,   180,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,     0,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
   -1011, -1011, -1011, -1011,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,     0,     0,     0,
       0,   736,     0,     0,     0,     0,     0,     0,     0,     0,
    1073,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   178,   179,   180,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   182,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,     0,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   777,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   178,   179,   180,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   779,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   178,   179,
     180,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1186,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   178,   179,   180,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   429,
     430,   431,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   432,     0,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,   456,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   178,   179,   180,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,  1541,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   429,   430,   431,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   432,     0,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,     0,
     455,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   456,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
     626,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   178,
     179,   180,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,    93,     0,  1378,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,     0,     0,     0,   116,   117,     0,   118,
     119,   262,   263,     0,   264,   265,     0,   431,   266,   267,
     268,   269,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   432,   270,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,     0,   455,
       0,     0,   272,     0,     0,     0,     0,     0,     0,     0,
       0,   456,     0,     0,     0,     0,   274,   275,   276,   277,
     278,   279,   280,     0,     0,     0,    36,     0,   211,     0,
       0,     0,     0,     0,     0,     0,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,    48,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
       0,     0,     0,     0,   723,   315,   316,   317,     0,     0,
       0,   318,   553,   554,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   262,   263,     0,   264,   265,     0,
     555,   266,   267,   268,   269,     0,    86,    87,     0,    88,
     183,    90,   323,     0,   324,     0,     0,   325,   270,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   272,     0,     0,     0,     0,
       0,   724,     0,   116,     0,     0,     0,     0,     0,   274,
     275,   276,   277,   278,   279,   280,     0,     0,     0,    36,
       0,   211,     0,     0,     0,     0,     0,     0,     0,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
      48,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,     0,     0,     0,     0,   314,   315,   316,
     317,     0,     0,     0,   318,   553,   554,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   262,   263,     0,
     264,   265,     0,   555,   266,   267,   268,   269,     0,    86,
      87,     0,    88,   183,    90,   323,     0,   324,     0,     0,
     325,   270,     0,   271,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   272,     0,
     273,     0,     0,     0,   724,     0,   116,     0,     0,     0,
       0,     0,   274,   275,   276,   277,   278,   279,   280,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,    48,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,     0,     0,     0,
       0,   315,   316,   317,    36,     0,     0,   318,   319,   320,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,   321,     0,     0,    84,
     322,     0,    86,    87,     0,    88,   183,    90,   323,     0,
     324,     0,     0,   325,     0,     0,     0,     0,     0,     0,
     326,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   262,   263,   327,   264,   265,     0,  1657,   266,
     267,   268,   269,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,     0,     0,     0,   270,     0,   271,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   272,     0,   273,   684,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   274,   275,   276,
     277,   278,   279,   280,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,    48,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,     0,     0,     0,     0,     0,   315,   316,   317,    36,
       0,     0,   318,   319,   320,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,   321,     0,     0,    84,   322,     0,    86,    87,     0,
      88,   183,    90,   323,     0,   324,     0,     0,   325,     0,
       0,     0,     0,     0,     0,   326,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   262,   263,   327,
     264,   265,     0,  1726,   266,   267,   268,   269,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,     0,     0,
       0,   270,     0,   271,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   272,     0,
     273,   944,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   274,   275,   276,   277,   278,   279,   280,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,    48,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,     0,     0,     0,
     314,   315,   316,   317,    36,     0,     0,   318,   319,   320,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,   321,     0,     0,    84,
     322,     0,    86,    87,     0,    88,   183,    90,   323,     0,
     324,     0,     0,   325,     0,     0,     0,     0,     0,     0,
     326,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   262,   263,   327,   264,   265,     0,     0,   266,
     267,   268,   269,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,     0,     0,     0,   270,     0,   271,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   272,     0,   273,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   274,   275,   276,
     277,   278,   279,   280,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,    48,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,     0,     0,     0,     0,     0,   315,   316,   317,     0,
       0,     0,   318,   319,   320,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   321,     0,     0,    84,   322,     0,    86,    87,     0,
      88,   183,    90,   323,     0,   324,     0,     0,   325,     0,
       0,     0,     0,     0,     0,   326,  1466,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   262,   263,   327,
     264,   265,     0,     0,   266,   267,   268,   269,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   270,   432,   271,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,     0,   455,   272,     0,
     273,     0,     0,     0,     0,     0,     0,     0,     0,   456,
       0,     0,   274,   275,   276,   277,   278,   279,   280,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,    48,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,     0,     0,     0,
       0,   315,   316,   317,     0,     0,     0,   318,   319,   320,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   321,     0,     0,    84,
     322,     0,    86,    87,     0,    88,   183,    90,   323,     0,
     324,     0,     0,   325,     0,     0,     0,     0,     0,     0,
     326,     0,     0,     0,   429,   430,   431,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   432,   327,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,     0,   455,   429,
     430,   431,     0,     0,     0,     0,     0,     0,     0,     0,
     456,     0,     0,     0,     0,     0,     0,     0,   432,     0,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,   429,   430,   431,     0,     0,     0,
       0,     0,     0,     0,     0,   456,     0,     0,     0,     0,
       0,     0,     0,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,     0,   455,   429,
     430,   431,     0,     0,     0,     0,     0,     0,     0,     0,
     456,     0,     0,     0,     0,     0,     0,     0,   432,     0,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,   429,   430,   431,     0,     0,     0,
       0,     0,     0,     0,     0,   456,     0,     0,  1098,     0,
       0,     0,     0,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,     0,   455,   429,
     430,   431,     0,     0,     0,     0,     0,     0,     0,     0,
     456,     0,     0,  1154,     0,     0,     0,     0,   432,     0,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   456,     0,     0,  1165,     0,
       0,     0,     0,     0,  1558,  1559,  1560,  1561,  1562,     0,
       0,  1563,  1564,  1565,  1566,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1567,  1568,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1188,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1569,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1570,
    1571,  1572,  1573,  1574,  1575,  1576,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,  1507,  1577,
    1578,  1579,  1580,  1581,  1582,  1583,  1584,  1585,  1586,  1587,
      48,  1588,  1589,  1590,  1591,  1592,  1593,  1594,  1595,  1596,
    1597,  1598,  1599,  1600,  1601,  1602,  1603,  1604,  1605,  1606,
    1607,  1608,  1609,  1610,  1611,  1612,  1613,  1614,  1615,  1616,
    1617,     0,     0,  1508,  1618,  1619,     0,  1620,  1621,  1622,
    1623,  1624,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1625,  1626,  1627,     0,     0,     0,    86,
      87,     0,    88,   183,    90,  1628,     0,  1629,  1630,     0,
    1631,     0,     0,     0,     0,     0,     0,  1632,  1633,     0,
    1634,     0,  1635,  1636,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   262,   263,
       0,   264,   265,     0,  1049,   266,   267,   268,   269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1050,   270,  1051,  1052,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,  1072,     0,     0,     0,     0,   272,
       0,     0,     0,     0,     0,     0,     0,     0,  1073,     0,
       0,     0,     0,   274,   275,   276,   277,   278,   279,   280,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,    48,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,     0,     0,     0,
       0,   314,   315,   316,   317,     0,     0,     0,   318,   553,
     554,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   262,   263,     0,   264,   265,     0,   555,   266,   267,
     268,   269,     0,    86,    87,     0,    88,   183,    90,   323,
       0,   324,     0,     0,   325,   270,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   272,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   274,   275,   276,   277,
     278,   279,   280,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,    48,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
       0,     0,     0,     0,  1229,   315,   316,   317,     0,     0,
       0,   318,   553,   554,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   262,   263,     0,   264,   265,     0,
     555,   266,   267,   268,   269,     0,    86,    87,     0,    88,
     183,    90,   323,     0,   324,     0,     0,   325,   270,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   272,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   274,
     275,   276,   277,   278,   279,   280,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
      48,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,     0,     0,     0,     0,     0,   315,   316,
     317,     0,     0,     0,   318,   553,   554,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   555,     0,     0,     0,     0,     0,    86,
      87,     0,    88,   183,    90,   323,     0,   324,     0,     0,
     325,     0,     0,     0,     0,     0,     0,     0,     0,   429,
     430,   431,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   432,  1355,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,   429,   430,   431,     0,     0,     0,
       0,     0,     0,     0,     0,   456,     0,     0,     0,     0,
       0,     0,     0,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,     0,   455,   429,
     430,   431,     0,     0,     0,     0,     0,     0,     0,     0,
     456,     0,     0,     0,     0,     0,     0,     0,   432,     0,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,   429,   430,   431,     0,     0,     0,
       0,     0,     0,     0,     0,   456,     0,     0,     0,     0,
       0,     0,     0,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,     0,   455,   429,
     430,   431,     0,     0,     0,     0,     0,     0,     0,     0,
     456,  1356,     0,     0,     0,     0,     0,     0,   432,     0,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,   429,   430,   431,     0,     0,     0,
       0,     0,     0,     0,     0,   456,   540,     0,     0,     0,
       0,     0,     0,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,     0,   455,   429,
     430,   431,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   542,     0,     0,     0,     0,     0,     0,   432,     0,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,     0,   455,   429,   430,   431,     0,     0,     0,
       0,     0,     0,     0,     0,   456,   560,     0,     0,     0,
       0,     0,   271,   432,     0,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,     0,   455,   273,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   584,     0,     0,     0,     0,     0,     0,   271,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,   273,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   769,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   543,   544,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,   182,     0,     0,    84,   322,
     793,    86,    87,     0,    88,   183,    90,     0,  1362,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   326,
       0,     0,     0,   543,   544,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
    1084,   182,     0,   327,    84,   322,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   326,     0,     0,     0,  1237,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   822,   823,     0,   327,
       0,     0,   824,     0,   825,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   826,     0,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,   429,   430,
     431,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   432,    48,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,     0,   455,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   827,   456,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,   182,    82,    83,    84,   828,     0,    86,    87,     0,
      88,   183,    90,     0,  1008,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,   829,     0,     0,     0,     0,
      97,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    28,     0,     0,   830,
     502,     0,     0,     0,    33,    34,    35,    36,     0,   211,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,  1474,     0,     0,     0,
       0,     0,  1009,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,     0,     0,     0,
      97,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    48,     0,     0,   215,
       0,     0,     0,     0,   116,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   822,   823,     0,     0,  1475,     0,
     824,     0,   825,     0,     0,     0,     0,     0,     0,     0,
       0,  1476,  1477,     0,   826,     0,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,     0,     0,     0,   182,
       0,     0,    84,  1478,   212,    86,    87,     0,    88,  1479,
      90,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,     0,     0,     0,
       0,   827,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,   182,
      82,    83,    84,   828,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,     0,     0,     0,
       0,     0,     0,   829,     0,     0,     0,     0,    97,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    28,     0,     0,   830,     0,     0,
       0,     0,    33,    34,    35,    36,     0,   211,     0,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   213,   455,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     456,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,    28,     0,   215,     0,     0,
     597,     0,   116,    33,    34,    35,    36,     0,   211,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   617,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,    28,   215,   964,
       0,     0,     0,   116,     0,    33,    34,    35,    36,     0,
     211,     0,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    97,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,    28,     0,
     215,     0,     0,     0,     0,   116,    33,    34,    35,    36,
       0,   211,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1109,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    97,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,    28,
       0,   215,     0,     0,     0,     0,   116,    33,    34,    35,
      36,     0,   211,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,     0,     0,     0,  1168,  1169,  1170,    36,
       0,     0,     0,     0,     0,     0,    73,     0,    74,    75,
      76,    77,    78,     0,     0,    36,     0,     0,     0,   214,
      48,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,    48,     0,     0,    92,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   215,    33,    34,    35,    36,   116,   211,    86,
      87,     0,    88,   183,    90,   212,     0,     0,     0,     0,
       0,     0,   378,     0,     0,    86,    87,    48,    88,   183,
      90,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   229,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    74,    75,    76,    77,    78,     0,
     379,    36,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,    48,     0,     0,    92,     0,     0,     0,     0,
     345,   346,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   230,    33,
      34,    35,    36,   116,   211,     0,     0,     0,     0,     0,
       0,   640,     0,     0,     0,     0,     0,     0,   347,     0,
       0,    86,    87,    48,    88,   183,    90,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   214,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   429,   430,   431,   641,     0,     0,     0,     0,   116,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     432,     0,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,     0,   455,   429,   430,   431,     0,
       0,     0,     0,     0,     0,     0,     0,   456,     0,     0,
       0,     0,     0,     0,     0,   432,     0,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,     0,
     455,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   456,   429,   430,   431,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   432,   511,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,     0,   455,   429,   430,
     431,     0,     0,     0,     0,     0,     0,     0,     0,   456,
       0,     0,     0,     0,     0,     0,     0,   432,   950,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,     0,   455,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   456,  1047,  1048,  1049,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1050,   994,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,     0,     0,
    1047,  1048,  1049,     0,     0,     0,     0,     0,     0,     0,
       0,  1073,     0,     0,     0,     0,     0,     0,     0,  1050,
    1315,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1073,  1047,  1048,  1049,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1050,  1219,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
       0,     0,  1047,  1048,  1049,     0,     0,     0,     0,     0,
       0,     0,     0,  1073,     0,     0,     0,     0,     0,     0,
       0,  1050,  1372,  1051,  1052,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,  1072,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1073,     0,
       0,     0,    36,   429,   430,   431,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1454,
       0,     0,   432,    48,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,     0,   455,  1488,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   456,
       0,     0,     0,  1642,  1540,    86,    87,  1643,    88,   183,
      90,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    86,    87,    36,    88,   183,    90,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    36,    48,     0,  1489,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,  1489,    48,     0,  1475,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1476,  1477,     0,     0,     0,    36,     0,   892,   893,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    36,
       0,    84,    85,     0,    86,    87,    48,    88,  1479,    90,
       0,     0,     0,     0,     0,     0,     0,     0,   182,     0,
      48,    84,    85,     0,    86,    87,     0,    88,   183,    90,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    36,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,    48,    86,    87,     0,    88,   183,
      90,     0,     0,     0,     0,     0,     0,   416,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    36,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    36,     0,
     585,     0,     0,    86,    87,    48,    88,   183,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   589,     0,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,     0,     0,   347,     0,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   429,   430,   431,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   805,   432,     0,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
       0,   455,   429,   430,   431,     0,     0,     0,     0,     0,
       0,     0,     0,   456,     0,     0,     0,     0,     0,     0,
       0,   432,   947,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   806,   455,  1047,  1048,  1049,
       0,     0,     0,     0,     0,     0,     0,     0,   456,     0,
       0,     0,     0,     0,     0,     0,  1050,  1377,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
    1047,  1048,  1049,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1073,     0,     0,     0,     0,     0,  1050,
       0,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1050,  1073,  1051,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1073,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,     0,   455,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   456,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1073,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,     0,   455,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   456
};

static const yytype_int16 yycheck[] =
{
       5,     6,   135,     8,     9,    10,    11,    12,    54,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    91,   161,    28,    29,    95,    96,   185,   386,   681,
     923,   526,    32,     4,     4,   732,   653,    42,     4,  1131,
       4,     4,   652,   227,    44,    50,   386,    52,   471,    49,
      55,   166,    57,   186,   492,   493,   386,    30,   113,   455,
     496,    30,   231,    55,   134,   161,   488,   804,    30,   771,
     912,    30,   113,  1118,    79,   633,   488,   113,   943,     9,
     811,   241,   520,    58,    42,     9,   581,     9,  1007,    30,
    1127,     9,    14,    30,   959,     9,    14,    54,     9,     9,
     522,     4,     9,    14,     9,     4,    81,     9,   113,    84,
     522,     9,    46,     9,     9,    46,     9,     9,     9,    33,
      46,    79,     9,     9,     9,     9,     9,   242,     9,     9,
      80,    80,     9,    86,     9,     9,     9,     9,     9,     9,
      80,    35,   130,   131,    67,   155,   111,    46,    80,    14,
       9,    80,   459,   151,     0,   102,   103,   200,    67,   654,
     215,   519,   200,    51,    80,    30,    67,    67,   200,   172,
     151,   172,    54,   118,   215,   230,     8,    98,    46,   215,
      46,   126,   489,    48,    66,   190,    80,   494,   200,   230,
      98,   172,   202,   203,     4,   148,  1671,   200,    67,   200,
      67,  1043,   200,    67,   169,    67,    67,   130,   131,   108,
     215,   371,   200,    67,   113,    67,   115,   116,   117,   118,
     119,   120,   121,    67,    35,   230,    67,    67,    67,   130,
     131,    67,    67,   165,   155,   151,   165,   116,   243,    67,
      67,   246,   151,    67,    67,    51,   200,   155,   253,   254,
     172,   151,  1727,   203,    67,   205,   205,   204,   161,   158,
     159,   202,   161,   421,   198,   205,   203,  1312,   156,    80,
     201,   165,   202,  1310,  1319,   201,  1321,   173,   202,   203,
     202,   247,  1201,   182,   202,   251,  1017,   201,  1019,   335,
       4,   202,   202,   173,   203,   202,   201,  1162,    80,   173,
     202,   796,   203,   203,   202,   204,   801,   202,   363,   202,
     202,   202,    26,    27,   173,   202,   202,   202,   202,   202,
     201,   505,   363,   940,   201,   135,   201,   363,   201,   201,
     201,   201,   198,   201,   203,   201,   203,   470,   151,  1181,
     498,   203,   203,   901,    80,   415,   200,    86,   200,   203,
     156,   203,   200,    98,   165,   200,   200,   362,   363,   203,
     200,   200,   203,   203,   369,   171,   412,   203,   203,   374,
     345,   346,   347,   200,   201,   203,   186,   200,   335,   203,
      80,   119,   203,  1420,   200,  1422,  1431,   200,   126,   156,
     203,   200,   172,   200,   399,   465,   466,   467,   468,   113,
      98,   102,   103,   378,   362,    35,   411,    80,   200,   148,
     155,   200,   148,   149,   150,   197,  1386,    35,    35,  1116,
     200,   203,   392,   200,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
      80,   456,   888,   458,   459,   412,   461,   155,    67,   643,
    1197,    30,    80,    80,     4,   165,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   471,
    1182,   155,   455,  1520,   489,   490,   455,   492,   493,   494,
     495,    98,   165,   455,   499,   398,   455,   502,    35,   659,
     660,   215,  1472,   204,   157,   462,   511,   929,   513,    80,
     224,    51,   200,    98,    54,   520,   230,   929,   956,  1014,
    1130,   130,   131,   528,    35,   530,  1496,  1369,  1498,  1146,
     669,    71,  1149,   247,    80,   165,   528,   251,   722,   202,
      86,   172,   151,    80,   130,   131,   519,   165,   155,    89,
     203,    91,   509,    29,   912,    95,    96,   726,   563,   151,
     202,   566,   567,   568,   115,   116,   117,   533,   964,   200,
     155,    47,   912,   669,    50,   151,   734,   202,   149,   150,
     172,    80,   912,   462,   172,   157,   641,    86,   398,   202,
     759,   200,   597,    67,   134,    80,   172,  1439,   170,    80,
     202,    86,    80,   149,   150,    86,   130,   131,    86,   488,
     585,   203,   200,   202,   589,   775,   776,   687,   204,   594,
      67,   781,   782,  1320,   200,   202,  1328,   203,  1330,   203,
     509,   203,    47,    48,   818,    29,   641,  1674,   352,     4,
     519,    72,    73,   522,    50,    51,    52,   361,    54,   363,
     149,   150,   200,    47,   368,   151,    50,   200,   198,   469,
      66,   375,   831,   209,   149,   150,   781,   148,   149,   150,
     839,   149,   150,   200,   679,   115,   116,   117,   118,   119,
     120,    46,   107,   653,   398,  1043,   691,    67,  1111,  1299,
     115,   116,   117,   118,   119,   120,   151,  1314,   155,    80,
      72,    73,    80,  1043,  1401,    86,    45,   247,    86,   208,
     202,   251,    66,  1043,   172,   255,   151,  1155,   200,   724,
     200,  1813,    47,    48,    49,    50,    51,    52,  1166,    54,
     735,    50,    51,    52,   207,  1437,  1828,   101,   102,   103,
       9,    66,   182,   108,   101,   102,   103,    66,   113,   754,
     115,   116,   117,   118,   119,   120,   121,   182,   117,   118,
     119,   200,   202,   151,  1200,   151,   669,   148,   149,   150,
    1807,   149,   150,   128,   129,  1270,  1473,    98,    99,   202,
     203,     8,   496,   788,   202,  1822,   115,   116,   117,   118,
     119,   120,   200,   158,   159,   335,   161,   126,   127,   804,
     800,   151,    75,    76,    77,   989,   115,   116,   117,   118,
     119,   120,   202,   203,    87,  1432,    14,   182,   151,   533,
     183,   184,   185,  1181,    80,  1527,   202,   190,   191,  1701,
    1702,   194,   195,   162,   202,   164,   841,  1697,  1698,   204,
     126,  1181,   126,   809,    14,  1281,  1341,    14,   177,  1033,
     179,  1181,   201,   182,   172,    98,  1040,   201,   398,   820,
     821,   201,  1357,   136,   137,   138,   139,   140,   408,   201,
     206,   107,   412,   182,   147,   415,   200,   200,  1316,     9,
     153,   154,   148,   200,    90,   860,   201,   201,  1781,   864,
     201,   794,   897,   963,   167,    47,    48,    49,    50,    51,
      52,   201,     9,   202,    14,   172,   911,  1800,   181,   200,
      26,    27,     9,   189,    66,  1808,   186,    80,    80,    80,
     200,   202,   462,   463,   464,   465,   466,   467,   468,   900,
     900,     9,   937,     9,   900,    80,   900,   900,   202,   201,
     201,   201,   947,   202,   128,   950,  1441,   952,   488,   200,
      67,   956,   855,   923,   201,  1450,    30,   129,   915,   171,
     151,   132,     9,   201,   678,   151,  1461,   198,  1137,   509,
     940,    14,     9,     9,  1676,   201,   173,     9,   128,    14,
    1418,   204,   522,   207,   794,   207,     9,    14,   207,   994,
     201,   964,  1176,   533,   201,   964,   200,   900,   207,   372,
    1000,   201,   964,   376,   151,   964,    98,   202,   202,     4,
      87,  1369,   552,   132,   151,     9,   201,   200,   921,   151,
     200,   203,   736,   151,    14,    80,   186,   186,   401,  1369,
     403,   404,   405,   406,     9,  1001,   915,   203,  1533,  1369,
      14,    14,  1226,   207,   202,   855,   203,   203,   927,  1233,
     929,    46,    14,   201,    30,   198,   596,    14,   202,   200,
     200,    30,   200,   777,   200,   779,    14,    49,     9,  1721,
     201,   200,  1077,  1078,  1079,   200,   202,   200,  1083,  1084,
     794,  1439,   622,   623,   202,  1042,   132,    14,  1045,     9,
     900,   132,   806,   201,    66,   809,   207,     9,     9,  1439,
    1003,    80,  1005,   132,   200,   200,  1111,    14,   224,  1439,
      80,   921,   202,   108,   201,   203,   995,   200,   113,  1111,
     115,   116,   117,   118,   119,   120,   121,   200,   203,   201,
     132,   115,   116,   117,   118,   119,   120,  1142,   203,   202,
     207,   855,   126,   127,  1115,  1115,  1798,   687,     9,  1115,
    1155,  1115,  1115,  1337,    87,   148,    30,    74,   202,   873,
     202,  1166,  1167,   158,   159,   173,   161,   201,   132,   132,
      30,  1666,     9,  1668,   888,   889,  1146,   201,   204,  1149,
     164,   201,  1677,   201,     9,   201,   900,   182,   203,  1189,
     204,    14,  1197,  1003,    80,  1005,   200,   202,   182,     9,
     201,   201,  1207,   201,   203,   201,   132,   921,   200,   204,
     201,    30,  1115,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,  1192,    54,   108,  1724,
     201,   771,   160,   773,   202,   201,   352,   202,     4,    66,
     201,   156,   202,    14,   203,   361,   202,    80,   113,   201,
     201,   132,   368,   203,   794,   201,   132,    14,   172,   375,
     203,    14,   202,  1402,  1397,    80,    26,    27,   808,   809,
     386,    14,    80,   203,   132,    14,   201,   200,   202,   201,
      46,   202,    14,   202,    14,   203,  1291,  1001,     9,  1003,
    1295,  1005,  1297,  1007,  1008,   204,    80,    56,   200,   172,
    1305,  1204,    80,     9,   202,  1115,    80,   111,    98,  1279,
    1315,  1316,   151,    98,   163,   855,    33,    14,  1288,   201,
     173,   861,   200,   202,   169,   865,   866,     9,   200,    80,
     166,  1826,   201,    80,   202,   201,  1494,   201,  1833,    14,
     203,    14,   108,    80,  1314,   885,    80,   113,    14,   115,
     116,   117,   118,   119,   120,   121,    80,    14,    80,   594,
     900,   864,   860,  1789,   468,   463,   465,   962,   903,  1804,
     957,  1532,  1198,  1378,  1354,   915,  1279,  1523,   599,  1800,
     496,   921,  1556,  1474,  1640,  1288,  1392,   927,  1832,   929,
    1820,  1652,   158,   159,  1204,   161,  1466,  1388,  1519,  1086,
     471,  1115,   570,  1083,   852,   570,  1119,  1164,  1041,   976,
    1415,  1177,   991,  1418,  1178,   927,   182,   820,  1756,  1102,
     369,  1381,  1026,   963,   412,  1074,    -1,  1141,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   975,   976,   977,   204,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1421,    -1,    -1,    -1,   995,    -1,  1427,    -1,  1429,
      -1,  1001,  1432,  1003,   224,  1005,    -1,    -1,    -1,    -1,
      -1,    -1,  1186,    -1,    -1,    -1,    -1,  1380,  1192,    -1,
      -1,  1447,    -1,    -1,    -1,  1025,  1200,  1201,    -1,    -1,
    1204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1656,  1402,
      -1,    -1,  1042,    -1,    -1,  1045,  1511,    -1,  1692,    -1,
      -1,    -1,  1469,    -1,    -1,    -1,    -1,    -1,  1421,    -1,
      -1,    -1,    -1,    -1,  1427,    -1,  1429,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1074,    -1,  1541,    -1,    -1,  1442,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1451,    -1,
      -1,  1091,    -1,    -1,  1094,  1525,    -1,    -1,    -1,    -1,
      -1,    -1,   678,    -1,     4,  1531,  1532,  1281,    26,    27,
    1380,    -1,    30,    26,    27,  1115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     4,    -1,  1396,    -1,    -1,    -1,
      -1,    -1,   352,    -1,    -1,    -1,    54,    -1,    -1,    -1,
      -1,   361,    -1,   363,    -1,  1651,    46,    -1,   368,    -1,
      -1,    -1,    -1,    -1,    -1,   375,    -1,    -1,    -1,    -1,
     736,    -1,  1525,    -1,    -1,    -1,    46,  1530,    -1,    -1,
      -1,    -1,  1442,  1536,    -1,    -1,  1775,    -1,  1178,  1542,
      -1,  1451,  1182,    -1,    -1,  1715,    -1,    -1,    -1,    -1,
      -1,    -1,  1192,    -1,    -1,    -1,  1661,    -1,    -1,    -1,
    1793,   777,    -1,   779,  1204,    -1,  1380,    -1,   108,    -1,
      -1,    -1,    -1,   113,    -1,   115,   116,   117,   118,   119,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,
     806,    -1,  1502,   113,  1651,   115,   116,   117,   118,   119,
     120,   121,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,   158,   159,
    1530,   161,    -1,    -1,    -1,    -1,  1536,    -1,  1442,    66,
      -1,    -1,  1542,  1447,  1274,    -1,   496,  1451,   158,   159,
      -1,   161,   182,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,   873,    -1,    -1,
      -1,    -1,   182,    -1,   204,    -1,   224,    -1,    -1,    -1,
      -1,   224,   888,   889,    -1,    -1,    -1,    -1,  1681,    -1,
      -1,    -1,    -1,    -1,   204,    -1,    -1,    -1,  1328,    -1,
    1330,    -1,    -1,    64,    65,    -1,   912,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1781,    -1,   271,    -1,   273,  1530,  1531,  1532,  1722,
    1723,    -1,  1536,    -1,    -1,    -1,  1729,    -1,  1542,    -1,
    1800,    -1,    -1,  1838,    -1,    -1,    -1,    -1,  1808,    -1,
    1380,  1846,    -1,    -1,    -1,    -1,  1386,  1852,    -1,    -1,
    1855,    -1,  1392,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,    -1,    -1,  1767,    -1,    -1,    -1,    -1,   327,
      -1,  1681,  1775,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,
      -1,  1007,  1008,    -1,   352,    -1,    -1,  1437,    -1,   352,
      -1,    -1,  1442,   361,    -1,    -1,    -1,  1447,   361,    -1,
     368,  1451,  1722,  1723,    -1,   368,    -1,   375,   678,  1729,
      -1,    -1,   375,    -1,    -1,    -1,  1466,  1043,   386,  1469,
     201,  1834,  1472,    -1,    -1,    -1,    -1,    -1,  1841,    -1,
      -1,    -1,  1482,    -1,    -1,    -1,    -1,    -1,    -1,  1489,
      -1,   409,    -1,    -1,   412,    -1,  1496,  1767,  1498,    -1,
      -1,    -1,    -1,    -1,  1504,    -1,    -1,  1681,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   736,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1527,    -1,    -1,
    1530,  1531,  1532,    -1,    -1,    -1,  1536,   455,    -1,    -1,
      -1,    -1,  1542,    -1,    -1,    -1,    -1,    -1,  1722,  1723,
      -1,    -1,    -1,    -1,    -1,  1729,    -1,   777,    -1,   779,
      -1,    -1,    -1,    -1,  1834,  1141,    -1,    -1,    -1,    -1,
      -1,  1841,    -1,    -1,    -1,    -1,    -1,    -1,   496,    -1,
      -1,    -1,    -1,   496,    -1,    -1,   806,    29,    -1,    -1,
      -1,    -1,    -1,  1767,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1181,    -1,    -1,    -1,    -1,
    1186,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1200,  1201,    -1,    -1,    -1,   547,
     548,    -1,    -1,   551,    -1,    -1,    78,    26,    27,    -1,
      -1,    30,    -1,    -1,   271,    -1,   273,    -1,    -1,    -1,
      -1,  1651,    -1,   873,    -1,    -1,    -1,    99,    -1,    -1,
    1834,    -1,    -1,    -1,    -1,   107,    -1,  1841,   888,   889,
      -1,    -1,  1672,    -1,    -1,    -1,  1676,    -1,    -1,    -1,
      -1,  1681,    -1,    -1,    -1,   603,    -1,    -1,    -1,    -1,
    1690,    -1,   134,   135,    -1,    -1,    -1,  1697,  1698,    -1,
     327,  1701,  1702,    -1,    -1,  1281,    -1,    -1,    -1,    -1,
     152,    -1,    -1,   155,   156,  1715,   158,   159,    -1,   161,
     162,   163,  1722,  1723,    -1,    -1,    -1,    -1,    -1,  1729,
      -1,   649,   650,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     658,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,    -1,   200,    -1,
     678,    -1,    -1,    -1,    -1,   678,    -1,  1767,    -1,    -1,
      -1,    -1,    -1,    -1,  1774,    10,    11,    12,    -1,    -1,
      -1,    -1,   409,    -1,    -1,   412,    -1,  1007,  1008,    -1,
      -1,    -1,  1792,  1369,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,   736,    54,
      -1,    -1,    -1,   736,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,  1834,   224,    -1,    -1,    -1,    -1,
      -1,  1841,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,   777,
      -1,   779,    -1,  1439,   777,    -1,   779,    -1,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,   806,   807,
      -1,    -1,    -1,   806,    -1,    64,    65,    -1,    -1,    -1,
      -1,    -1,   820,   821,   822,   823,   824,   825,   826,    -1,
      -1,    -1,   830,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     547,  1141,    64,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   856,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,   873,    -1,    -1,    -1,    -1,
     873,   130,   131,   352,    -1,    -1,  1186,    -1,   886,   204,
     888,   889,   361,    -1,    -1,   888,   889,    -1,    -1,   368,
    1200,  1201,    -1,    -1,   902,   903,   375,    -1,   130,   131,
      64,    65,    -1,    -1,   912,    -1,    -1,   386,    -1,    -1,
     918,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   931,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   939,   649,   650,   942,    -1,    -1,    -1,    -1,    -1,
      -1,   658,   201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   960,    29,    -1,    -1,   964,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   201,
      -1,  1281,    -1,    -1,    -1,    -1,   455,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1007,
    1008,    -1,    78,    -1,  1007,  1008,    -1,    -1,    -1,    -1,
      -1,    87,    -1,  1021,    -1,    -1,  1024,   496,  1026,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    64,    65,    -1,
      -1,    -1,    -1,  1041,    -1,  1043,    -1,    -1,  1046,  1047,
    1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,  1072,  1073,    -1,    -1,    -1,    -1,
      -1,    -1,   551,    -1,    -1,    -1,   152,    -1,    -1,   155,
      -1,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,  1099,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   820,   821,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,  1129,    -1,  1131,   603,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1141,    -1,    -1,    -1,    -1,  1141,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
    1158,    -1,    -1,  1161,    29,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,  1181,    54,    -1,    -1,    -1,  1186,    -1,
      -1,    56,    -1,  1186,    -1,   902,    66,    -1,    -1,    -1,
      -1,    -1,  1200,  1201,    -1,  1203,    -1,  1200,  1201,   678,
      -1,   918,    -1,    78,  1212,    -1,    -1,    -1,  1216,    -1,
      -1,  1219,    -1,  1221,   931,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,  1237,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   960,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   736,    -1,   134,
     135,    -1,    -1,    -1,    -1,    -1,    -1,  1275,  1276,    -1,
    1278,    -1,    -1,  1281,    -1,    -1,    -1,   152,  1281,    -1,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
     165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   777,    -1,
     779,   176,    -1,    -1,  1021,    -1,    -1,  1024,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,   200,    -1,   806,   807,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   822,   823,   824,   825,   826,    -1,    -1,
      -1,   830,    -1,  1361,    -1,  1363,    -1,    -1,    -1,   551,
    1368,  1369,    -1,    -1,  1372,   271,  1374,   273,    -1,  1377,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   856,  1386,  1387,
      -1,    -1,  1390,    -1,    -1,    -1,    -1,    -1,    -1,  1397,
      -1,    -1,    -1,    -1,   873,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   886,    -1,   888,
     889,   603,  1129,    -1,  1131,    -1,    -1,    -1,    -1,    -1,
      -1,   327,    -1,    -1,   903,    -1,    -1,    -1,    -1,    -1,
      -1,  1439,    -1,   912,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1158,    -1,    -1,  1161,    -1,  1454,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1464,  1465,    -1,    -1,
     939,    -1,    -1,   942,  1472,    -1,  1474,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,   964,    -1,    -1,  1496,    -1,
    1498,    -1,    -1,    -1,    -1,  1212,  1504,    -1,    66,  1216,
      -1,    -1,    -1,   409,    -1,    -1,   412,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,  1007,  1008,
      -1,  1539,  1540,    -1,    -1,    -1,    -1,    -1,  1546,    -1,
    1548,    66,    78,    -1,    -1,    -1,  1554,    -1,  1556,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1275,  1276,
      -1,    -1,  1041,    99,  1043,    -1,    -1,  1046,  1047,  1048,
    1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,   807,   152,    -1,    -1,   155,
    1099,    -1,   158,   159,    -1,   161,   162,   163,    66,    -1,
     822,   823,   824,   825,   826,    -1,    -1,    -1,   830,    -1,
      -1,   547,   548,    -1,  1361,   551,  1363,  1655,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,  1141,    -1,  1672,    -1,    -1,    -1,   204,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1397,    -1,  1690,    -1,    -1,    -1,    -1,    -1,  1696,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   603,    -1,  1707,
      -1,    -1,  1181,    -1,    -1,  1713,    -1,  1186,    -1,  1717,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1200,  1201,    -1,  1203,    -1,    -1,    -1,    -1,    -1,
      -1,  1739,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1219,    -1,  1221,   649,   650,    -1,    -1,   939,    -1,    -1,
      -1,    -1,   658,    -1,    -1,    -1,    -1,    -1,  1237,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1779,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1788,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1804,    -1,    -1,  1278,
      -1,    -1,  1281,    -1,    -1,  1813,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
    1828,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1554,    -1,    -1,
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1041,
      -1,    -1,    -1,    -1,  1046,  1047,  1048,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1368,
    1369,    -1,    -1,  1372,    -1,  1374,    -1,   156,  1377,   158,
     159,   807,   161,   162,   163,    -1,    -1,  1099,  1387,    26,
      27,  1390,    -1,    30,   820,   821,   822,   823,   824,   825,
     826,    -1,    -1,    -1,   830,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,  1655,    -1,
      -1,    47,    48,    -1,   203,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1439,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    -1,    -1,  1454,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,  1464,  1465,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,  1474,   902,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1203,   918,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1739,    -1,    -1,   931,    -1,  1219,   134,  1221,
      -1,    -1,    -1,   939,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,  1237,    -1,    -1,    -1,    -1,
      -1,    -1,   158,   159,   960,   161,   162,   163,    -1,    -1,
    1539,  1540,    -1,    -1,    -1,    -1,    -1,  1546,    -1,  1548,
     176,    -1,    -1,    -1,    -1,    -1,    -1,  1556,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,    -1,    -1,  1813,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   224,    -1,    -1,
      -1,  1828,    -1,    -1,    -1,  1021,    -1,    -1,  1024,    -1,
    1026,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1041,    -1,    -1,    -1,    -1,
    1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,  1368,    -1,    -1,    -1,
    1372,    -1,  1374,    -1,    -1,  1377,    -1,    -1,    -1,    -1,
      -1,    -1,    29,  1099,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,  1696,    -1,    -1,
      -1,    -1,    -1,  1129,    -1,  1131,   551,    -1,  1707,    66,
      -1,    -1,    -1,    -1,  1713,   352,    -1,    -1,  1717,    -1,
      -1,    -1,    -1,    -1,   361,    -1,    -1,    -1,    -1,    -1,
      -1,   368,  1158,    -1,    -1,  1161,    -1,    -1,   375,    -1,
      -1,    -1,  1454,    -1,    -1,    -1,    -1,    -1,    -1,   386,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   603,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1203,    -1,    -1,
    1779,    -1,    -1,    -1,    -1,    -1,  1212,    -1,    -1,  1788,
    1216,    -1,    -1,  1219,    -1,  1221,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1804,    -1,    -1,    -1,    -1,
      -1,  1237,    -1,    -1,    -1,    -1,    -1,    -1,   455,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1539,  1540,    -1,
      -1,    -1,    -1,    -1,  1546,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,   201,    -1,    -1,    -1,    -1,  1275,
    1276,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   496,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,   551,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1361,    -1,  1363,    -1,    -1,
      -1,    -1,  1368,    -1,    -1,    -1,  1372,    -1,  1374,    -1,
      -1,  1377,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
    1386,    -1,   807,    -1,    -1,    -1,   603,    -1,    -1,    -1,
      -1,  1397,    -1,    -1,    -1,    78,    -1,   822,   823,   824,
     825,    -1,    -1,    -1,  1696,   830,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1707,    99,    -1,    -1,    -1,
      -1,  1713,    -1,    -1,   107,  1717,    -1,    -1,    -1,    -1,
      -1,    -1,   115,   116,   117,   118,   119,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1454,    -1,
      -1,   134,   135,    -1,    -1,    -1,   207,    -1,    -1,    -1,
      -1,   678,    -1,    -1,    -1,    -1,  1472,    -1,    -1,   152,
      -1,    -1,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,    -1,    -1,    -1,  1779,    -1,    -1,
    1496,    -1,  1498,   176,    -1,    -1,    -1,    -1,  1504,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   939,    -1,    -1,   200,    -1,   736,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1539,  1540,    -1,    -1,    -1,    -1,    -1,
    1546,    -1,    10,    11,    12,    -1,    -1,    -1,  1554,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     777,    29,   779,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,   806,
     807,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,   822,   823,   824,   825,   826,
      -1,    -1,    -1,   830,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1655,
      -1,    -1,    -1,    -1,    -1,    -1,   873,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1672,    -1,    -1,    -1,
      -1,   888,   889,    -1,  1099,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1690,    -1,    -1,    -1,    -1,    -1,
    1696,    -1,    -1,    -1,    -1,   912,    -1,    -1,    -1,    -1,
      -1,  1707,    -1,    -1,    -1,    -1,    -1,  1713,    -1,    -1,
      -1,  1717,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   939,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,  1739,    -1,    -1,   204,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   964,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,  1779,    -1,    -1,    -1,    -1,  1203,    -1,
      -1,    -1,    78,    66,    80,    -1,    -1,    -1,    -1,    -1,
    1007,  1008,    -1,    -1,  1219,    -1,  1221,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,  1813,    -1,    -1,
      -1,    -1,  1237,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1828,    -1,  1041,    -1,  1043,    -1,    -1,  1046,
    1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,  1073,    -1,    -1,    -1,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1099,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,     3,     4,     5,     6,     7,    -1,   203,    -1,   205,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   204,    -1,    -1,  1141,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1368,    -1,    47,    48,  1372,    -1,  1374,
      -1,    53,  1377,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,  1181,    67,    68,    69,    70,  1186,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    -1,
      -1,    -1,    -1,  1200,  1201,    87,  1203,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,  1219,    -1,  1221,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,   116,   117,   118,   119,   120,    -1,
    1237,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,  1454,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,  1281,   167,    -1,    -1,   170,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   205,   206,    -1,   208,   209,    -1,    -1,
      -1,    -1,    -1,    -1,  1539,  1540,    -1,    -1,    -1,    -1,
      -1,  1546,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1368,  1369,    66,    -1,  1372,    -1,  1374,    -1,    29,
    1377,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1439,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1454,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    -1,
      -1,  1696,    75,    76,    77,    78,    79,    80,    -1,    82,
      83,    -1,  1707,    -1,    87,    88,    89,    90,  1713,    92,
      -1,    94,  1717,    96,    -1,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,    -1,   112,
      -1,   114,   115,   116,   117,   118,   119,   120,    -1,   122,
     123,   124,  1539,  1540,    -1,    -1,    -1,    -1,    -1,  1546,
     133,   134,   202,   136,   137,   138,   139,   140,  1555,    -1,
      -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,  1779,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,   202,
     203,    -1,   205,   206,    -1,   208,   209,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1696,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
    1707,    -1,    -1,    -1,    13,    -1,  1713,    -1,    -1,    -1,
    1717,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1741,    -1,    -1,    46,    47,    48,
      -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    71,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,  1779,    82,    83,    -1,    -1,    -1,    87,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,
      99,   100,    -1,    -1,    -1,   104,   105,   106,   107,   108,
     109,   110,    -1,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   204,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,
      -1,   170,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,
     179,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
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
      -1,    -1,   104,   105,   106,   107,   108,   109,   110,    -1,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,   123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,
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
      90,    91,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
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
     200,    -1,   202,   203,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,    82,
      83,    -1,    -1,    -1,    87,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    96,    97,    -1,    99,   100,    -1,    -1,
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
     196,   197,    -1,    -1,   200,    -1,   202,   203,   204,   205,
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
      92,    -1,    94,    95,    96,    -1,    -1,    99,   100,    -1,
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
     202,   203,    -1,   205,   206,    -1,   208,   209,     3,     4,
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
     105,   106,   107,    -1,   109,   110,    -1,   112,    -1,   114,
     115,   116,   117,   118,   119,   120,    -1,   122,   123,   124,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,   144,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,   164,
      -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
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
      -1,    92,    93,    94,    -1,    96,    -1,    -1,    99,   100,
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
      -1,   202,   203,    -1,   205,   206,    -1,   208,   209,     3,
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
     203,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    71,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    80,    -1,    82,    83,    -1,    -1,
      -1,    87,    88,    89,    90,    -1,    92,    -1,    94,    -1,
      96,    -1,    -1,    99,   100,    -1,    -1,    -1,   104,   105,
     106,   107,    -1,   109,   110,    -1,   112,    -1,   114,   115,
     116,   117,   118,   119,   120,    -1,   122,   123,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
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
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    -1,
      -1,    -1,    53,    -1,    55,    56,    57,    58,    59,    60,
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
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,   200,
      -1,   202,    -1,    -1,   205,   206,    -1,   208,   209,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    47,    48,    -1,    -1,    -1,    -1,    53,
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
      -1,   165,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,    -1,
      -1,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     197,    -1,    -1,   200,    -1,    -1,   203,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    47,    48,    -1,
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
      -1,   161,   162,   163,    -1,   165,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,
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
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,    10,
      11,    12,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    48,    -1,    -1,    66,    -1,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,   104,    -1,
      -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
     116,   117,   118,   119,   120,    -1,    -1,   123,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,
     176,   192,   193,    -1,    -1,   181,   182,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    47,    48,
      -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,
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
      -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,
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
      -1,    -1,   189,    -1,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
     202,    11,    12,   205,   206,    -1,   208,   209,     3,     4,
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
      -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,   202,    11,    12,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
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
      -1,    -1,   200,   201,    -1,    -1,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    -1,
      -1,    -1,    53,    -1,    55,    56,    57,    58,    59,    60,
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
      -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,    53,
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
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
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
     200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,
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
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,    10,
      11,    12,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    48,    -1,    -1,    66,    -1,    53,    -1,    55,
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
      -1,   167,    -1,    -1,   170,    -1,    -1,   188,    -1,    -1,
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
      -1,   170,    -1,   187,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,
     209,     3,     4,    -1,     6,     7,    -1,    12,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    27,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
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
      56,    -1,    -1,    -1,   203,    -1,   205,    -1,    -1,    -1,
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
      -1,   127,   128,   129,    -1,    -1,    -1,   133,   134,   135,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    10,    11,    12,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    29,   200,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,   204,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,   204,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,   204,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      69,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   204,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,   204,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   152,   153,   154,    -1,    -1,    -1,   158,
     159,    -1,   161,   162,   163,   164,    -1,   166,   167,    -1,
     169,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,
     179,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,     3,     4,
      -1,     6,     7,    -1,    12,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    27,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    68,    69,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
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
     195,   196,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      69,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,    -1,    -1,    -1,    -1,    -1,   127,   128,
     129,    -1,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,    -1,   158,
     159,    -1,   161,   162,   163,   164,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,   202,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,   202,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,   202,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,   202,    -1,    -1,    -1,
      -1,    -1,    29,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,   202,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,
     201,   158,   159,    -1,   161,   162,   163,    -1,   165,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,   134,   135,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     196,   152,    -1,   200,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    30,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    47,    48,    -1,   200,
      -1,    -1,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    99,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,    66,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    35,    -1,   167,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,
     181,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    67,    -1,    -1,   200,
     132,    -1,    -1,    -1,    75,    76,    77,    78,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,
     181,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    99,    -1,    -1,   200,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    -1,    -1,   121,    -1,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,   135,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,   152,
      -1,    -1,   155,   156,    87,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    67,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,   120,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    67,    -1,   200,    -1,    -1,
     203,    -1,   205,    75,    76,    77,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,    67,   200,    69,
      -1,    -1,    -1,   205,    -1,    75,    76,    77,    78,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    67,    -1,
     200,    -1,    -1,    -1,    -1,   205,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    67,
      -1,   200,    -1,    -1,    -1,    -1,   205,    75,    76,    77,
      78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,    -1,    -1,    -1,    75,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    78,    -1,    -1,    -1,   147,
      99,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    99,    -1,    -1,   167,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,   200,    75,    76,    77,    78,   205,    80,   158,
     159,    -1,   161,   162,   163,    87,    -1,    -1,    -1,    -1,
      -1,    -1,   155,    -1,    -1,   158,   159,    99,   161,   162,
     163,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   120,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   136,   137,   138,   139,   140,    -1,
     203,    78,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    99,    -1,    -1,   167,    -1,    -1,    -1,    -1,
     107,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,    -1,   200,    75,
      76,    77,    78,   205,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,   155,    -1,
      -1,   158,   159,    99,   161,   162,   163,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    10,    11,    12,   200,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   132,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   132,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   132,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     132,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   132,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   132,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    78,    10,    11,    12,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,
      -1,    -1,    29,    99,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,   124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,   156,   132,   158,   159,   160,   161,   162,
     163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   158,   159,    78,   161,   162,   163,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    78,    99,    -1,   200,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,   200,    99,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,    -1,    -1,    78,    -1,    80,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,    78,
      -1,   155,   156,    -1,   158,   159,    99,   161,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,
      99,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    78,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    99,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    78,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    78,    -1,
     155,    -1,    -1,   158,   159,    99,   161,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   155,    -1,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,    -1,    -1,   155,    -1,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    98,    54,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    66,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66
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
     368,   370,   371,   373,   392,   402,   403,   404,   406,   411,
     414,   433,   441,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   468,   470,   472,   118,   119,
     120,   133,   152,   162,   217,   250,   329,   348,   445,   348,
     200,   348,   348,   348,   104,   348,   348,   431,   432,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,    80,    87,   120,   147,   200,   227,   367,   403,   406,
     411,   445,   448,   445,    35,   348,   459,   460,   348,   120,
     200,   227,   403,   404,   405,   407,   411,   442,   443,   444,
     452,   456,   457,   200,   339,   408,   200,   339,   355,   340,
     348,   236,   339,   200,   200,   200,   339,   202,   348,   217,
     202,   348,     3,     4,     6,     7,    10,    11,    12,    13,
      27,    29,    54,    56,    68,    69,    70,    71,    72,    73,
      74,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   126,   127,   128,   129,   133,   134,
     135,   152,   156,   164,   166,   169,   176,   200,   217,   218,
     219,   230,   473,   488,   489,   491,   183,   202,   345,   348,
     372,   374,   203,   243,   348,   107,   108,   155,   220,   223,
     226,    80,   205,   295,   296,   119,   126,   118,   126,    80,
     297,   200,   200,   200,   200,   217,   267,   476,   200,   200,
     340,    80,    86,   148,   149,   150,   465,   466,   155,   203,
     226,   226,   217,   268,   476,   156,   200,   476,   476,    80,
     197,   203,   357,   338,   348,   349,   445,   449,   232,   203,
     454,    86,   409,   465,    86,   465,   465,    30,   155,   172,
     477,   200,     9,   202,    35,   249,   156,   266,   476,   120,
     182,   250,   330,   202,   202,   202,   202,   202,   202,    10,
      11,    12,    29,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    54,    66,   202,    67,    67,
     202,   203,   151,   127,   162,   164,   177,   179,   269,   328,
     329,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    64,    65,   130,   131,   435,    67,
     203,   440,   200,   200,    67,   203,   200,   249,   250,    14,
     348,   202,   132,    45,   217,   430,    86,   338,   349,   151,
     445,   132,   207,     9,   416,   338,   349,   445,   477,   151,
     200,   410,   435,   440,   201,   348,    30,   234,     8,   360,
       9,   202,   234,   235,   340,   341,   348,   217,   281,   238,
     202,   202,   202,   134,   135,   491,   491,   172,   200,   107,
     491,    14,   151,   134,   135,   152,   217,   219,    80,   202,
     202,   202,   183,   184,   185,   190,   191,   194,   195,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   387,   388,
     389,   244,   111,   169,   202,   155,   221,   224,   226,   155,
     222,   225,   226,   226,     9,   202,    98,   203,   445,     9,
     202,   126,   126,    14,     9,   202,   445,   469,   340,   338,
     349,   445,   448,   449,   201,   172,   261,   133,   445,   458,
     459,   202,    67,   435,   148,   466,    79,   348,   445,    86,
     148,   466,   226,   216,   202,   203,   256,   264,   393,   395,
      87,   200,   361,   362,   364,   406,   451,   453,   470,    14,
      98,   471,   356,   358,   359,   291,   292,   433,   434,   201,
     201,   201,   201,   204,   233,   234,   251,   258,   263,   433,
     348,   206,   208,   209,   217,   478,   479,   491,    35,   165,
     293,   294,   348,   473,   200,   476,   259,   249,   348,   348,
     348,    30,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   407,   348,   348,   455,   455,
     348,   461,   462,   126,   203,   218,   219,   454,   267,   217,
     268,   476,   476,   266,   250,    27,    35,   342,   345,   348,
     372,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   156,   203,   217,   436,   437,   438,   439,
     454,   455,   348,   293,   293,   455,   348,   458,   249,   201,
     348,   200,   429,     9,   416,   201,   201,    35,   348,    35,
     348,   201,   201,   201,   452,   453,   454,   293,   203,   217,
     436,   437,   454,   201,   232,   285,   203,   345,   348,   348,
      90,    30,   234,   279,   202,    28,    98,    14,     9,   201,
      30,   203,   282,   491,    29,    87,   230,   485,   486,   487,
     200,     9,    47,    48,    53,    55,    67,   134,   156,   176,
     200,   227,   228,   230,   369,   403,   411,   412,   413,   217,
     490,   186,    80,   348,    80,    80,   348,   384,   385,   348,
     348,   377,   387,   189,   390,   232,   200,   242,   226,   202,
       9,    98,   226,   202,     9,    98,    98,   223,   217,   348,
     296,   412,    80,     9,   201,   201,   201,   201,   201,   201,
     201,   202,    47,    48,   483,   484,   128,   272,   200,     9,
     201,   201,    80,    81,   217,   467,   217,    67,   204,   204,
     213,   215,    30,   129,   271,   171,    51,   156,   171,   397,
     349,   132,     9,   416,   201,   151,   491,   491,    14,   360,
     291,   232,   198,     9,   417,   491,   492,   435,   440,   435,
     204,     9,   416,   173,   445,   348,   201,     9,   417,    14,
     352,   252,   128,   270,   200,   476,   348,    30,   207,   207,
     132,   204,     9,   416,   348,   477,   200,   262,   257,   265,
      14,   471,   260,   249,    69,   445,   348,   477,   207,   204,
     201,   201,   207,   204,   201,    47,    48,    67,    75,    76,
      77,    87,   134,   147,   176,   217,   419,   421,   422,   425,
     428,   217,   445,   445,   132,   435,   440,   201,   348,   286,
      72,    73,   287,   232,   339,   232,   341,    98,    35,   133,
     276,   445,   412,   217,    30,   234,   280,   202,   283,   202,
     283,     9,   173,    87,   132,   151,     9,   416,   201,   165,
     478,   479,   480,   478,   412,   412,   412,   412,   412,   415,
     418,   200,   151,   200,   412,   151,   203,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    66,   151,   477,   348,   186,   186,    14,
     192,   193,   386,     9,   196,   390,    80,   204,   403,   203,
     246,    98,   224,   217,    98,   225,   217,   217,   204,    14,
     445,   202,     9,   173,   217,   273,   403,   203,   458,   133,
     445,    14,   207,   348,   204,   213,   491,   273,   203,   396,
      14,   201,   348,   361,   454,   202,   491,   198,   204,    30,
     481,   434,    35,    80,   165,   436,   437,   439,   436,   437,
     491,    35,   165,   348,   412,   291,   200,   403,   271,   353,
     253,   348,   348,   348,   204,   200,   293,   272,    30,   271,
     491,    14,   270,   476,   407,   204,   200,    14,    75,    76,
      77,   217,   420,   420,   422,   423,   424,    49,   200,    86,
     148,   200,     9,   416,   201,   429,    35,   348,   204,    72,
      73,   288,   339,   234,   204,   202,    91,   202,   276,   445,
     200,   132,   275,    14,   232,   283,   101,   102,   103,   283,
     204,   491,   132,   491,   217,   485,     9,   201,   416,   132,
     207,     9,   416,   415,   218,   361,   363,   365,   201,   126,
     218,   412,   463,   464,   412,   412,   412,    30,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   490,   348,   348,   348,   385,   348,   375,    80,
     247,   217,   217,   412,   484,    98,    99,   482,     9,   301,
     201,   200,   342,   345,   348,   207,   204,   471,   301,   157,
     170,   203,   392,   399,   157,   203,   398,   132,   202,   481,
     491,   360,   492,    80,   165,    14,    80,   477,   445,   348,
     201,   291,   203,   291,   200,   132,   200,   293,   201,   203,
     491,   203,   202,   491,   271,   254,   410,   293,   132,   207,
       9,   416,   421,   423,   148,   361,   426,   427,   422,   445,
     339,    30,    74,   234,   202,   341,   275,   458,   276,   201,
     412,    97,   101,   202,   348,    30,   202,   284,   204,   173,
     491,   132,   165,    30,   201,   412,   412,   201,   132,     9,
     416,   201,   132,   204,     9,   416,   412,    30,   187,   201,
     232,   217,   491,   491,   403,     4,   108,   113,   119,   121,
     158,   159,   161,   204,   302,   327,   328,   329,   334,   335,
     336,   337,   433,   458,   204,   203,   204,    51,   348,   348,
     348,   360,    35,    80,   165,    14,    80,   348,   200,   481,
     201,   301,   201,   291,   348,   293,   201,   301,   471,   301,
     202,   203,   200,   201,   422,   422,   201,   132,   201,     9,
     416,    30,   232,   202,   201,   201,   201,   239,   202,   202,
     284,   232,   491,   491,   132,   412,   361,   412,   412,   412,
     348,   203,   204,   482,   128,   129,   177,   218,   474,   491,
     274,   403,   108,   337,    29,   121,   134,   135,   156,   162,
     311,   312,   313,   314,   403,   160,   319,   320,   124,   200,
     217,   321,   322,   303,   250,   491,     9,   202,     9,   202,
     202,   471,   328,   201,   298,   156,   394,   204,   204,    80,
     165,    14,    80,   348,   293,   113,   350,   481,   204,   481,
     201,   201,   204,   203,   204,   301,   291,   132,   422,   361,
     232,   237,   240,    30,   234,   278,   232,   201,   412,   132,
     132,   188,   232,   403,   403,   476,    14,   218,     9,   202,
     203,   474,   471,   314,   172,   203,     9,   202,     3,     4,
       5,     6,     7,    10,    11,    12,    13,    27,    28,    54,
      68,    69,    70,    71,    72,    73,    74,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   133,   134,
     136,   137,   138,   139,   140,   152,   153,   154,   164,   166,
     167,   169,   176,   177,   179,   181,   182,   217,   400,   401,
       9,   202,   156,   160,   217,   322,   323,   324,   202,    80,
     333,   249,   304,   474,   474,    14,   250,   204,   299,   300,
     474,    14,    80,   348,   201,   200,   203,   202,   203,   325,
     350,   481,   298,   204,   201,   422,   132,    30,   234,   277,
     278,   232,   412,   412,   348,   204,   202,   202,   412,   403,
     307,   491,   315,   316,   411,   312,    14,    30,    48,   317,
     320,     9,    33,   201,    29,    47,    50,    14,     9,   202,
     219,   475,   333,    14,   491,   249,   202,    14,   348,    35,
      80,   391,   232,   232,   203,   325,   204,   481,   422,   232,
      95,   189,   245,   204,   217,   230,   308,   309,   310,     9,
     173,     9,   416,   204,   412,   401,   401,    56,   318,   323,
     323,    29,    47,    50,   412,    80,   172,   200,   202,   412,
     476,   412,    80,     9,   417,   204,   204,   232,   325,    93,
     202,    80,   111,   241,   151,    98,   491,   411,   163,    14,
     483,   305,   200,    35,    80,   201,   204,   202,   200,   169,
     248,   217,   328,   329,   173,   412,   173,   289,   290,   434,
     306,    80,   403,   246,   166,   217,   202,   201,     9,   417,
     115,   116,   117,   331,   332,   289,    80,   274,   202,   481,
     434,   492,   201,   201,   202,   202,   203,   326,   331,    35,
      80,   165,   481,   203,   232,   492,    80,   165,    14,    80,
     326,   232,   204,    35,    80,   165,    14,    80,   348,   204,
      80,   165,    14,    80,   348,    14,    80,   348,   348
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
#line 2446 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { (yyval).reset();;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval).reset();;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { (yyval).reset();;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval).reset();;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { (yyval).reset();;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval).reset();;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval).reset();;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { (yyval).reset();;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { (yyval).reset();;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { (yyval).reset();;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2726 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { (yyval).reset();;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
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

  case 852:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
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

  case 853:

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

  case 854:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
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

  case 855:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
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

  case 862:

/* Line 1455 of yacc.c  */
#line 2874 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
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

  case 871:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyval).reset();;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
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

  case 905:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
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

  case 911:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyval).reset();;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3079 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3093 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3096 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3112 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3114 "hphp.y"
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

  case 947:

/* Line 1455 of yacc.c  */
#line 3124 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3136 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3142 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3150 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3172 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3177 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3204 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3208 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3226 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3230 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3247 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3252 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3262 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3264 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3268 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3275 "hphp.y"
    {;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3276 "hphp.y"
    {;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3277 "hphp.y"
    {;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3283 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3288 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3299 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3305 "hphp.y"
    { ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3310 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3311 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3317 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3327 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3331 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3338 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3341 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3345 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3348 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3354 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3358 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3361 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3364 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3370 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3376 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3385 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14496 "hphp.7.tab.cpp"
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

