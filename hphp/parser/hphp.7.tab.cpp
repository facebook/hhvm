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
     T_LAMBDA_OP = 419,
     T_LAMBDA_CP = 420,
     T_UNRESOLVED_OP = 421
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
#line 872 "hphp.7.tab.cpp"

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
#define YYLAST   16655

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  196
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  264
/* YYNRULES -- Number of rules.  */
#define YYNRULES  981
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1786

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   421

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    53,   194,     2,   191,    52,    35,   195,
     186,   187,    50,    47,     9,    48,    49,    51,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   188,
      40,    14,    41,    29,    56,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    67,     2,   193,    34,     2,   192,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   189,    33,   190,    55,     2,     2,     2,
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
     184,   185
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
     261,   266,   268,   272,   274,   278,   281,   283,   286,   289,
     295,   300,   303,   304,   306,   308,   310,   312,   316,   322,
     331,   332,   337,   338,   345,   346,   357,   358,   363,   366,
     370,   373,   377,   380,   384,   388,   392,   396,   400,   404,
     410,   412,   414,   416,   417,   427,   428,   439,   445,   446,
     460,   461,   467,   471,   475,   478,   481,   484,   487,   490,
     493,   497,   500,   503,   504,   509,   519,   520,   521,   526,
     529,   530,   532,   533,   535,   536,   546,   547,   558,   559,
     571,   572,   582,   583,   594,   595,   604,   605,   615,   616,
     624,   625,   634,   635,   643,   644,   653,   655,   657,   659,
     661,   663,   666,   670,   674,   677,   680,   681,   684,   685,
     688,   689,   691,   695,   697,   701,   704,   705,   707,   710,
     715,   717,   722,   724,   729,   731,   736,   738,   743,   747,
     753,   757,   762,   767,   773,   779,   784,   785,   787,   789,
     794,   795,   801,   802,   805,   806,   810,   811,   819,   828,
     835,   838,   844,   851,   856,   857,   862,   868,   876,   883,
     890,   898,   908,   917,   924,   932,   938,   941,   946,   952,
     956,   957,   961,   966,   973,   979,   985,   992,  1001,  1009,
    1012,  1013,  1015,  1018,  1021,  1025,  1030,  1035,  1039,  1041,
    1043,  1046,  1051,  1055,  1061,  1063,  1067,  1070,  1071,  1074,
    1078,  1081,  1082,  1083,  1088,  1089,  1095,  1098,  1101,  1104,
    1105,  1116,  1117,  1129,  1133,  1137,  1141,  1146,  1151,  1155,
    1161,  1164,  1167,  1168,  1175,  1181,  1186,  1190,  1192,  1194,
    1198,  1203,  1205,  1208,  1210,  1212,  1217,  1224,  1226,  1228,
    1233,  1235,  1237,  1241,  1244,  1247,  1248,  1251,  1252,  1254,
    1258,  1260,  1262,  1264,  1266,  1270,  1275,  1280,  1285,  1287,
    1289,  1292,  1295,  1298,  1302,  1306,  1308,  1310,  1312,  1314,
    1318,  1320,  1324,  1326,  1328,  1330,  1331,  1333,  1336,  1338,
    1340,  1342,  1344,  1346,  1348,  1350,  1352,  1353,  1355,  1357,
    1359,  1363,  1369,  1371,  1375,  1381,  1386,  1390,  1394,  1398,
    1403,  1407,  1411,  1415,  1418,  1420,  1422,  1426,  1430,  1432,
    1434,  1435,  1437,  1440,  1445,  1449,  1453,  1460,  1463,  1467,
    1474,  1476,  1478,  1480,  1482,  1484,  1491,  1495,  1500,  1507,
    1511,  1515,  1519,  1523,  1527,  1531,  1535,  1539,  1543,  1547,
    1551,  1555,  1558,  1561,  1564,  1567,  1571,  1575,  1579,  1583,
    1587,  1591,  1595,  1599,  1603,  1607,  1611,  1615,  1619,  1623,
    1627,  1631,  1635,  1638,  1641,  1644,  1647,  1651,  1655,  1659,
    1663,  1667,  1671,  1675,  1679,  1683,  1687,  1691,  1697,  1702,
    1704,  1707,  1710,  1713,  1716,  1719,  1722,  1725,  1728,  1731,
    1733,  1735,  1737,  1741,  1744,  1746,  1752,  1753,  1754,  1766,
    1767,  1780,  1781,  1786,  1787,  1795,  1796,  1802,  1803,  1807,
    1808,  1815,  1818,  1821,  1826,  1828,  1830,  1836,  1840,  1846,
    1850,  1853,  1854,  1857,  1858,  1863,  1868,  1872,  1877,  1882,
    1887,  1892,  1894,  1896,  1898,  1900,  1904,  1908,  1913,  1915,
    1918,  1923,  1926,  1933,  1934,  1936,  1941,  1942,  1945,  1946,
    1948,  1950,  1954,  1956,  1960,  1962,  1964,  1968,  1972,  1974,
    1976,  1978,  1980,  1982,  1984,  1986,  1988,  1990,  1992,  1994,
    1996,  1998,  2000,  2002,  2004,  2006,  2008,  2010,  2012,  2014,
    2016,  2018,  2020,  2022,  2024,  2026,  2028,  2030,  2032,  2034,
    2036,  2038,  2040,  2042,  2044,  2046,  2048,  2050,  2052,  2054,
    2056,  2058,  2060,  2062,  2064,  2066,  2068,  2070,  2072,  2074,
    2076,  2078,  2080,  2082,  2084,  2086,  2088,  2090,  2092,  2094,
    2096,  2098,  2100,  2102,  2104,  2106,  2108,  2110,  2112,  2114,
    2116,  2118,  2120,  2122,  2124,  2126,  2128,  2130,  2132,  2137,
    2139,  2141,  2143,  2145,  2147,  2149,  2153,  2155,  2159,  2161,
    2163,  2167,  2169,  2171,  2173,  2176,  2178,  2179,  2180,  2182,
    2184,  2188,  2189,  2191,  2193,  2195,  2197,  2199,  2201,  2203,
    2205,  2207,  2209,  2211,  2213,  2215,  2219,  2222,  2224,  2226,
    2231,  2235,  2240,  2242,  2244,  2248,  2252,  2256,  2260,  2264,
    2268,  2272,  2276,  2280,  2284,  2288,  2292,  2296,  2300,  2304,
    2308,  2312,  2316,  2319,  2322,  2325,  2328,  2332,  2336,  2340,
    2344,  2348,  2352,  2356,  2360,  2364,  2370,  2375,  2379,  2383,
    2387,  2389,  2391,  2393,  2395,  2399,  2403,  2407,  2410,  2411,
    2413,  2414,  2416,  2417,  2423,  2427,  2431,  2433,  2435,  2437,
    2439,  2443,  2446,  2448,  2450,  2452,  2454,  2456,  2460,  2462,
    2464,  2466,  2469,  2472,  2477,  2481,  2486,  2489,  2490,  2496,
    2500,  2504,  2506,  2510,  2512,  2515,  2516,  2522,  2526,  2529,
    2530,  2534,  2535,  2540,  2543,  2544,  2548,  2552,  2554,  2555,
    2557,  2559,  2561,  2563,  2567,  2569,  2571,  2573,  2577,  2579,
    2581,  2585,  2589,  2592,  2597,  2600,  2605,  2611,  2617,  2623,
    2629,  2631,  2633,  2635,  2637,  2639,  2641,  2645,  2649,  2654,
    2659,  2663,  2665,  2667,  2669,  2671,  2675,  2677,  2682,  2686,
    2690,  2692,  2694,  2696,  2698,  2700,  2704,  2708,  2713,  2718,
    2722,  2724,  2726,  2734,  2744,  2752,  2759,  2768,  2770,  2775,
    2780,  2782,  2784,  2789,  2792,  2794,  2795,  2797,  2799,  2801,
    2805,  2809,  2813,  2814,  2816,  2818,  2822,  2826,  2829,  2833,
    2840,  2841,  2843,  2848,  2851,  2852,  2858,  2862,  2866,  2868,
    2875,  2880,  2885,  2888,  2891,  2892,  2898,  2902,  2906,  2908,
    2911,  2912,  2918,  2922,  2926,  2928,  2931,  2934,  2936,  2939,
    2941,  2946,  2950,  2954,  2961,  2965,  2967,  2969,  2971,  2976,
    2981,  2986,  2991,  2996,  3001,  3004,  3007,  3012,  3015,  3018,
    3020,  3024,  3028,  3032,  3033,  3036,  3042,  3049,  3056,  3064,
    3066,  3069,  3071,  3074,  3076,  3081,  3083,  3088,  3092,  3093,
    3095,  3099,  3102,  3106,  3108,  3110,  3111,  3112,  3115,  3118,
    3121,  3126,  3129,  3135,  3139,  3141,  3143,  3144,  3148,  3153,
    3159,  3163,  3165,  3168,  3169,  3174,  3176,  3180,  3183,  3186,
    3189,  3191,  3193,  3195,  3197,  3201,  3206,  3213,  3215,  3224,
    3231,  3233
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     197,     0,    -1,    -1,   198,   199,    -1,   199,   200,    -1,
      -1,   220,    -1,   237,    -1,   244,    -1,   241,    -1,   249,
      -1,   439,    -1,   125,   186,   187,   188,    -1,   152,   212,
     188,    -1,    -1,   152,   212,   189,   201,   199,   190,    -1,
      -1,   152,   189,   202,   199,   190,    -1,   113,   206,   188,
      -1,   113,   107,   207,   188,    -1,   113,   108,   208,   188,
      -1,   217,   188,    -1,    78,    -1,    99,    -1,   158,    -1,
     159,    -1,   161,    -1,   163,    -1,   162,    -1,   203,    -1,
     135,    -1,   164,    -1,   128,    -1,   129,    -1,   120,    -1,
     119,    -1,   118,    -1,   117,    -1,   116,    -1,   115,    -1,
     108,    -1,    97,    -1,    93,    -1,    95,    -1,    74,    -1,
      91,    -1,    12,    -1,   114,    -1,   105,    -1,    54,    -1,
     166,    -1,   127,    -1,   152,    -1,    69,    -1,    10,    -1,
      11,    -1,   110,    -1,   113,    -1,   121,    -1,    70,    -1,
     133,    -1,    68,    -1,     7,    -1,     6,    -1,   112,    -1,
     134,    -1,    13,    -1,    88,    -1,     4,    -1,     3,    -1,
     109,    -1,    73,    -1,    72,    -1,   103,    -1,   104,    -1,
     106,    -1,   100,    -1,    27,    -1,   107,    -1,    71,    -1,
     101,    -1,   169,    -1,    92,    -1,    94,    -1,    96,    -1,
     102,    -1,    89,    -1,    90,    -1,    98,    -1,   111,    -1,
     122,    -1,   204,    -1,   126,    -1,   206,     9,   209,    -1,
     209,    -1,   207,     9,   210,    -1,   210,    -1,   208,     9,
     211,    -1,   211,    -1,   212,    -1,   155,   212,    -1,   212,
      98,   203,    -1,   155,   212,    98,   203,    -1,   212,    -1,
     155,   212,    -1,   212,    98,   203,    -1,   155,   212,    98,
     203,    -1,   212,    -1,   155,   212,    -1,   212,    98,   203,
      -1,   155,   212,    98,   203,    -1,   203,    -1,   212,   155,
     203,    -1,   212,    -1,   152,   155,   212,    -1,   155,   212,
      -1,   213,    -1,   213,   444,    -1,   213,   444,    -1,   217,
       9,   440,    14,   379,    -1,   108,   440,    14,   379,    -1,
     218,   219,    -1,    -1,   220,    -1,   237,    -1,   244,    -1,
     249,    -1,   189,   218,   190,    -1,    71,   325,   220,   271,
     273,    -1,    71,   325,    30,   218,   272,   274,    74,   188,
      -1,    -1,    90,   325,   221,   265,    -1,    -1,    89,   222,
     220,    90,   325,   188,    -1,    -1,    92,   186,   327,   188,
     327,   188,   327,   187,   223,   263,    -1,    -1,   100,   325,
     224,   268,    -1,   104,   188,    -1,   104,   334,   188,    -1,
     106,   188,    -1,   106,   334,   188,    -1,   109,   188,    -1,
     109,   334,   188,    -1,    27,   104,   188,    -1,   114,   281,
     188,    -1,   120,   283,   188,    -1,    88,   326,   188,    -1,
     144,   326,   188,    -1,   122,   186,   436,   187,   188,    -1,
     188,    -1,    82,    -1,    83,    -1,    -1,    94,   186,   334,
      98,   262,   261,   187,   225,   264,    -1,    -1,    94,   186,
     334,    28,    98,   262,   261,   187,   226,   264,    -1,    96,
     186,   267,   187,   266,    -1,    -1,   110,   229,   111,   186,
     370,    80,   187,   189,   218,   190,   231,   227,   234,    -1,
      -1,   110,   229,   169,   228,   232,    -1,   112,   334,   188,
      -1,   105,   203,   188,    -1,   334,   188,    -1,   328,   188,
      -1,   329,   188,    -1,   330,   188,    -1,   331,   188,    -1,
     332,   188,    -1,   109,   331,   188,    -1,   333,   188,    -1,
     203,    30,    -1,    -1,   189,   230,   218,   190,    -1,   231,
     111,   186,   370,    80,   187,   189,   218,   190,    -1,    -1,
      -1,   189,   233,   218,   190,    -1,   169,   232,    -1,    -1,
      35,    -1,    -1,   107,    -1,    -1,   236,   235,   443,   238,
     186,   277,   187,   448,   311,    -1,    -1,   315,   236,   235,
     443,   239,   186,   277,   187,   448,   311,    -1,    -1,   400,
     314,   236,   235,   443,   240,   186,   277,   187,   448,   311,
      -1,    -1,   162,   203,   242,    30,   458,   438,   189,   284,
     190,    -1,    -1,   400,   162,   203,   243,    30,   458,   438,
     189,   284,   190,    -1,    -1,   255,   252,   245,   256,   257,
     189,   287,   190,    -1,    -1,   400,   255,   252,   246,   256,
     257,   189,   287,   190,    -1,    -1,   127,   253,   247,   258,
     189,   287,   190,    -1,    -1,   400,   127,   253,   248,   258,
     189,   287,   190,    -1,    -1,   164,   254,   250,   257,   189,
     287,   190,    -1,    -1,   400,   164,   254,   251,   257,   189,
     287,   190,    -1,   443,    -1,   156,    -1,   443,    -1,   443,
      -1,   126,    -1,   119,   126,    -1,   119,   118,   126,    -1,
     118,   119,   126,    -1,   118,   126,    -1,   128,   370,    -1,
      -1,   129,   259,    -1,    -1,   128,   259,    -1,    -1,   370,
      -1,   259,     9,   370,    -1,   370,    -1,   260,     9,   370,
      -1,   132,   262,    -1,    -1,   412,    -1,    35,   412,    -1,
     133,   186,   425,   187,    -1,   220,    -1,    30,   218,    93,
     188,    -1,   220,    -1,    30,   218,    95,   188,    -1,   220,
      -1,    30,   218,    91,   188,    -1,   220,    -1,    30,   218,
      97,   188,    -1,   203,    14,   379,    -1,   267,     9,   203,
      14,   379,    -1,   189,   269,   190,    -1,   189,   188,   269,
     190,    -1,    30,   269,   101,   188,    -1,    30,   188,   269,
     101,   188,    -1,   269,   102,   334,   270,   218,    -1,   269,
     103,   270,   218,    -1,    -1,    30,    -1,   188,    -1,   271,
      72,   325,   220,    -1,    -1,   272,    72,   325,    30,   218,
      -1,    -1,    73,   220,    -1,    -1,    73,    30,   218,    -1,
      -1,   276,     9,   401,   317,   459,   165,    80,    -1,   276,
       9,   401,   317,   459,    35,   165,    80,    -1,   276,     9,
     401,   317,   459,   165,    -1,   276,   384,    -1,   401,   317,
     459,   165,    80,    -1,   401,   317,   459,    35,   165,    80,
      -1,   401,   317,   459,   165,    -1,    -1,   401,   317,   459,
      80,    -1,   401,   317,   459,    35,    80,    -1,   401,   317,
     459,    35,    80,    14,   334,    -1,   401,   317,   459,    80,
      14,   334,    -1,   276,     9,   401,   317,   459,    80,    -1,
     276,     9,   401,   317,   459,    35,    80,    -1,   276,     9,
     401,   317,   459,    35,    80,    14,   334,    -1,   276,     9,
     401,   317,   459,    80,    14,   334,    -1,   278,     9,   401,
     459,   165,    80,    -1,   278,     9,   401,   459,    35,   165,
      80,    -1,   278,     9,   401,   459,   165,    -1,   278,   384,
      -1,   401,   459,   165,    80,    -1,   401,   459,    35,   165,
      80,    -1,   401,   459,   165,    -1,    -1,   401,   459,    80,
      -1,   401,   459,    35,    80,    -1,   401,   459,    35,    80,
      14,   334,    -1,   401,   459,    80,    14,   334,    -1,   278,
       9,   401,   459,    80,    -1,   278,     9,   401,   459,    35,
      80,    -1,   278,     9,   401,   459,    35,    80,    14,   334,
      -1,   278,     9,   401,   459,    80,    14,   334,    -1,   280,
     384,    -1,    -1,   334,    -1,    35,   412,    -1,   165,   334,
      -1,   280,     9,   334,    -1,   280,     9,   165,   334,    -1,
     280,     9,    35,   412,    -1,   281,     9,   282,    -1,   282,
      -1,    80,    -1,   191,   412,    -1,   191,   189,   334,   190,
      -1,   283,     9,    80,    -1,   283,     9,    80,    14,   379,
      -1,    80,    -1,    80,    14,   379,    -1,   284,   285,    -1,
      -1,   286,   188,    -1,   441,    14,   379,    -1,   287,   288,
      -1,    -1,    -1,   313,   289,   319,   188,    -1,    -1,   315,
     458,   290,   319,   188,    -1,   320,   188,    -1,   321,   188,
      -1,   322,   188,    -1,    -1,   314,   236,   235,   442,   186,
     291,   275,   187,   448,   312,    -1,    -1,   400,   314,   236,
     235,   443,   186,   292,   275,   187,   448,   312,    -1,   158,
     297,   188,    -1,   159,   305,   188,    -1,   161,   307,   188,
      -1,     4,   128,   370,   188,    -1,     4,   129,   370,   188,
      -1,   113,   260,   188,    -1,   113,   260,   189,   293,   190,
      -1,   293,   294,    -1,   293,   295,    -1,    -1,   216,   151,
     203,   166,   260,   188,    -1,   296,    98,   314,   203,   188,
      -1,   296,    98,   315,   188,    -1,   216,   151,   203,    -1,
     203,    -1,   298,    -1,   297,     9,   298,    -1,   299,   367,
     303,   304,    -1,   156,    -1,    29,   300,    -1,   300,    -1,
     134,    -1,   134,   172,   458,   173,    -1,   134,   172,   458,
       9,   458,   173,    -1,   370,    -1,   121,    -1,   162,   189,
     302,   190,    -1,   135,    -1,   378,    -1,   301,     9,   378,
      -1,   301,   383,    -1,    14,   379,    -1,    -1,    56,   163,
      -1,    -1,   306,    -1,   305,     9,   306,    -1,   160,    -1,
     308,    -1,   203,    -1,   124,    -1,   186,   309,   187,    -1,
     186,   309,   187,    50,    -1,   186,   309,   187,    29,    -1,
     186,   309,   187,    47,    -1,   308,    -1,   310,    -1,   310,
      50,    -1,   310,    29,    -1,   310,    47,    -1,   309,     9,
     309,    -1,   309,    33,   309,    -1,   203,    -1,   156,    -1,
     160,    -1,   188,    -1,   189,   218,   190,    -1,   188,    -1,
     189,   218,   190,    -1,   315,    -1,   121,    -1,   315,    -1,
      -1,   316,    -1,   315,   316,    -1,   115,    -1,   116,    -1,
     117,    -1,   120,    -1,   119,    -1,   118,    -1,   182,    -1,
     318,    -1,    -1,   115,    -1,   116,    -1,   117,    -1,   319,
       9,    80,    -1,   319,     9,    80,    14,   379,    -1,    80,
      -1,    80,    14,   379,    -1,   320,     9,   441,    14,   379,
      -1,   108,   441,    14,   379,    -1,   321,     9,   441,    -1,
     119,   108,   441,    -1,   119,   323,   438,    -1,   323,   438,
      14,   458,    -1,   108,   177,   443,    -1,   186,   324,   187,
      -1,    69,   374,   377,    -1,    68,   334,    -1,   359,    -1,
     354,    -1,   186,   334,   187,    -1,   326,     9,   334,    -1,
     334,    -1,   326,    -1,    -1,    27,    -1,    27,   334,    -1,
      27,   334,   132,   334,    -1,   186,   328,   187,    -1,   412,
      14,   328,    -1,   133,   186,   425,   187,    14,   328,    -1,
      28,   334,    -1,   412,    14,   331,    -1,   133,   186,   425,
     187,    14,   331,    -1,   335,    -1,   412,    -1,   324,    -1,
     416,    -1,   415,    -1,   133,   186,   425,   187,    14,   334,
      -1,   412,    14,   334,    -1,   412,    14,    35,   412,    -1,
     412,    14,    35,    69,   374,   377,    -1,   412,    26,   334,
      -1,   412,    25,   334,    -1,   412,    24,   334,    -1,   412,
      23,   334,    -1,   412,    22,   334,    -1,   412,    21,   334,
      -1,   412,    20,   334,    -1,   412,    19,   334,    -1,   412,
      18,   334,    -1,   412,    17,   334,    -1,   412,    16,   334,
      -1,   412,    15,   334,    -1,   412,    65,    -1,    65,   412,
      -1,   412,    64,    -1,    64,   412,    -1,   334,    31,   334,
      -1,   334,    32,   334,    -1,   334,    10,   334,    -1,   334,
      12,   334,    -1,   334,    11,   334,    -1,   334,    33,   334,
      -1,   334,    35,   334,    -1,   334,    34,   334,    -1,   334,
      49,   334,    -1,   334,    47,   334,    -1,   334,    48,   334,
      -1,   334,    50,   334,    -1,   334,    51,   334,    -1,   334,
      66,   334,    -1,   334,    52,   334,    -1,   334,    46,   334,
      -1,   334,    45,   334,    -1,    47,   334,    -1,    48,   334,
      -1,    53,   334,    -1,    55,   334,    -1,   334,    37,   334,
      -1,   334,    36,   334,    -1,   334,    39,   334,    -1,   334,
      38,   334,    -1,   334,    40,   334,    -1,   334,    44,   334,
      -1,   334,    41,   334,    -1,   334,    43,   334,    -1,   334,
      42,   334,    -1,   334,    54,   374,    -1,   186,   335,   187,
      -1,   334,    29,   334,    30,   334,    -1,   334,    29,    30,
     334,    -1,   435,    -1,    63,   334,    -1,    62,   334,    -1,
      61,   334,    -1,    60,   334,    -1,    59,   334,    -1,    58,
     334,    -1,    57,   334,    -1,    70,   375,    -1,    56,   334,
      -1,   381,    -1,   353,    -1,   352,    -1,   192,   376,   192,
      -1,    13,   334,    -1,   356,    -1,   113,   186,   358,   384,
     187,    -1,    -1,    -1,   236,   235,   186,   338,   277,   187,
     448,   336,   189,   218,   190,    -1,    -1,   315,   236,   235,
     186,   339,   277,   187,   448,   336,   189,   218,   190,    -1,
      -1,   182,    80,   341,   346,    -1,    -1,   182,   183,   342,
     277,   184,   448,   346,    -1,    -1,   182,   189,   343,   218,
     190,    -1,    -1,    80,   344,   346,    -1,    -1,   183,   345,
     277,   184,   448,   346,    -1,     8,   334,    -1,     8,   331,
      -1,     8,   189,   218,   190,    -1,    87,    -1,   437,    -1,
     348,     9,   347,   132,   334,    -1,   347,   132,   334,    -1,
     349,     9,   347,   132,   379,    -1,   347,   132,   379,    -1,
     348,   383,    -1,    -1,   349,   383,    -1,    -1,   176,   186,
     350,   187,    -1,   134,   186,   426,   187,    -1,    67,   426,
     193,    -1,   370,   189,   428,   190,    -1,   370,   189,   430,
     190,    -1,   356,    67,   422,   193,    -1,   357,    67,   422,
     193,    -1,   353,    -1,   437,    -1,   415,    -1,    87,    -1,
     186,   335,   187,    -1,   358,     9,    80,    -1,   358,     9,
      35,    80,    -1,    80,    -1,    35,    80,    -1,   170,   156,
     360,   171,    -1,   362,    51,    -1,   362,   171,   363,   170,
      51,   361,    -1,    -1,   156,    -1,   362,   364,    14,   365,
      -1,    -1,   363,   366,    -1,    -1,   156,    -1,   157,    -1,
     189,   334,   190,    -1,   157,    -1,   189,   334,   190,    -1,
     359,    -1,   368,    -1,   367,    30,   368,    -1,   367,    48,
     368,    -1,   203,    -1,    70,    -1,   107,    -1,   108,    -1,
     109,    -1,    27,    -1,    28,    -1,   110,    -1,   111,    -1,
     169,    -1,   112,    -1,    71,    -1,    72,    -1,    74,    -1,
      73,    -1,    90,    -1,    91,    -1,    89,    -1,    92,    -1,
      93,    -1,    94,    -1,    95,    -1,    96,    -1,    97,    -1,
      54,    -1,    98,    -1,   100,    -1,   101,    -1,   102,    -1,
     103,    -1,   104,    -1,   106,    -1,   105,    -1,    88,    -1,
      13,    -1,   126,    -1,   127,    -1,   128,    -1,   129,    -1,
      69,    -1,    68,    -1,   121,    -1,     5,    -1,     7,    -1,
       6,    -1,     4,    -1,     3,    -1,   152,    -1,   113,    -1,
     114,    -1,   123,    -1,   124,    -1,   125,    -1,   120,    -1,
     119,    -1,   118,    -1,   117,    -1,   116,    -1,   115,    -1,
     182,    -1,   122,    -1,   133,    -1,   134,    -1,    10,    -1,
      12,    -1,    11,    -1,   136,    -1,   138,    -1,   137,    -1,
     139,    -1,   140,    -1,   154,    -1,   153,    -1,   181,    -1,
     164,    -1,   167,    -1,   166,    -1,   177,    -1,   179,    -1,
     176,    -1,   215,   186,   279,   187,    -1,   216,    -1,   156,
      -1,   370,    -1,   378,    -1,   120,    -1,   420,    -1,   186,
     335,   187,    -1,   371,    -1,   372,   151,   421,    -1,   371,
      -1,   418,    -1,   373,   151,   421,    -1,   370,    -1,   120,
      -1,   423,    -1,   186,   187,    -1,   325,    -1,    -1,    -1,
      86,    -1,   432,    -1,   186,   279,   187,    -1,    -1,    75,
      -1,    76,    -1,    77,    -1,    87,    -1,   139,    -1,   140,
      -1,   154,    -1,   136,    -1,   167,    -1,   137,    -1,   138,
      -1,   153,    -1,   181,    -1,   147,    86,   148,    -1,   147,
     148,    -1,   378,    -1,   214,    -1,   134,   186,   382,   187,
      -1,    67,   382,   193,    -1,   176,   186,   351,   187,    -1,
     380,    -1,   355,    -1,   186,   379,   187,    -1,   379,    31,
     379,    -1,   379,    32,   379,    -1,   379,    10,   379,    -1,
     379,    12,   379,    -1,   379,    11,   379,    -1,   379,    33,
     379,    -1,   379,    35,   379,    -1,   379,    34,   379,    -1,
     379,    49,   379,    -1,   379,    47,   379,    -1,   379,    48,
     379,    -1,   379,    50,   379,    -1,   379,    51,   379,    -1,
     379,    52,   379,    -1,   379,    46,   379,    -1,   379,    45,
     379,    -1,   379,    66,   379,    -1,    53,   379,    -1,    55,
     379,    -1,    47,   379,    -1,    48,   379,    -1,   379,    37,
     379,    -1,   379,    36,   379,    -1,   379,    39,   379,    -1,
     379,    38,   379,    -1,   379,    40,   379,    -1,   379,    44,
     379,    -1,   379,    41,   379,    -1,   379,    43,   379,    -1,
     379,    42,   379,    -1,   379,    29,   379,    30,   379,    -1,
     379,    29,    30,   379,    -1,   216,   151,   204,    -1,   156,
     151,   204,    -1,   216,   151,   126,    -1,   214,    -1,    79,
      -1,   437,    -1,   378,    -1,   194,   432,   194,    -1,   195,
     432,   195,    -1,   147,   432,   148,    -1,   385,   383,    -1,
      -1,     9,    -1,    -1,     9,    -1,    -1,   385,     9,   379,
     132,   379,    -1,   385,     9,   379,    -1,   379,   132,   379,
      -1,   379,    -1,    75,    -1,    76,    -1,    77,    -1,   147,
      86,   148,    -1,   147,   148,    -1,    75,    -1,    76,    -1,
      77,    -1,   203,    -1,    87,    -1,    87,    49,   388,    -1,
     386,    -1,   388,    -1,   203,    -1,    47,   387,    -1,    48,
     387,    -1,   134,   186,   390,   187,    -1,    67,   390,   193,
      -1,   176,   186,   393,   187,    -1,   391,   383,    -1,    -1,
     391,     9,   389,   132,   389,    -1,   391,     9,   389,    -1,
     389,   132,   389,    -1,   389,    -1,   392,     9,   389,    -1,
     389,    -1,   394,   383,    -1,    -1,   394,     9,   347,   132,
     389,    -1,   347,   132,   389,    -1,   392,   383,    -1,    -1,
     186,   395,   187,    -1,    -1,   397,     9,   203,   396,    -1,
     203,   396,    -1,    -1,   399,   397,   383,    -1,    46,   398,
      45,    -1,   400,    -1,    -1,   130,    -1,   131,    -1,   203,
      -1,   156,    -1,   189,   334,   190,    -1,   403,    -1,   421,
      -1,   203,    -1,   189,   334,   190,    -1,   405,    -1,   421,
      -1,    67,   422,   193,    -1,   189,   334,   190,    -1,   413,
     407,    -1,   186,   324,   187,   407,    -1,   424,   407,    -1,
     186,   324,   187,   407,    -1,   186,   324,   187,   402,   404,
      -1,   186,   335,   187,   402,   404,    -1,   186,   324,   187,
     402,   403,    -1,   186,   335,   187,   402,   403,    -1,   419,
      -1,   369,    -1,   417,    -1,   418,    -1,   408,    -1,   410,
      -1,   412,   402,   404,    -1,   373,   151,   421,    -1,   414,
     186,   279,   187,    -1,   415,   186,   279,   187,    -1,   186,
     412,   187,    -1,   369,    -1,   417,    -1,   418,    -1,   408,
      -1,   412,   402,   404,    -1,   411,    -1,   414,   186,   279,
     187,    -1,   186,   412,   187,    -1,   373,   151,   421,    -1,
     419,    -1,   408,    -1,   369,    -1,   353,    -1,   378,    -1,
     186,   412,   187,    -1,   186,   335,   187,    -1,   415,   186,
     279,   187,    -1,   414,   186,   279,   187,    -1,   186,   416,
     187,    -1,   337,    -1,   340,    -1,   412,   402,   406,   444,
     186,   279,   187,    -1,   186,   324,   187,   402,   406,   444,
     186,   279,   187,    -1,   373,   151,   205,   444,   186,   279,
     187,    -1,   373,   151,   421,   186,   279,   187,    -1,   373,
     151,   189,   334,   190,   186,   279,   187,    -1,   420,    -1,
     420,    67,   422,   193,    -1,   420,   189,   334,   190,    -1,
     421,    -1,    80,    -1,   191,   189,   334,   190,    -1,   191,
     421,    -1,   334,    -1,    -1,   419,    -1,   409,    -1,   410,
      -1,   423,   402,   404,    -1,   372,   151,   419,    -1,   186,
     412,   187,    -1,    -1,   409,    -1,   411,    -1,   423,   402,
     403,    -1,   186,   412,   187,    -1,   425,     9,    -1,   425,
       9,   412,    -1,   425,     9,   133,   186,   425,   187,    -1,
      -1,   412,    -1,   133,   186,   425,   187,    -1,   427,   383,
      -1,    -1,   427,     9,   334,   132,   334,    -1,   427,     9,
     334,    -1,   334,   132,   334,    -1,   334,    -1,   427,     9,
     334,   132,    35,   412,    -1,   427,     9,    35,   412,    -1,
     334,   132,    35,   412,    -1,    35,   412,    -1,   429,   383,
      -1,    -1,   429,     9,   334,   132,   334,    -1,   429,     9,
     334,    -1,   334,   132,   334,    -1,   334,    -1,   431,   383,
      -1,    -1,   431,     9,   379,   132,   379,    -1,   431,     9,
     379,    -1,   379,   132,   379,    -1,   379,    -1,   432,   433,
      -1,   432,    86,    -1,   433,    -1,    86,   433,    -1,    80,
      -1,    80,    67,   434,   193,    -1,    80,   402,   203,    -1,
     149,   334,   190,    -1,   149,    79,    67,   334,   193,   190,
      -1,   150,   412,   190,    -1,   203,    -1,    81,    -1,    80,
      -1,   123,   186,   326,   187,    -1,   124,   186,   412,   187,
      -1,   124,   186,   335,   187,    -1,   124,   186,   416,   187,
      -1,   124,   186,   415,   187,    -1,   124,   186,   324,   187,
      -1,     7,   334,    -1,     6,   334,    -1,     5,   186,   334,
     187,    -1,     4,   334,    -1,     3,   334,    -1,   412,    -1,
     436,     9,   412,    -1,   373,   151,   204,    -1,   373,   151,
     126,    -1,    -1,    98,   458,    -1,   177,   443,    14,   458,
     188,    -1,   400,   177,   443,    14,   458,   188,    -1,   179,
     443,   438,    14,   458,   188,    -1,   400,   179,   443,   438,
      14,   458,   188,    -1,   205,    -1,   458,   205,    -1,   204,
      -1,   458,   204,    -1,   205,    -1,   205,   172,   450,   173,
      -1,   203,    -1,   203,   172,   450,   173,    -1,   172,   446,
     173,    -1,    -1,   458,    -1,   445,     9,   458,    -1,   445,
     383,    -1,   445,     9,   165,    -1,   446,    -1,   165,    -1,
      -1,    -1,    30,   458,    -1,    98,   458,    -1,    99,   458,
      -1,   450,     9,   451,   203,    -1,   451,   203,    -1,   450,
       9,   451,   203,   449,    -1,   451,   203,   449,    -1,    47,
      -1,    48,    -1,    -1,    87,   132,   458,    -1,    29,    87,
     132,   458,    -1,   216,   151,   203,   132,   458,    -1,   453,
       9,   452,    -1,   452,    -1,   453,   383,    -1,    -1,   176,
     186,   454,   187,    -1,   216,    -1,   203,   151,   457,    -1,
     203,   444,    -1,    29,   458,    -1,    56,   458,    -1,   216,
      -1,   134,    -1,   135,    -1,   455,    -1,   456,   151,   457,
      -1,   134,   172,   458,   173,    -1,   134,   172,   458,     9,
     458,   173,    -1,   156,    -1,   186,   107,   186,   447,   187,
      30,   458,   187,    -1,   186,   458,     9,   445,   383,   187,
      -1,   458,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   725,   725,   725,   734,   736,   739,   740,   741,   742,
     743,   744,   745,   748,   750,   750,   752,   752,   754,   755,
     757,   759,   764,   765,   766,   767,   768,   769,   770,   774,
     775,   776,   777,   778,   779,   780,   781,   782,   783,   784,
     785,   786,   787,   788,   789,   790,   791,   792,   793,   794,
     795,   796,   797,   798,   799,   800,   801,   802,   803,   804,
     805,   806,   807,   808,   809,   810,   811,   812,   813,   814,
     815,   816,   817,   818,   819,   820,   821,   822,   823,   824,
     825,   826,   827,   828,   829,   830,   831,   832,   833,   834,
     838,   842,   843,   847,   849,   853,   855,   859,   861,   865,
     866,   867,   868,   873,   874,   875,   876,   881,   882,   883,
     884,   889,   890,   894,   895,   897,   900,   906,   913,   920,
     924,   930,   932,   935,   936,   937,   938,   941,   942,   946,
     951,   951,   957,   957,   964,   963,   969,   969,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   992,   990,   999,   997,  1004,  1012,  1006,
    1016,  1014,  1018,  1019,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1039,  1039,  1044,  1050,  1054,  1054,  1062,
    1063,  1067,  1068,  1072,  1078,  1076,  1091,  1088,  1104,  1101,
    1118,  1117,  1126,  1124,  1136,  1135,  1154,  1152,  1171,  1170,
    1179,  1177,  1189,  1188,  1200,  1198,  1211,  1212,  1216,  1219,
    1222,  1223,  1224,  1227,  1228,  1231,  1233,  1236,  1237,  1240,
    1241,  1244,  1245,  1249,  1250,  1255,  1256,  1259,  1260,  1261,
    1265,  1266,  1270,  1271,  1275,  1276,  1280,  1281,  1286,  1287,
    1292,  1293,  1294,  1295,  1298,  1301,  1303,  1306,  1307,  1311,
    1313,  1316,  1319,  1322,  1323,  1326,  1327,  1331,  1337,  1343,
    1350,  1352,  1357,  1362,  1368,  1372,  1376,  1380,  1385,  1390,
    1395,  1400,  1406,  1415,  1420,  1425,  1431,  1433,  1437,  1441,
    1446,  1450,  1453,  1456,  1460,  1464,  1468,  1472,  1477,  1485,
    1487,  1490,  1491,  1492,  1493,  1495,  1497,  1502,  1503,  1506,
    1507,  1508,  1512,  1513,  1515,  1516,  1520,  1522,  1525,  1529,
    1535,  1537,  1540,  1540,  1544,  1543,  1547,  1549,  1552,  1555,
    1553,  1569,  1565,  1579,  1581,  1583,  1585,  1587,  1589,  1591,
    1595,  1596,  1597,  1600,  1606,  1610,  1616,  1619,  1624,  1626,
    1631,  1636,  1640,  1641,  1645,  1646,  1648,  1650,  1656,  1657,
    1659,  1663,  1664,  1669,  1673,  1674,  1678,  1679,  1683,  1685,
    1691,  1696,  1697,  1699,  1703,  1704,  1705,  1706,  1710,  1711,
    1712,  1713,  1714,  1715,  1717,  1722,  1725,  1726,  1730,  1731,
    1735,  1736,  1739,  1740,  1743,  1744,  1747,  1748,  1752,  1753,
    1754,  1755,  1756,  1757,  1758,  1762,  1763,  1766,  1767,  1768,
    1771,  1773,  1775,  1776,  1779,  1781,  1785,  1787,  1791,  1795,
    1799,  1804,  1805,  1807,  1808,  1809,  1812,  1816,  1817,  1821,
    1822,  1826,  1827,  1828,  1829,  1833,  1837,  1842,  1846,  1850,
    1855,  1856,  1857,  1858,  1859,  1863,  1865,  1866,  1867,  1870,
    1871,  1872,  1873,  1874,  1875,  1876,  1877,  1878,  1879,  1880,
    1881,  1882,  1883,  1884,  1885,  1886,  1887,  1888,  1889,  1890,
    1891,  1892,  1893,  1894,  1895,  1896,  1897,  1898,  1899,  1900,
    1901,  1902,  1903,  1904,  1905,  1906,  1907,  1908,  1909,  1910,
    1911,  1912,  1914,  1915,  1917,  1918,  1920,  1921,  1922,  1923,
    1924,  1925,  1926,  1927,  1928,  1929,  1930,  1931,  1932,  1933,
    1934,  1935,  1936,  1937,  1938,  1942,  1946,  1951,  1950,  1965,
    1963,  1981,  1980,  1999,  1998,  2017,  2016,  2034,  2034,  2049,
    2049,  2067,  2068,  2069,  2074,  2076,  2080,  2084,  2090,  2094,
    2100,  2102,  2106,  2108,  2112,  2116,  2117,  2121,  2128,  2135,
    2137,  2142,  2143,  2144,  2145,  2147,  2151,  2152,  2153,  2154,
    2158,  2164,  2173,  2186,  2187,  2190,  2193,  2196,  2197,  2200,
    2204,  2207,  2210,  2217,  2218,  2222,  2223,  2225,  2229,  2230,
    2231,  2232,  2233,  2234,  2235,  2236,  2237,  2238,  2239,  2240,
    2241,  2242,  2243,  2244,  2245,  2246,  2247,  2248,  2249,  2250,
    2251,  2252,  2253,  2254,  2255,  2256,  2257,  2258,  2259,  2260,
    2261,  2262,  2263,  2264,  2265,  2266,  2267,  2268,  2269,  2270,
    2271,  2272,  2273,  2274,  2275,  2276,  2277,  2278,  2279,  2280,
    2281,  2282,  2283,  2284,  2285,  2286,  2287,  2288,  2289,  2290,
    2291,  2292,  2293,  2294,  2295,  2296,  2297,  2298,  2299,  2300,
    2301,  2302,  2303,  2304,  2305,  2306,  2307,  2308,  2312,  2317,
    2318,  2322,  2323,  2324,  2325,  2327,  2331,  2332,  2343,  2344,
    2346,  2358,  2359,  2360,  2364,  2365,  2366,  2370,  2371,  2372,
    2375,  2377,  2381,  2382,  2383,  2384,  2386,  2387,  2388,  2389,
    2390,  2391,  2392,  2393,  2394,  2395,  2398,  2403,  2404,  2405,
    2407,  2408,  2410,  2411,  2412,  2413,  2415,  2417,  2419,  2421,
    2423,  2424,  2425,  2426,  2427,  2428,  2429,  2430,  2431,  2432,
    2433,  2434,  2435,  2436,  2437,  2438,  2439,  2441,  2443,  2445,
    2447,  2448,  2451,  2452,  2456,  2460,  2462,  2466,  2469,  2472,
    2478,  2479,  2480,  2481,  2482,  2483,  2484,  2489,  2491,  2495,
    2496,  2499,  2500,  2504,  2507,  2509,  2511,  2515,  2516,  2517,
    2518,  2521,  2525,  2526,  2527,  2528,  2532,  2534,  2541,  2542,
    2543,  2544,  2545,  2546,  2548,  2549,  2554,  2556,  2559,  2562,
    2564,  2566,  2569,  2571,  2575,  2577,  2580,  2583,  2589,  2591,
    2594,  2595,  2600,  2603,  2607,  2607,  2612,  2615,  2616,  2620,
    2621,  2625,  2626,  2627,  2631,  2636,  2641,  2642,  2646,  2651,
    2656,  2657,  2661,  2662,  2667,  2669,  2674,  2685,  2699,  2711,
    2726,  2727,  2728,  2729,  2730,  2731,  2732,  2742,  2751,  2753,
    2755,  2759,  2760,  2761,  2762,  2763,  2779,  2780,  2782,  2784,
    2791,  2792,  2793,  2794,  2795,  2796,  2797,  2798,  2800,  2805,
    2809,  2810,  2814,  2817,  2824,  2828,  2837,  2844,  2852,  2854,
    2855,  2859,  2860,  2862,  2867,  2868,  2879,  2880,  2881,  2882,
    2893,  2896,  2899,  2900,  2901,  2902,  2913,  2917,  2918,  2919,
    2921,  2922,  2923,  2927,  2929,  2932,  2934,  2935,  2936,  2937,
    2940,  2942,  2943,  2947,  2949,  2952,  2954,  2955,  2956,  2960,
    2962,  2965,  2968,  2970,  2972,  2976,  2977,  2979,  2980,  2986,
    2987,  2989,  2999,  3001,  3003,  3006,  3007,  3008,  3012,  3013,
    3014,  3015,  3016,  3017,  3018,  3019,  3020,  3021,  3022,  3026,
    3027,  3031,  3033,  3041,  3043,  3047,  3051,  3056,  3060,  3068,
    3069,  3073,  3074,  3080,  3081,  3090,  3091,  3099,  3102,  3106,
    3109,  3114,  3119,  3121,  3122,  3123,  3127,  3128,  3132,  3133,
    3136,  3139,  3141,  3145,  3151,  3152,  3153,  3157,  3161,  3171,
    3179,  3181,  3185,  3187,  3192,  3198,  3201,  3206,  3214,  3217,
    3220,  3221,  3224,  3227,  3228,  3233,  3236,  3240,  3244,  3250,
    3260,  3261
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
  "T_COMPILER_HALT_OFFSET", "T_ASYNC", "T_LAMBDA_OP", "T_LAMBDA_CP",
  "T_UNRESOLVED_OP", "'('", "')'", "';'", "'{'", "'}'", "'$'", "'`'",
  "']'", "'\"'", "'\\''", "$accept", "start", "$@1", "top_statement_list",
  "top_statement", "$@2", "$@3", "ident_no_semireserved",
  "ident_for_class_const", "ident", "use_declarations",
  "use_fn_declarations", "use_const_declarations", "use_declaration",
  "use_fn_declaration", "use_const_declaration", "namespace_name",
  "namespace_string_base", "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "try_statement_list", "$@12",
  "additional_catches", "finally_statement_list", "$@13",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@14", "$@15", "$@16",
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
     416,   417,   418,   419,   420,   421,    40,    41,    59,   123,
     125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   196,   198,   197,   199,   199,   200,   200,   200,   200,
     200,   200,   200,   200,   201,   200,   202,   200,   200,   200,
     200,   200,   203,   203,   203,   203,   203,   203,   203,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   205,   205,   206,   206,   207,   207,   208,   208,   209,
     209,   209,   209,   210,   210,   210,   210,   211,   211,   211,
     211,   212,   212,   213,   213,   213,   214,   215,   216,   217,
     217,   218,   218,   219,   219,   219,   219,   220,   220,   220,
     221,   220,   222,   220,   223,   220,   224,   220,   220,   220,
     220,   220,   220,   220,   220,   220,   220,   220,   220,   220,
     220,   220,   220,   225,   220,   226,   220,   220,   227,   220,
     228,   220,   220,   220,   220,   220,   220,   220,   220,   220,
     220,   220,   220,   230,   229,   231,   231,   233,   232,   234,
     234,   235,   235,   236,   238,   237,   239,   237,   240,   237,
     242,   241,   243,   241,   245,   244,   246,   244,   247,   244,
     248,   244,   250,   249,   251,   249,   252,   252,   253,   254,
     255,   255,   255,   255,   255,   256,   256,   257,   257,   258,
     258,   259,   259,   260,   260,   261,   261,   262,   262,   262,
     263,   263,   264,   264,   265,   265,   266,   266,   267,   267,
     268,   268,   268,   268,   269,   269,   269,   270,   270,   271,
     271,   272,   272,   273,   273,   274,   274,   275,   275,   275,
     275,   275,   275,   275,   275,   276,   276,   276,   276,   276,
     276,   276,   276,   277,   277,   277,   277,   277,   277,   277,
     277,   278,   278,   278,   278,   278,   278,   278,   278,   279,
     279,   280,   280,   280,   280,   280,   280,   281,   281,   282,
     282,   282,   283,   283,   283,   283,   284,   284,   285,   286,
     287,   287,   289,   288,   290,   288,   288,   288,   288,   291,
     288,   292,   288,   288,   288,   288,   288,   288,   288,   288,
     293,   293,   293,   294,   295,   295,   296,   296,   297,   297,
     298,   298,   299,   299,   300,   300,   300,   300,   300,   300,
     300,   301,   301,   302,   303,   303,   304,   304,   305,   305,
     306,   307,   307,   307,   308,   308,   308,   308,   309,   309,
     309,   309,   309,   309,   309,   310,   310,   310,   311,   311,
     312,   312,   313,   313,   314,   314,   315,   315,   316,   316,
     316,   316,   316,   316,   316,   317,   317,   318,   318,   318,
     319,   319,   319,   319,   320,   320,   321,   321,   322,   322,
     323,   324,   324,   324,   324,   324,   325,   326,   326,   327,
     327,   328,   328,   328,   328,   329,   330,   331,   332,   333,
     334,   334,   334,   334,   334,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   336,   336,   338,   337,   339,
     337,   341,   340,   342,   340,   343,   340,   344,   340,   345,
     340,   346,   346,   346,   347,   347,   348,   348,   349,   349,
     350,   350,   351,   351,   352,   353,   353,   354,   355,   356,
     356,   357,   357,   357,   357,   357,   358,   358,   358,   358,
     359,   360,   360,   361,   361,   362,   362,   363,   363,   364,
     365,   365,   366,   366,   366,   367,   367,   367,   368,   368,
     368,   368,   368,   368,   368,   368,   368,   368,   368,   368,
     368,   368,   368,   368,   368,   368,   368,   368,   368,   368,
     368,   368,   368,   368,   368,   368,   368,   368,   368,   368,
     368,   368,   368,   368,   368,   368,   368,   368,   368,   368,
     368,   368,   368,   368,   368,   368,   368,   368,   368,   368,
     368,   368,   368,   368,   368,   368,   368,   368,   368,   368,
     368,   368,   368,   368,   368,   368,   368,   368,   368,   368,
     368,   368,   368,   368,   368,   368,   368,   368,   369,   370,
     370,   371,   371,   371,   371,   371,   372,   372,   373,   373,
     373,   374,   374,   374,   375,   375,   375,   376,   376,   376,
     377,   377,   378,   378,   378,   378,   378,   378,   378,   378,
     378,   378,   378,   378,   378,   378,   378,   379,   379,   379,
     379,   379,   379,   379,   379,   379,   379,   379,   379,   379,
     379,   379,   379,   379,   379,   379,   379,   379,   379,   379,
     379,   379,   379,   379,   379,   379,   379,   379,   379,   379,
     379,   379,   379,   379,   379,   379,   379,   380,   380,   380,
     381,   381,   381,   381,   381,   381,   381,   382,   382,   383,
     383,   384,   384,   385,   385,   385,   385,   386,   386,   386,
     386,   386,   387,   387,   387,   387,   388,   388,   389,   389,
     389,   389,   389,   389,   389,   389,   390,   390,   391,   391,
     391,   391,   392,   392,   393,   393,   394,   394,   395,   395,
     396,   396,   397,   397,   399,   398,   400,   401,   401,   402,
     402,   403,   403,   403,   404,   404,   405,   405,   406,   406,
     407,   407,   408,   408,   409,   409,   410,   410,   411,   411,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   413,   413,   413,   413,   413,   413,   413,   413,   413,
     414,   414,   414,   414,   414,   414,   414,   414,   414,   415,
     416,   416,   417,   417,   418,   418,   418,   419,   420,   420,
     420,   421,   421,   421,   422,   422,   423,   423,   423,   423,
     423,   423,   424,   424,   424,   424,   424,   425,   425,   425,
     425,   425,   425,   426,   426,   427,   427,   427,   427,   427,
     427,   427,   427,   428,   428,   429,   429,   429,   429,   430,
     430,   431,   431,   431,   431,   432,   432,   432,   432,   433,
     433,   433,   433,   433,   433,   434,   434,   434,   435,   435,
     435,   435,   435,   435,   435,   435,   435,   435,   435,   436,
     436,   437,   437,   438,   438,   439,   439,   439,   439,   440,
     440,   441,   441,   442,   442,   443,   443,   444,   444,   445,
     445,   446,   447,   447,   447,   447,   448,   448,   449,   449,
     450,   450,   450,   450,   451,   451,   451,   452,   452,   452,
     453,   453,   454,   454,   455,   456,   457,   457,   458,   458,
     458,   458,   458,   458,   458,   458,   458,   458,   458,   458,
     459,   459
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
       4,     1,     3,     1,     3,     2,     1,     2,     2,     5,
       4,     2,     0,     1,     1,     1,     1,     3,     5,     8,
       0,     4,     0,     6,     0,    10,     0,     4,     2,     3,
       2,     3,     2,     3,     3,     3,     3,     3,     3,     5,
       1,     1,     1,     0,     9,     0,    10,     5,     0,    13,
       0,     5,     3,     3,     2,     2,     2,     2,     2,     2,
       3,     2,     2,     0,     4,     9,     0,     0,     4,     2,
       0,     1,     0,     1,     0,     9,     0,    10,     0,    11,
       0,     9,     0,    10,     0,     8,     0,     9,     0,     7,
       0,     8,     0,     7,     0,     8,     1,     1,     1,     1,
       1,     2,     3,     3,     2,     2,     0,     2,     0,     2,
       0,     1,     3,     1,     3,     2,     0,     1,     2,     4,
       1,     4,     1,     4,     1,     4,     1,     4,     3,     5,
       3,     4,     4,     5,     5,     4,     0,     1,     1,     4,
       0,     5,     0,     2,     0,     3,     0,     7,     8,     6,
       2,     5,     6,     4,     0,     4,     5,     7,     6,     6,
       7,     9,     8,     6,     7,     5,     2,     4,     5,     3,
       0,     3,     4,     6,     5,     5,     6,     8,     7,     2,
       0,     1,     2,     2,     3,     4,     4,     3,     1,     1,
       2,     4,     3,     5,     1,     3,     2,     0,     2,     3,
       2,     0,     0,     4,     0,     5,     2,     2,     2,     0,
      10,     0,    11,     3,     3,     3,     4,     4,     3,     5,
       2,     2,     0,     6,     5,     4,     3,     1,     1,     3,
       4,     1,     2,     1,     1,     4,     6,     1,     1,     4,
       1,     1,     3,     2,     2,     0,     2,     0,     1,     3,
       1,     1,     1,     1,     3,     4,     4,     4,     1,     1,
       2,     2,     2,     3,     3,     1,     1,     1,     1,     3,
       1,     3,     1,     1,     1,     0,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     1,     1,     1,
       3,     5,     1,     3,     5,     4,     3,     3,     3,     4,
       3,     3,     3,     2,     1,     1,     3,     3,     1,     1,
       0,     1,     2,     4,     3,     3,     6,     2,     3,     6,
       1,     1,     1,     1,     1,     6,     3,     4,     6,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     4,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       1,     1,     3,     2,     1,     5,     0,     0,    11,     0,
      12,     0,     4,     0,     7,     0,     5,     0,     3,     0,
       6,     2,     2,     4,     1,     1,     5,     3,     5,     3,
       2,     0,     2,     0,     4,     4,     3,     4,     4,     4,
       4,     1,     1,     1,     1,     3,     3,     4,     1,     2,
       4,     2,     6,     0,     1,     4,     0,     2,     0,     1,
       1,     3,     1,     3,     1,     1,     3,     3,     1,     1,
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
       3,     4,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     4,     3,     3,     3,
       1,     1,     1,     1,     3,     3,     3,     2,     0,     1,
       0,     1,     0,     5,     3,     3,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     2,     2,     4,     3,     4,     2,     0,     5,     3,
       3,     1,     3,     1,     2,     0,     5,     3,     2,     0,
       3,     0,     4,     2,     0,     3,     3,     1,     0,     1,
       1,     1,     1,     3,     1,     1,     1,     3,     1,     1,
       3,     3,     2,     4,     2,     4,     5,     5,     5,     5,
       1,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     1,     1,     1,     1,     3,     1,     4,     3,     3,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     7,     9,     7,     6,     8,     1,     4,     4,
       1,     1,     4,     2,     1,     0,     1,     1,     1,     3,
       3,     3,     0,     1,     1,     3,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     0,     5,     3,     3,     1,     2,
       0,     5,     3,     3,     1,     2,     2,     1,     2,     1,
       4,     3,     3,     6,     3,     1,     1,     1,     4,     4,
       4,     4,     4,     4,     2,     2,     4,     2,     2,     1,
       3,     3,     3,     0,     2,     5,     6,     6,     7,     1,
       2,     1,     2,     1,     4,     1,     4,     3,     0,     1,
       3,     2,     3,     1,     1,     0,     0,     2,     2,     2,
       4,     2,     5,     3,     1,     1,     0,     3,     4,     5,
       3,     1,     2,     0,     4,     1,     3,     2,     2,     2,
       1,     1,     1,     1,     3,     4,     6,     1,     8,     6,
       1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   421,     0,   784,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   874,     0,
     862,   666,     0,   672,   673,   674,    22,   731,   851,   151,
     152,   675,     0,   132,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
     388,   389,   390,   393,   392,   391,     0,     0,     0,     0,
     210,     0,     0,     0,   679,   681,   682,   676,   677,     0,
       0,     0,   683,   678,     0,   650,    24,    25,    26,    28,
      27,     0,   680,     0,     0,     0,     0,   684,   394,   519,
       0,   150,   122,     0,   667,     0,     0,     4,   111,   113,
     116,   730,     0,   649,     0,     6,   182,     7,     9,     8,
      10,     0,     0,   386,   432,     0,     0,     0,     0,     0,
       0,     0,   430,   840,   841,   501,   500,   415,   504,     0,
     414,   811,   651,   658,     0,   733,   499,   385,   814,   815,
     826,   431,     0,     0,   434,   433,   812,   813,   810,   847,
     850,   489,   732,    11,   393,   392,   391,     0,     0,    28,
       0,   111,   182,     0,   918,   431,   917,     0,   915,   914,
     503,     0,   422,   427,     0,     0,   472,   473,   474,   475,
     498,   496,   495,   494,   493,   492,   491,   490,   851,   675,
     653,     0,     0,   938,   833,   651,     0,   652,   454,     0,
     452,     0,   878,     0,   740,   413,   662,     0,   938,   661,
     656,     0,   671,   652,   857,   858,   864,   856,   663,     0,
       0,   665,   497,     0,     0,     0,     0,   418,     0,   130,
     420,     0,     0,   136,   138,     0,     0,   140,     0,    69,
      68,    63,    62,    54,    55,    46,    66,    77,     0,    49,
       0,    61,    53,    59,    79,    72,    71,    44,    67,    86,
      87,    45,    82,    42,    83,    43,    84,    41,    88,    76,
      80,    85,    73,    74,    48,    75,    78,    40,    70,    56,
      89,    64,    57,    47,    39,    38,    37,    36,    35,    34,
      58,    90,    92,    51,    32,    33,    60,   971,   972,    52,
     977,    31,    50,    81,     0,     0,   111,    91,   929,   970,
       0,   973,     0,     0,   142,     0,     0,   173,     0,     0,
       0,     0,     0,     0,    94,    99,   299,     0,     0,   298,
       0,   214,     0,   211,   304,     0,     0,     0,     0,     0,
     935,   198,   208,   870,   874,     0,   899,     0,   686,     0,
       0,     0,   897,     0,    16,     0,   115,   190,   202,   209,
     556,   531,     0,   923,   511,   513,   515,   788,   421,   432,
       0,     0,   430,   431,   433,     0,     0,   853,   668,     0,
     669,     0,     0,     0,   172,     0,     0,   118,   290,     0,
      21,   181,     0,   207,   194,   206,   391,   394,   182,   387,
     165,   166,   167,   168,   169,   171,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   862,     0,   164,   855,   855,   884,     0,     0,
       0,     0,     0,     0,     0,     0,   384,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     453,   451,   789,   790,     0,   855,     0,   802,   290,   290,
     855,     0,   870,     0,   182,     0,     0,   144,     0,   786,
     781,   740,     0,   432,   430,     0,   882,     0,   536,   739,
     873,   432,   430,   431,   118,     0,   290,   412,     0,   804,
     664,     0,   122,   250,     0,   518,     0,   147,     0,     0,
     419,     0,     0,     0,     0,     0,   139,   163,   141,   971,
     972,   968,   969,     0,   963,     0,     0,     0,     0,    65,
      30,    52,    29,   930,   170,   143,   122,     0,   160,   162,
       0,     0,    96,   103,     0,     0,    98,   107,   100,     0,
      18,     0,     0,   300,     0,   145,   213,   212,     0,     0,
     146,   919,     0,     0,   432,   430,   431,   434,   433,     0,
     956,   220,     0,   871,     0,     0,   148,     0,     0,   685,
     898,   731,     0,     0,   896,   736,   895,   114,     5,    13,
      14,     0,   218,     0,     0,   524,     0,     0,   740,     0,
       0,   659,   654,   525,     0,     0,     0,     0,   788,   122,
       0,   742,   787,   981,   411,   424,   486,   820,   839,   127,
     121,   123,   124,   125,   126,   385,     0,   502,   734,   735,
     112,   740,     0,   939,     0,     0,     0,   742,   291,     0,
     507,   184,   216,     0,   457,   459,   458,     0,     0,   455,
     456,   460,   462,   461,   477,   476,   479,   478,   480,   482,
     484,   483,   481,   471,   470,   464,   465,   463,   466,   467,
     469,   485,   468,   854,     0,     0,   888,     0,   740,   922,
       0,   921,   938,   817,   200,   192,   204,     0,   923,   196,
     182,     0,   425,   428,   436,   450,   449,   448,   447,   446,
     445,   444,   443,   442,   441,   440,   439,   792,     0,   791,
     794,   816,   798,   938,   795,     0,     0,     0,     0,     0,
       0,     0,     0,   916,   423,   779,   783,   739,   785,     0,
     655,     0,   877,     0,   876,     0,   655,   861,   860,   847,
     850,     0,     0,   791,   794,   859,   795,   416,   252,   254,
     122,   522,   521,   417,     0,   122,   234,   131,   420,     0,
       0,     0,     0,     0,   246,   246,   137,     0,     0,     0,
       0,   961,   740,     0,   945,     0,     0,     0,     0,     0,
     738,     0,   650,     0,     0,   116,   688,   649,   693,     0,
     687,   120,   692,   938,   974,     0,     0,     0,   104,     0,
      19,     0,   108,     0,    20,     0,     0,    93,   101,     0,
     297,   305,   302,     0,     0,   908,   913,   910,   909,   912,
     911,    12,   954,   955,     0,     0,     0,     0,   870,   867,
       0,   535,   907,   906,   905,     0,   901,     0,   902,   904,
       0,     5,     0,     0,     0,   550,   551,   559,   558,     0,
     430,     0,   739,   530,   534,     0,     0,   924,     0,   512,
       0,     0,   946,   788,   276,   980,     0,     0,   803,     0,
     852,   739,   941,   937,   292,   293,   648,   741,   289,     0,
     788,     0,     0,   218,   509,   186,   488,     0,   539,   540,
       0,   537,   739,   883,     0,     0,   290,   220,     0,   218,
       0,     0,   216,     0,   862,   437,     0,     0,   800,   801,
     818,   819,   848,   849,     0,     0,     0,   767,   747,   748,
     749,   756,     0,     0,     0,   760,   758,   759,   773,   740,
       0,   781,   881,   880,     0,     0,   805,   670,     0,   256,
       0,     0,   128,     0,     0,     0,     0,     0,     0,     0,
     226,   227,   238,     0,   122,   236,   157,   246,     0,   246,
       0,     0,   975,     0,     0,     0,   739,   962,   964,   944,
     740,   943,     0,   740,   714,   715,   712,   713,   746,     0,
     740,   738,     0,   533,     0,     0,   890,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   967,   174,     0,   177,   161,
       0,    95,   105,     0,    97,   109,   102,   301,     0,   920,
     149,   956,   936,   951,   219,   221,   311,     0,     0,   868,
       0,   900,     0,    17,     0,   923,   217,   311,     0,     0,
     655,   527,     0,   660,   925,     0,   946,   516,     0,     0,
     981,     0,   281,   279,   794,   806,   938,   794,   807,   940,
       0,     0,   294,   119,     0,   788,   215,     0,   788,     0,
     487,   887,   886,     0,   290,     0,     0,     0,     0,     0,
       0,   218,   188,   671,   793,   290,     0,   752,   753,   754,
     755,   761,   762,   771,     0,   740,     0,   767,     0,   751,
     775,   739,   778,   780,   782,     0,   875,   793,     0,     0,
       0,     0,   253,   523,   133,     0,   420,   226,   228,   870,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   240,
       0,     0,   957,     0,   960,   739,     0,     0,     0,   690,
     739,   737,     0,   728,     0,   740,     0,   694,   729,   727,
     894,     0,   740,   697,   699,   698,     0,     0,   695,   696,
     700,   702,   701,   717,   716,   719,   718,   720,   722,   724,
     723,   721,   710,   709,   704,   705,   703,   706,   707,   708,
     711,   966,     0,   122,   106,   110,   303,     0,     0,     0,
     953,     0,   385,   872,   870,   426,   429,   435,     0,    15,
       0,   385,   562,     0,     0,   564,   557,   560,     0,   555,
       0,   927,     0,   947,   520,     0,   282,     0,     0,   277,
       0,   296,   295,   946,     0,   311,     0,   788,     0,   290,
       0,   845,   311,   923,   311,   926,     0,     0,     0,   438,
       0,     0,   764,   739,   766,   757,     0,   750,     0,     0,
     740,   772,   879,     0,   122,     0,   249,   235,     0,     0,
       0,   225,   153,   239,     0,     0,   242,     0,   247,   248,
     122,   241,   976,   958,     0,   942,     0,   979,   745,   744,
     689,     0,   739,   532,   691,     0,   538,   739,   889,   726,
       0,     0,     0,   950,   948,   949,   222,     0,     0,     0,
     392,   383,     0,     0,     0,   199,   310,   312,     0,   382,
       0,     0,     0,   923,   385,     0,   903,   307,   203,   553,
       0,     0,   526,   514,     0,   285,   275,     0,   278,   284,
     290,   506,   946,   385,   946,     0,   885,     0,   844,   385,
       0,   385,   928,   311,   788,   842,   770,   769,   763,     0,
     765,   739,   774,   122,   255,   129,   134,   155,   229,     0,
     237,   243,   122,   245,   959,     0,     0,   529,     0,   893,
     892,   725,   122,   178,   952,     0,     0,     0,   931,     0,
       0,     0,   223,     0,   923,     0,   348,   344,   350,   650,
      28,     0,   338,     0,   343,   347,   360,     0,   358,   363,
       0,   362,     0,   361,     0,   182,   314,     0,   316,     0,
     317,   318,     0,     0,   869,     0,   554,   552,   563,   561,
     286,     0,     0,   273,   283,     0,     0,     0,     0,   195,
     506,   946,   846,   201,   307,   205,   385,     0,     0,   777,
       0,   251,     0,     0,   122,   232,   154,   244,   978,   743,
       0,     0,     0,     0,     0,   410,     0,   932,     0,   328,
     332,   407,   408,   342,     0,     0,     0,   323,   614,   613,
     610,   612,   611,   631,   633,   632,   602,   573,   574,   592,
     608,   607,   569,   579,   580,   582,   581,   601,   585,   583,
     584,   586,   587,   588,   589,   590,   591,   593,   594,   595,
     596,   597,   598,   600,   599,   570,   571,   572,   575,   576,
     578,   616,   617,   626,   625,   624,   623,   622,   621,   609,
     628,   618,   619,   620,   603,   604,   605,   606,   629,   630,
     634,   636,   635,   637,   638,   615,   640,   639,   642,   644,
     643,   577,   647,   645,   646,   641,   627,   568,   355,   565,
       0,   324,   376,   377,   375,   368,     0,   369,   325,   402,
       0,     0,     0,     0,   406,     0,   182,   191,   306,     0,
       0,     0,   274,   288,   843,     0,   122,   378,   122,   185,
       0,     0,     0,   197,   946,   768,     0,   122,   230,   135,
     156,     0,   528,   891,   176,   326,   327,   405,   224,     0,
       0,   740,     0,   351,   339,     0,     0,     0,   357,   359,
       0,     0,   364,   371,   372,   370,     0,     0,   313,   933,
       0,     0,     0,   409,     0,   308,     0,   287,     0,   548,
     742,     0,     0,   122,   187,   193,     0,   776,     0,     0,
     158,   329,   111,     0,   330,   331,     0,     0,   345,   739,
     353,   349,   354,   566,   567,     0,   340,   373,   374,   366,
     367,   365,   403,   400,   956,   319,   315,   404,     0,   309,
     549,   741,     0,   508,   379,     0,   189,     0,   233,     0,
     180,     0,   385,     0,   352,   356,     0,     0,   788,   321,
       0,   546,   505,   510,   231,     0,     0,   159,   336,     0,
     384,   346,   401,   934,     0,   742,   396,   788,   547,     0,
     179,     0,     0,   335,   946,   788,   260,   397,   398,   399,
     981,   395,     0,     0,     0,   334,     0,   396,     0,   946,
       0,   333,   380,   122,   320,   981,     0,   265,   263,     0,
     122,     0,     0,   266,     0,     0,   261,   322,     0,   381,
       0,   269,   259,     0,   262,   268,   175,   270,     0,     0,
     257,   267,     0,   258,   272,   271
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   851,   598,   171,  1398,   692,
     333,   551,   555,   334,   552,   556,   109,   110,   111,   112,
     113,   114,   385,   630,   631,   519,   238,  1462,   525,  1379,
    1463,  1700,   807,   328,   546,  1660,  1029,  1203,  1717,   402,
     172,   632,   891,  1089,  1258,   118,   601,   908,   633,   652,
     912,   581,   907,   634,   602,   909,   404,   351,   368,   121,
     893,   854,   837,  1044,  1401,  1141,   960,  1609,  1466,   767,
     966,   524,   776,   968,  1290,   759,   949,   952,  1130,  1724,
    1725,   620,   621,   646,   647,   338,   339,   345,  1435,  1588,
    1589,  1212,  1326,  1424,  1582,  1708,  1727,  1619,  1664,  1665,
    1666,  1411,  1412,  1413,  1414,  1621,  1622,  1628,  1676,  1417,
    1418,  1422,  1575,  1576,  1577,  1599,  1754,  1327,  1328,   173,
     123,  1740,  1741,  1580,  1330,  1331,  1332,  1333,   124,   231,
     520,   521,   125,   126,   127,   128,   129,   130,   131,   132,
    1447,   133,   890,  1088,   134,   617,   618,   619,   235,   377,
     515,   607,   608,  1165,   609,  1166,   135,   136,   137,   798,
     138,   139,  1650,   140,   603,  1437,   604,  1058,   859,  1229,
    1226,  1568,  1569,   141,   142,   143,   221,   144,   222,   232,
     389,   507,   145,   988,   802,   146,   989,   882,   874,   990,
     936,  1111,   937,  1113,  1114,  1115,   939,  1269,  1270,   940,
     736,   491,   184,   185,   635,   623,   474,  1074,  1075,   722,
     723,   878,   148,   224,   149,   150,   175,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   684,   228,   229,   584,
     213,   214,   687,   688,  1171,  1172,   361,   362,   845,   161,
     572,   162,   616,   163,   320,  1590,  1640,   352,   397,   641,
     642,   982,  1069,  1210,   834,   835,   781,   782,   783,   321,
     322,   804,  1400,   876
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1251
static const yytype_int16 yypact[] =
{
   -1251,   160, -1251, -1251,  5500, 13220, 13220,    -8, 13220, 13220,
   13220, 11097, 13220, -1251, 13220, 13220, 13220, 13220, 13220, 13220,
   13220, 13220, 13220, 13220, 13220, 13220, 15536, 15536, 11290, 13220,
   14459,    14,    18, -1251, -1251, -1251, -1251, -1251,   187, -1251,
   -1251,   244, 13220, -1251,    18,   175,   177,   215, -1251,    18,
   11483,  1639, 11676, -1251, 14122, 10132,   191, 13220,  1128,    24,
   -1251, -1251, -1251,   452,    85,    66,   219,   221,   350,   360,
   -1251,  1639,   363,   403, -1251, -1251, -1251, -1251, -1251, 13220,
     506,  1029, -1251, -1251,  1639, -1251, -1251, -1251, -1251,  1639,
   -1251,  1639, -1251,   220,   414,  1639,  1639, -1251,   401, -1251,
   11869, -1251, -1251,   311,   458,   513,   513, -1251,   557,   457,
      10, -1251,   462, -1251,    92, -1251,   596, -1251, -1251, -1251,
   -1251,  1104,  1161, -1251, -1251,   491,   493,   503,   510,   515,
     524,  4839, -1251, -1251, -1251, -1251,    90, -1251,   571,   583,
   -1251,    40,   533, -1251,   578,     7, -1251,  2367,   152, -1251,
   -1251,  3102,   146,   549,   107, -1251,   151,   208,   559,   211,
   -1251, -1251,   688, -1251, -1251, -1251,   608,   576,   609, -1251,
   13220, -1251,   596,  1161, 16235,  3879, 16235, 13220, 16235, 16235,
    5000,   592, 15136,  5000,   721,  1639,   703,   703,   573,   703,
     703,   703,   703,   703,   703,   703,   703,   703, -1251, -1251,
   -1251,   418, 13220,   616, -1251, -1251,   660,   607,   314,   635,
     314, 15536, 15697,   629,   816, -1251,   608, 13220,   616,   675,
   -1251,   678,   649, -1251,   155, -1251, -1251, -1251,   314,   146,
   12062, -1251, -1251, 13220,  8781,   828,    93, 16235,  9746, -1251,
   13220, 13220,  1639, -1251, -1251,  4885,   651, -1251,  4962, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,  2288, -1251,
    2288, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251,    83,    89,   609,
   -1251, -1251, -1251, -1251,   661,   957,    91, -1251, -1251,   695,
     835, -1251,   701, 14703, -1251,   662, 11082, -1251,    16, 11468,
    1573,  1912,  1639,    99, -1251,    47, -1251, 15160,   102, -1251,
     725, -1251,   727, -1251,   841,   103, 15536, 13220, 13220,   670,
     692, -1251, -1251, 15253, 11290,   106,    77,   465, -1251, 13413,
   15536,   546, -1251,  1639, -1251,   353,   457, -1251, -1251, -1251,
   -1251, 16029,   846,   768, -1251, -1251, -1251,   105, 13220,   683,
     685, 16235,   686,  2360,   687,  5693, 13220, -1251,   413,   677,
     538,   413,   316,   497, -1251,  1639,  2288,   690, 10325, 14122,
   -1251, -1251,   682, -1251, -1251, -1251, -1251, -1251,   596, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, 13220, 13220, 13220, 12255,
   13220, 13220, 13220, 13220, 13220, 13220, 13220, 13220, 13220, 13220,
   13220, 13220, 13220, 13220, 13220, 13220, 13220, 13220, 13220, 13220,
   13220, 13220, 14459, 13220, -1251, 13220, 13220, 13220, 13575,  1639,
    1639,  1639,  1639,  1639,  1104,   770,  1442,  9939, 13220, 13220,
   13220, 13220, 13220, 13220, 13220, 13220, 13220, 13220, 13220, 13220,
   -1251, -1251, -1251, -1251,   565, 13220, 13220, -1251, 10325, 10325,
   13220, 13220, 15253,   705,   596, 12448, 12047, -1251, 13220, -1251,
     707,   887,   751,   716,   723, 13708,   314, 12641, -1251, 12834,
   -1251,   724,   726,  2636, -1251,    42, 10325, -1251,   820, -1251,
   -1251, 13205, -1251, -1251, 10518, -1251, 13220, -1251,   818,  8974,
     903,   729, 16190,   901,    70,    63, -1251, -1251, -1251,   753,
   -1251, -1251, -1251,  2288,  1339,   741,   920, 15067,  1639, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251,   745, -1251, -1251,
    1639,   109, -1251,    55,  1639,   110, -1251,    65,    69,  1954,
   -1251,  1639, 13220,   314,    24, -1251, -1251, -1251, 15067,   852,
   -1251,   314,    72,    97,   746,   748,  3089,   281,   755,   757,
     619,   829,   762,   314,   126,   771, -1251,  1250,  1639, -1251,
   -1251,   892,  2582,   390, -1251, -1251, -1251,   457, -1251, -1251,
   -1251,   930,   832,   793,   199,   814, 13220,   834,   958,   781,
     819, -1251,   163, -1251,  2288,  2288,   960,   828,   105, -1251,
     787,   963, -1251,  2288,   445, -1251,   451,   164, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251,  2058,  2753, -1251, -1251, -1251,
   -1251,   968,   812, -1251, 15536, 13220,   800,   979, 16235,   976,
   -1251, -1251,   865,   911, 11661, 16406,  5000, 13220, 13398, 16554,
    3700, 10305,  3574,  2861, 11268, 11268, 11268, 11268,  3215,  3215,
    3215,  3215,  3215,  1247,  1247,   680,   680,   680,   573,   573,
     573, -1251,   703, 16235,   804,   805, 15742,   810,   994,   -22,
   13220,   -17,   616,   182, -1251, -1251, -1251,   992,   768, -1251,
     596, 15350, -1251, -1251,  5000,  5000,  5000,  5000,  5000,  5000,
    5000,  5000,  5000,  5000,  5000,  5000,  5000, -1251, 13220,   333,
   -1251,   167, -1251,   616,   444,   821,  2805,   825,   833,   822,
    2974,   128,   838, -1251, 16235,  2598, -1251,  1639, -1251,   445,
     511, 15536, 16235, 15536, 15799,   445,   314,   171, -1251,   163,
     868,   839, 13220, -1251,   203, -1251, -1251, -1251,  8588,   602,
   -1251, -1251, 16235, 16235,    18, -1251, -1251, -1251, 13220,   923,
    4440, 15067,  1639,  9167,   837,   842, -1251,    75,   940,   897,
     881, -1251,  1024,   847,  1323,  2288, 15067, 15067, 15067, 15067,
   15067,   850,   886,   854, 15067,     3, -1251,   890, -1251,   857,
   -1251, 16323, -1251,   207, -1251,  5886,  1293,   858,   436,  1573,
   -1251,  1639,   450,  1912, -1251,  1639,  1639, -1251, -1251,  3036,
   -1251, 16323,  1034, 15536,   855, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251,    78,  1639,  1293,   862, 15253, 15443,
    1039, -1251, -1251, -1251, -1251,   859, -1251, 13220, -1251, -1251,
    5108, -1251,  2288,  1293,   866, -1251, -1251, -1251, -1251,  1040,
     870, 13220, 16029, -1251, -1251, 13575,   871, -1251,  2288, -1251,
     874,  6079,  1031,   143, -1251, -1251,   104,   565, -1251,   820,
   -1251,  2288, -1251, -1251,   314, 16235, -1251, 10711, -1251, 15067,
      87,   876,  1293,   832, -1251, -1251,  5044, 13220, -1251, -1251,
   13220, -1251, 13220, -1251,  3292,   877, 10325,   829,  1037,   832,
    2288,  1061,   865,  1639, 14459,   314,  3452,   891, -1251, -1251,
     180,   893, -1251, -1251,  1064,   700,   700,  2598, -1251, -1251,
   -1251,  1032,   894,   437,   900, -1251, -1251, -1251, -1251,  1081,
     906,   707,   314,   314, 13027,   820, -1251, -1251,  3542,   611,
      18,  9746, -1251,  6272,   907,  6465,   908,  4440, 15536,   912,
     962,   314, 16323,  1089, -1251, -1251, -1251, -1251,   520, -1251,
     282,  2288, -1251,   972,  2288,  1639,  1339, -1251, -1251, -1251,
    1101, -1251,   924,   968,   699,   699,  1048,  1048, 15901,   928,
    1108, 15067, 14969, 16029,  3807, 14836, 15067, 15067, 15067, 15067,
    4157, 15067, 15067, 15067, 15067, 15067, 15067, 15067, 15067, 15067,
   15067, 15067, 15067, 15067, 15067, 15067, 15067, 15067, 15067, 15067,
   15067, 15067, 15067, 15067,  1639, -1251, -1251,  1042, -1251, -1251,
    1639, -1251, -1251,  1639, -1251, -1251, -1251, -1251, 15067,   314,
   -1251,   619, -1251,   654,  1114, -1251, -1251,   131,   938,   314,
   10904, -1251,  2240, -1251,  5307,   768,  1114, -1251,   439,   358,
   -1251, 16235,   995,   943, -1251,   942,  1031, -1251,  2288,   828,
    2288,    56,  1112,  1052,   206, -1251,   616,   308, -1251, -1251,
   15536, 13220, 16235, 16323,   947,    87, -1251,   946,    87,   952,
    5044, 16235, 15844,   953, 10325,   955,   951,  2288,   956,   964,
    2288,   832, -1251,   649,   472, 10325, 13220, -1251, -1251, -1251,
   -1251, -1251, -1251,  1012,   970,  1139,  1063,  2598,  1013, -1251,
   16029,  2598, -1251, -1251, -1251, 15536, 16235, -1251,    18,  1134,
    1091,  9746, -1251, -1251, -1251,   978, 13220,   962,   314, 15253,
    4440,   981, 15067,  6658,   604,   985, 13220,   119,   301, -1251,
     996,  2288, -1251,  1043, -1251,  2073,  1144,   989, 15067, -1251,
   15067, -1251,   993, -1251,  1047,  1172,   999, -1251, -1251, -1251,
   15946,  1004,  1174, 16365, 16447, 16483, 15067, 16280, 16589, 13576,
   10691,  4309,  3099, 12426, 12426, 12426, 12426,  3244,  3244,  3244,
    3244,  3244,  1463,  1463,   699,   699,   699,  1048,  1048,  1048,
    1048, -1251,  1002, -1251, -1251, -1251, 16323,  1639,  2288,  2288,
   -1251,  1293,   124, -1251, 15253, -1251, -1251,  5000,  1008, -1251,
    1011,  1038, -1251,   332, 13220, -1251, -1251, -1251, 13220, -1251,
   13220, -1251,   828, -1251, -1251,   136,  1188,  1121, 13220, -1251,
    1018,   314, 16235,  1031,  1023, -1251,  1028,    87, 13220, 10325,
    1035, -1251, -1251,   768, -1251, -1251,  1017,  1022,  1030, -1251,
    1044,  2598, -1251,  2598, -1251, -1251,  1045, -1251,  1092,  1050,
    1212, -1251,   314,  1196, -1251,  1046, -1251, -1251,  1054,  1060,
     134, -1251, -1251, 16323,  1051,  1065, -1251,  4712, -1251, -1251,
   -1251, -1251, -1251, -1251,  2288, -1251,  2288, -1251, 16323, 16003,
   -1251, 15067, 16029, -1251, -1251, 15067, -1251, 15067, -1251, 16519,
   15067,  1041,  6851,   654, -1251, -1251, -1251,   572, 14260,  1293,
    1141, -1251,   581,  1094,  1809, -1251, -1251, -1251,   770,  2007,
     111,   112,  1067,   768,  1442,   139, -1251, -1251, -1251,  1096,
    3880,  4113, 16235, -1251,    58,  1243,  1178, 13220, -1251, 16235,
   10325,  1158,  1031,  1238,  1031,  1085, 16235,  1086, -1251,  1491,
    1093,  1675, -1251, -1251,    87, -1251, -1251,  1156, -1251,  2598,
   -1251, 16029, -1251, -1251,  8588, -1251, -1251, -1251, -1251,  9360,
   -1251, -1251, -1251,  8588, -1251,  1105, 15067, 16323,  1170, 16323,
   16048, 16519, -1251, -1251, -1251,  1293,  1293,  1639, -1251,  1279,
   14969,    79, -1251, 14260,   768,  1825, -1251,  1133, -1251,   114,
    1118,   115, -1251, 14565, -1251, -1251, -1251,   116, -1251, -1251,
    1472, -1251,  1120, -1251,  1230,   596, -1251, 14398, -1251, 14398,
   -1251, -1251,  1301,   770, -1251, 13846, -1251, -1251, -1251, -1251,
    1302,  1240, 13220, -1251, 16235,  1135,  1137,  1132,   529, -1251,
    1158,  1031, -1251, -1251, -1251, -1251,  1886,  1140,  2598, -1251,
    1197,  8588,  9553,  9360, -1251, -1251, -1251,  8588, -1251, 16323,
   15067, 15067,  7044,  1145,  1147, -1251, 15067, -1251,  1293, -1251,
   -1251, -1251, -1251, -1251,  2288,  1538,   581, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,   138, -1251,
    1094, -1251, -1251, -1251, -1251, -1251,   123,   130, -1251,  1322,
     117, 14703,  1230,  1325, -1251,  2288,   596, -1251, -1251,  1152,
    1330, 13220, -1251, 16235, -1251,   329, -1251, -1251, -1251, -1251,
    1176,   529, 13984, -1251,  1031, -1251,  2598, -1251, -1251, -1251,
   -1251,  7237, 16323, 16323, -1251, -1251, -1251, 16323, -1251,   642,
      80,  1338,  1160, -1251, -1251, 15067, 14565, 14565,  1307, -1251,
    1472,  1472,   643, -1251, -1251, -1251, 15067,  1286, -1251,  1200,
    1183,   120, 15067, -1251,  1639, -1251, 15067, 16235,  1294, -1251,
    1366,  7430,  7623, -1251, -1251, -1251,   529, -1251,  7816,  1189,
    1271, -1251,  1285,  1233, -1251, -1251,  1287,  2288, -1251,  1538,
   -1251, -1251, 16323, -1251, -1251,  1224, -1251,  1356, -1251, -1251,
   -1251, -1251, 16323,  1380,   619, -1251, -1251, 16323,  1209, 16323,
   -1251,   330,  1215, -1251, -1251,  8009, -1251,  1217, -1251,  1228,
    1246,  1639,  1442,  1248, -1251, -1251, 15067,   122,   141, -1251,
    1336, -1251, -1251, -1251, -1251,  1293,   858, -1251,  1253,  1639,
    1521, -1251, 16323, -1251,  1236,  1415,   594,   141, -1251,  1349,
   -1251,  1293,  1244, -1251,  1031,   145, -1251, -1251, -1251, -1251,
    2288, -1251,  1254,  1255,   121, -1251,   595,   594,   166,  1031,
    1242, -1251, -1251, -1251, -1251,  2288,    62,  1420,  1355,   595,
   -1251,  8202,   174,  1426,  1363, 13220, -1251, -1251,  8395, -1251,
      96,  1430,  1373, 13220, -1251, 16235, -1251,  1432,  1382, 13220,
   -1251, 16235, 13220, -1251, 16235, 16235
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1251, -1251, -1251,  -529, -1251, -1251, -1251,   435,     0,   -27,
   -1251, -1251, -1251,   905,   658,   647,    15,  1567,  3483, -1251,
    2656, -1251,  -436, -1251,    54, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251, -1251, -1251, -1251, -1251,  -247, -1251, -1251,  -150,
     194,    28, -1251, -1251, -1251, -1251, -1251, -1251,    29, -1251,
   -1251, -1251, -1251,    31, -1251, -1251,  1016,  1027,  1020,   -92,
     580,  -815,   570,   627,  -248,   366,  -874, -1251,    41, -1251,
   -1251, -1251, -1251,  -705,   218, -1251, -1251, -1251, -1251,  -220,
   -1251,  -533, -1251,  -416, -1251, -1251,   944, -1251,    64, -1251,
   -1251,  -989, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251,    34, -1251,   125, -1251, -1251, -1251, -1251, -1251,
     -43, -1251,   204,  -840, -1251, -1250,  -228, -1251,  -139,   113,
    -120,  -215, -1251,   -48, -1251, -1251, -1251,   216,   -29,    -3,
      30,  -693,   -75, -1251, -1251,   -11, -1251, -1251,    -5,   -36,
      88, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251, -1251,
    -556,  -803, -1251, -1251, -1251, -1251, -1251,  1187, -1251, -1251,
   -1251, -1251, -1251,   477, -1251, -1251, -1251, -1251, -1251, -1251,
   -1251, -1251,  -831, -1251,  2391,    27, -1251,  1377,  -377, -1251,
   -1251,   438,  3428,  3682, -1251, -1251,   548,  -171,  -619, -1251,
   -1251,   617,   426,  -644,   428, -1251, -1251, -1251, -1251, -1251,
     605, -1251, -1251, -1251,    17,  -833,  -142,  -394,  -392, -1251,
     672,  -116, -1251, -1251,    36,    37,   568, -1251, -1251,   179,
     -23, -1251,  -333,     9,  -311,    59,   190, -1251, -1251,  -440,
    1199, -1251, -1251, -1251, -1251, -1251,   667,   425, -1251, -1251,
   -1251,  -322,  -647, -1251,  1165,  -868, -1251,   -65,  -170,    23,
     767, -1251,  -794,   242,  -119,   526,   597, -1251, -1251, -1251,
   -1251,   551,   393, -1036
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -966
static const yytype_int16 yytable[] =
{
     174,   176,   409,   178,   179,   180,   182,   183,   455,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   147,   483,   212,   215,   380,   369,   318,   888,   234,
     372,   373,   117,   119,  1235,   120,   477,   237,   611,   227,
    1070,   239,   731,   500,   325,   245,   243,   248,   504,   613,
     326,   911,   329,   409,   317,   454,   405,   220,   115,  1062,
     612,   869,   727,   728,   382,   681,   225,   226,  1221,   850,
     970,   379,   236,   335,   237,   956,   758,   384,  1087,   772,
     720,   823,   721,  1137,   971,   870,   508,  1041,  1478,  1667,
     751,   938,   -65,   774,  1098,   381,   365,   -65,   -30,   366,
     -29,   399,   516,   -30,   336,   -29,   516,  -821,   559,   355,
     805,   564,   569,   509,   754,   516,   755,   122,   809,   813,
    1427,  1429,   198,  -341,  1486,  1570,  1637,   547,  1317,  1637,
    1478,  1041,  1630,    13,   382,   839,  1236,   839,  1440,  1071,
     839,   379,  1763,   839,   587,   561,   344,   384,   839,  1288,
     -92,    13,  1625,   811,  -938,   -91,  1631,  -541,  -652,  1633,
       3,  -938,   387,   815,   -92,   381,   494,   816,  1626,   -91,
      13,  1344,   486,   493,  -543,   396,  1777,  1634,   177,   384,
    1635,   502,   396,   871,  1072,   548,  1627,    13,   501,    13,
    1164,    13,  -938,  -834,   749,  -517,  -938,   381,   116,  -938,
     230,  1756,   395,   342,   233,   209,   209,   472,   473,  1770,
     395,   343,   381,   475,   588,   337,  1345,  -653,  -822,  -824,
     395,  1237,  -863,  1441,   395,   511,  -832,  1764,   511,  -821,
     480,  -828,  1318,   103,  -825,   237,   522,  1319,  -866,    60,
      61,    62,   164,  1320,   406,  1321,  1757,  -827,   972,  -829,
     856,  1042,   775,  1668,  1771,   533,  1353,   773,   653,   824,
     456,  1778,  1144,  1359,  1148,  1361,  1281,  1479,  1480,  1073,
    -865,   -65,  1232,  -808,  -280,  -823,  -833,   -30,   480,   -29,
     400,   517,  1322,  1323,   825,  1324,  1257,   560,   513,  -280,
     565,   570,   518,   479,   586,  1723,   543,   810,   814,  1428,
    1430,  1346,  -341,  1487,  1571,  1638,   407,  1289,  1686,  1751,
    1632,  -544,   575,   840,  1325,   924,   408,  1268,  1213,   574,
     738,  1378,  1054,   317,   953,   578,  1434,  -741,  -264,   955,
    -741,  1758,  -741,  -660,   732,   476,   409,   651,  -831,  1772,
    -822,  -824,   237,   381,  -863,   553,   557,   558,  -543,   212,
    -835,  1654,   481,  -828,   592,   857,  -825,  1084,  1024,  -659,
    -866,   240,  -654,   241,  1648,  1710,  -838,   484,   906,  -827,
     858,  -829,   318,   182,  1456,  -809,   370,   573,   597,   396,
     327,   636,   702,  1339,  1146,  1147,   369,   697,   698,   405,
     209,   198,  -865,   648,   622,  -808,   356,  -823,  1047,   317,
     481,   242,   594,  1146,  1147,   346,  1696,   347,  1220,  1649,
    1711,   654,   655,   656,   658,   659,   660,   661,   662,   663,
     664,   665,   666,   667,   668,   669,   670,   671,   672,   673,
     674,   675,   676,   677,   678,   679,   680,   863,   682,   108,
     683,   683,   686,  1278,   472,   473,   703,   323,   691,  1351,
    1399,   227,   704,   705,   706,   707,   708,   709,   710,   711,
     712,   713,   714,   715,   716,   359,   360,   479,   829,   220,
     683,   726,  1149,   648,   648,   683,   730,  1271,   225,   226,
     704,   374,   877,   734,   879,  1077,   246,  1078,   370,   316,
    1095,  1291,   742,   356,   744,   317,   455,  -809,   122,  1388,
     386,   648,   103,   761,   492,  -796,   350,   693,   395,   762,
     638,   763,   475,  1234,   748,  1227,   209,   903,  -545,  -796,
     472,   473,   905,  1118,   367,   209,   350,   577,  1143,   611,
     350,   350,   209,   724,  1030,  1481,   348,  1103,   356,   209,
     613,   599,   600,   454,   388,   356,   349,  1228,  1033,   353,
     913,   612,  1244,   917,   693,  1246,   350,   819,  1448,  1583,
    1450,  1584,   359,   360,   750,   808,   358,   756,  1460,   812,
     860,   340,   151,   766,   335,   472,   473,   356,   341,   116,
     849,   472,   473,   594,   375,  1119,   356,   394,   895,   354,
     376,   395,   357,   356,   208,   210,  1222,   877,   879,   391,
     371,   381,  -655,   945,   879,   395,  1360,   359,   360,  1223,
    1405,   977,   395,   589,   359,   360,  -799,  1366,   356,  1367,
     490,  1145,  1146,  1147,   594,   504,   356,   442,  1224,   946,
    -799,   401,   594,  1025,   476,   622,   685,  -836,   445,   443,
     885,   472,   473,    36,  -797,   198,   359,   360,   398,   700,
     446,   531,   896,   532,   358,   359,   360,  1601,  -797,    36,
     611,   209,   359,   360,    48,   725,   832,   833,   383,   108,
     729,   613,  1679,   108,   950,   951,  1343,   523,  1250,   410,
      48,   411,   612,  1128,  1129,   904,  1432,   359,   360,  1260,
    1680,   412,   639,  1681,   595,   359,   360,  -836,   413,  1280,
    1395,  1396,  1406,   414,  1748,  1285,  1146,  1147,   536,  1737,
    1738,  1739,   415,   916,  1355,  1407,  1408,  1597,  1598,  1762,
      36,   717,   447,    86,    87,  1459,    88,   169,    90,   448,
     439,   440,   441,   168,   442,   478,    84,  1409,   383,    86,
      87,    48,    88,  1410,    90,  -830,   443,   948,   456,  1020,
    1021,  1022,  1208,  1209,   718,  -542,   103,  1482,   542,  -653,
      36,   954,   482,   237,   363,  1023,   489,  1312,  1122,   443,
     383,   390,   392,   393,  1335,  1107,  1108,  1109,    36,   496,
     487,    48,   590,  1752,  1753,   503,   596,   611,   396,   643,
    1677,  1678,   323,  -834,   168,  1673,  1674,    84,   613,    48,
      86,    87,   151,    88,   169,    90,   151,   980,   983,   612,
    1656,   495,  1157,   590,  1605,   596,   590,   596,   596,  1161,
     108,   479,   498,   209,   553,   499,  -651,   965,   557,   505,
     640,  1457,  1661,  1357,   316,   506,   514,   350,  1374,   527,
      86,    87,  1052,    88,   169,    90,  -965,   534,  1102,   537,
     544,   566,   538,   567,  1383,   568,  1061,   579,    86,    87,
     614,    88,   169,    90,   580,   691,   615,   147,   650,   637,
     624,   122,   625,   626,   628,  1726,  -117,    53,   117,   119,
     209,   120,  1082,   542,   350,   695,   350,   350,   350,   350,
     622,   650,  1090,   735,  1726,  1091,   737,  1092,    36,   589,
     198,   648,  1747,   739,   115,   563,  1240,   622,   764,   719,
     740,   745,   516,   746,   571,   771,   576,   768,   122,    48,
     209,   583,   209,   227,  1063,   533,   777,   784,   593,   785,
     542,   806,   822,   826,  1445,   827,   724,  1461,   756,  1126,
    1746,   220,   830,   753,  1264,   831,  1467,  1131,   838,   209,
     225,   226,   116,   151,   108,  1759,  1472,   836,   841,   847,
     852,   853,  1657,   122,   855,  -675,   861,   862,   864,   611,
     865,   872,   873,   803,   868,  1215,   717,   881,    86,    87,
     613,    88,   169,    90,   122,   883,   258,   886,   887,    36,
     889,   612,  1163,   892,  1303,  1169,   818,   898,   899,   116,
     901,  1308,   209,   902,   756,  1132,   910,   866,   867,   752,
      48,   103,   920,   260,   918,   922,   875,   209,   209,  -657,
     921,   957,   844,   846,   894,   967,   947,   973,  1611,   974,
     969,  1692,   975,   976,   978,    36,   991,   992,   611,  1216,
     993,   995,  1317,  1040,   116,  1217,   996,  1028,  1038,   613,
     583,  1046,  1051,  1050,  1059,  1057,    48,  1060,  1066,  1064,
     612,  1068,  1085,  1094,   535,   116,   122,  1097,   122,    86,
      87,   147,    88,   169,    90,  1100,  1242,  1105,  1106,  -837,
    1117,  1116,   117,   119,    13,   120,  1120,   151,   350,   648,
    1121,   529,   530,  1123,  1140,  1134,  1136,   894,  1139,  1372,
     648,  1217,   622,  1142,  1151,   622,  1736,    36,   115,   168,
    1155,  1156,    84,   310,  1023,    86,    87,  1160,    88,   169,
      90,  1159,  1202,  1211,  1214,  1273,  1238,  1230,    48,   906,
    1231,   237,  1239,   314,  1243,  1245,   209,   209,  1247,  1249,
    1252,  1287,  1251,   315,  1261,  1254,  1318,   116,  1263,   116,
     931,  1319,  1255,    60,    61,    62,   164,  1320,   406,  1321,
    1651,  1267,  1652,  1262,  1274,  1275,  1277,   122,  1282,  1292,
     935,  1658,   941,  1286,  1296,  1294,  1297,   643,   643,  1301,
    1300,  1302,    36,  1307,   363,  1276,  1304,    86,    87,  1311,
      88,   169,    90,   108,  1306,  1433,  1322,  1323,  1336,  1324,
    1337,  1348,  1347,    48,  1350,  1362,    36,   963,   108,   409,
    1352,  1363,   884,   204,   204,  1354,  1364,  1695,   364,  1340,
     407,  1371,  1358,  1341,  1369,  1342,  1373,    48,  1338,  1334,
    1392,  1365,  1368,  1349,  1375,   330,   331,  1370,  1334,  1380,
     108,  1376,  1317,  1356,   648,  1055,  1032,  1377,   116,  1403,
    1035,  1036,  1436,  1381,  1416,  1431,   122,  1442,  1443,   209,
     403,  1065,    86,    87,   622,    88,   169,    90,    53,   915,
    1043,  1446,  1451,  1452,  1079,  1581,    60,    61,    62,   164,
     165,   406,  1454,   332,    13,   108,    86,    87,  1458,    88,
     169,    90,  1468,  1476,   436,   437,   438,   439,   440,   441,
     542,   442,  1470,  1099,   209,  1484,   108,  1485,  1578,   942,
    1579,   943,   719,   443,   753,  1585,  1591,  1761,   209,   209,
    1592,  1596,  1594,  1595,  1768,  1329,   151,  1604,    36,  1606,
     842,   843,  1475,  1615,  1329,  1616,  1636,   116,   961,  1642,
    1645,   151,  1444,   407,  1646,   648,  1318,  1669,   350,    48,
    1671,  1319,   258,    60,    61,    62,   164,  1320,   406,  1321,
    1110,  1110,   935,  1675,  1150,  1653,  1683,  1152,   778,  1685,
    1334,    36,  1684,   151,  1690,  1691,  1334,  1698,  1334,   260,
     753,   622,  1699,  -337,  1701,  1702,   108,  1705,   108,  1631,
     108,  1039,    48,   209,  1706,  1709,  1322,  1323,   204,  1324,
    1477,    36,  1712,   206,   206,  1714,   583,  1049,    86,    87,
    1153,    88,   169,    90,  1715,  1716,  1728,    36,   151,  1731,
     407,  1721,    48,  1734,  1735,   122,   779,   542,  1449,  1743,
     542,  1760,  1745,  1465,  1765,  1766,  1644,  1593,    48,   151,
    1773,  1749,  1750,  1774,  1779,   168,  1782,   456,    84,    85,
    1670,    86,    87,  1780,    88,   169,    90,   529,   530,   803,
    1034,  1233,  1783,   875,   817,  1204,  1329,  1031,  1205,  1730,
     699,   696,  1329,  1334,  1329,   168,   694,  1096,    84,   310,
    1056,    86,    87,  1744,    88,   169,    90,   122,   979,   108,
    1253,   168,  1101,  1256,    84,  1317,   122,    86,    87,   314,
      88,   169,    90,  1279,  1610,  1382,   116,  1742,   820,   315,
    1017,  1018,  1019,  1020,  1021,  1022,  1608,  1465,  1602,   151,
    1624,   151,  1425,   151,   204,   961,  1138,  1629,  1423,  1023,
    1483,  1767,  1755,   204,  1641,  1225,  1404,    13,  1600,  1162,
     204,  1259,  1265,  1112,  1293,  1266,  1124,   204,  1079,  1076,
      36,   981,   935,   585,  1639,  1394,   935,    60,    61,    62,
     164,   165,   406,  1719,   649,  1707,   108,  1207,   116,  1329,
       0,    48,     0,  1154,   122,  1201,     0,   116,   108,  1688,
     122,   317,     0,     0,     0,   122,  1647,     0,   206,     0,
       0,     0,     0,   203,   203,     0,     0,   218,     0,  1318,
     409,  1314,  1315,     0,  1319,     0,    60,    61,    62,   164,
    1320,   406,  1321,    33,    34,    35,     0,     0,     0,     0,
       0,   218,   151,     0,   407,   199,     0,  1586,  1572,     0,
      86,    87,  1573,    88,   169,    90,    60,    61,    62,   164,
     165,   406,  1313,     0,     0,     0,     0,     0,  1241,  1322,
    1323,    36,  1324,     0,     0,   116,     0,     0,  1420,     0,
       0,   116,     0,     0,     0,     0,   116,     0,     0,   204,
       0,     0,    48,   407,    74,    75,    76,    77,    78,  1317,
       0,  1453,     0,     0,     0,   201,     0,  1384,     0,  1385,
       0,    82,    83,  1272,     0,     0,   935,     0,   935,   151,
       0,     0,     0,   407,     0,    92,     0,   583,   961,  1733,
       0,   151,     0,     0,   206,     0,     0,    36,     0,    97,
       0,    13,  1426,   206,   122,   622,     0,     0,   550,     0,
     206,    86,    87,     0,    88,   169,    90,   206,    48,     0,
       0,     0,     0,     0,   622,     0,     0,   108,   610,     0,
       0,     0,   622,   316,     0,     0,     0,     0,     0,  1421,
    1775,     0,     0,     0,   122,   122,     0,     0,  1781,     0,
       0,   122,     0,     0,  1784,     0,     0,  1785,   203,     0,
       0,     0,   583,  1318,     0,     0,     0,     0,  1319,     0,
      60,    61,    62,   164,  1320,   406,  1321,    86,    87,     0,
      88,   169,    90,     0,   935,   116,     0,     0,   122,   108,
       0,     0,     0,     0,   108,  1720,     0,     0,   108,     0,
       0,     0,     0,     0,     0,   218,     0,   218,     0,     0,
       0,   204,   350,  1322,  1323,   542,  1324,     0,   316,     0,
       0,     0,     0,     0,     0,   116,   116,     0,  1567,     0,
       0,     0,   116,     0,     0,  1574,     0,   407,     0,   206,
       0,     0,   316,     0,   316,  1455,     0,     0,     0,     0,
     316,     0,     0,     0,   122,     0,     0,  1620,     0,     0,
     151,   122,   218,     0,     0,     0,     0,    36,   204,   116,
    1317,     0,     0,   935,     0,     0,   108,   108,   108,     0,
       0,     0,   108,    36,   203,     0,     0,   108,    48,     0,
       0,     0,     0,   203,     0,     0,     0,     0,     0,     0,
     203,     0,     0,     0,    48,     0,     0,   203,   204,     0,
     204,     0,    13,  1419,     0,     0,     0,     0,   218,     0,
       0,     0,   151,     0,     0,     0,  1406,   151,     0,     0,
       0,   151,     0,     0,     0,   116,     0,   204,     0,  1407,
    1408,     0,   116,   218,     0,     0,   218,    86,    87,     0,
      88,   169,    90,     0,     0,     0,     0,   168,  1643,     0,
      84,    85,     0,    86,    87,     0,    88,  1410,    90,     0,
      36,     0,     0,     0,  1318,  1420,     0,     0,     0,  1319,
       0,    60,    61,    62,   164,  1320,   406,  1321,     0,   218,
     204,    48,     0,     0,     0,     0,   542,     0,     0,     0,
       0,   206,     0,     0,     0,   204,   204,     0,     0,   151,
     151,   151,    36,     0,     0,   151,   258,   316,     0,     0,
     151,   935,     0,     0,  1322,  1323,   108,  1324,     0,   203,
       0,     0,     0,    48,  1662,     0,     0,     0,     0,     0,
    1703,  1567,  1567,   260,     0,  1574,  1574,   554,   407,     0,
      86,    87,     0,    88,   169,    90,  1603,     0,   206,   350,
       0,     0,     0,     0,     0,    36,   108,   108,     0,     0,
       0,     0,     0,   108,     0,     0,     0,     0,     0,     0,
     218,   218,   258,     0,   795,     0,    48,     0,     0,   332,
       0,     0,    86,    87,  -384,    88,   169,    90,   206,     0,
     206,     0,    60,    61,    62,   164,   165,   406,     0,   260,
     108,     0,     0,   875,     0,   795,  1718,     0,     0,     0,
       0,   529,   530,     0,   204,   204,     0,   206,   875,     0,
       0,    36,     0,     0,  1732,     0,     0,     0,     0,   168,
       0,     0,    84,   310,     0,    86,    87,     0,    88,   169,
      90,     0,    48,    60,    61,    62,    63,    64,   406,   151,
       0,   218,   218,   314,    70,   449,     0,     0,     0,   407,
     218,     0,     0,   315,     0,     0,   108,     0,     0,     0,
     206,     0,     0,   108,     0,     0,     0,   529,   530,     0,
       0,   203,     0,     0,     0,   206,   206,     0,     0,   151,
     151,     0,   451,     0,     0,   168,   151,     0,    84,   310,
       0,    86,    87,     0,    88,   169,    90,     0,  1295,   610,
     407,     0,     0,     0,     0,     0,     0,     0,     0,   314,
     416,   417,   418,     0,     0,     0,     0,     0,     0,   315,
       0,     0,     0,   151,     0,     0,     0,   204,   203,   419,
       0,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,     0,   442,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   443,     0,   203,     0,
     203,     0,   204,     0,     0,     0,     0,   258,     0,     0,
       0,     0,     0,     0,     0,     0,   204,   204,     0,   151,
       0,     0,     0,     0,   206,   206,   151,   203,   795,     0,
       0,     0,     0,     0,   260,     0,     0,     0,     0,     0,
       0,   218,   218,   795,   795,   795,   795,   795,     0,     0,
       0,   795,     0,     0,     0,     0,    36,     0,     0,     0,
     610,     0,     0,   218,   485,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,    48,     0,     0,
     203,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   204,     0,   218,     0,   203,   203,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   205,   205,   218,
     218,   219,   529,   530,   470,   471,     0,     0,     0,   218,
       0,     0,     0,  1218,     0,   218,     0,     0,     0,     0,
     168,     0,     0,    84,   310,     0,    86,    87,   218,    88,
     169,    90,     0,     0,     0,     0,   795,   206,     0,   218,
       0,     0,     0,     0,   314,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   315,     0,     0,   218,     0,     0,
       0,   218,    60,    61,    62,    63,    64,   406,     0,     0,
     472,   473,     0,    70,   449,     0,     0,   610,     0,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   206,   206,     0,     0,
       0,     0,     0,     0,   203,   203,     0,     0,     0,   450,
       0,   451,     0,     0,     0,     0,     0,     0,   218,     0,
       0,   218,     0,   218,   452,     0,   453,   627,     0,   407,
       0,     0,     0,     0,     0,     0,     0,     0,   795,     0,
     218,     0,     0,   795,   795,   795,   795,   795,   795,   795,
     795,   795,   795,   795,   795,   795,   795,   795,   795,   795,
     795,   795,   795,   795,   795,   795,   795,   795,   795,   795,
     795,   206,   416,   417,   418,     0,     0,     0,     0,     0,
       0,     0,   205,     0,     0,   795,     0,     0,     0,     0,
       0,   419,     0,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   218,   442,   218,     0,     0,
       0,     0,     0,     0,     0,   925,   926,   203,   443,     0,
     485,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,     0,   218,   927,     0,   218,     0,     0,
       0,     0,     0,   928,   929,   930,    36,     0,     0,   610,
       0,     0,     0,     0,     0,   931,     0,   218,     0,     0,
       0,     0,   203,     0,     0,     0,     0,    48,     0,     0,
     470,   471,     0,     0,     0,     0,   203,   203,     0,   795,
     319,     0,     0,     0,     0,     0,     0,     0,   218,     0,
       0,     0,   218,     0,     0,   795,     0,   795,   205,     0,
       0,     0,   932,     0,     0,     0,     0,   205,     0,     0,
       0,     0,     0,   795,   205,   933,     0,     0,   610,     0,
       0,   205,     0,     0,     0,     0,    86,    87,     0,    88,
     169,    90,   205,   416,   417,   418,   472,   473,     0,     0,
       0,     0,   848,     0,   934,   218,   218,     0,   218,     0,
       0,   203,   419,     0,   420,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,     0,   442,     0,     0,
       0,     0,     0,     0,     0,   416,   417,   418,     0,   443,
       0,     0,     0,   747,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,   419,     0,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,     0,   442,
       0,   218,     0,   218,     0,     0,     0,     0,   795,   218,
       0,   443,   795,   205,   795,     0,     0,   795,     0,     0,
       0,     0,     0,     0,     0,   218,   218,     0,     0,   218,
       0,     0,     0,     0,     0,     0,   218,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   319,   442,   319,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   443,   799,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   218,     0,
       0,     0,     0,   880,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   795,     0,     0,     0,     0,     0,   799,
       0,     0,   218,   218,     0,     0,     0,     0,     0,     0,
     218,   319,   218,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   416,   417,   418,     0,     0,     0,
       0,     0,     0,     0,   218,   919,   218,     0,     0,     0,
       0,     0,   218,   419,     0,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,     0,   442,     0,
       0,     0,     0,     0,     0,   205,     0,   795,   795,     0,
     443,     0,     0,   795,     0,   218,   416,   417,   418,     0,
       0,   218,   319,   218,     0,   319,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   419,     0,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,     0,
     442,     0,   205,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   443,   485,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,     0,
       0,     0,   205,     0,   205,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,   218,   470,   471,     0,     0,     0,     0,     0,
       0,   205,   799,     0,   923,  1023,   470,   471,     0,   218,
       0,     0,     0,     0,     0,     0,     0,   799,   799,   799,
     799,   799,     0,     0,     0,   799,   218,     0,     0,   319,
     780,     0,   795,   797,     0,     0,     0,  1027,     0,     0,
       0,     0,     0,   795,     0,     0,     0,     0,     0,   795,
       0,     0,     0,   795,   205,     0,     0,     0,     0,   472,
     473,     0,     0,     0,   797,     0,  1037,  1045,     0,   205,
     205,     0,   472,   473,   218,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1045,     0,     0,     0,     0,     0,
       0,     0,     0,   205,     0,  -966,  -966,  -966,  -966,  -966,
     434,   435,   436,   437,   438,   439,   440,   441,     0,   442,
     319,   319,     0,   795,     0,     0,   828,     0,     0,   319,
     799,   443,   218,  1086,  -966,  -966,  -966,  -966,  -966,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,     0,   218,     0,
       0,     0,   416,   417,   418,   219,     0,   218,     0,     0,
    1023,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   419,   218,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,     0,   442,     0,   205,   205,
       0,     0,     0,     0,     0,     0,     0,     0,   443,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   799,     0,   205,     0,     0,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   797,     0,   799,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     319,   319,   797,   797,   797,   797,   797,     0,     0,     0,
     797,     0,     0,     0,   207,   207,     0,     0,   223,     0,
       0,     0,   416,   417,   418,     0,     0,     0,     0,     0,
       0,   205,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   419,  1093,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,     0,   442,     0,   319,     0,
       0,   205,     0,     0,     0,     0,   205,     0,   443,     0,
       0,     0,     0,     0,   319,     0,     0,     0,     0,     0,
     205,   205,     0,   799,     0,     0,     0,   319,     0,     0,
       0,     0,     0,     0,     0,   797,     0,     0,     0,   799,
       0,   799,   416,   417,   418,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   319,   799,     0,     0,
       0,   419,     0,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,     0,   442,     0,     0,     0,
       0,     0,  1316,     0,     0,   205,     0,     0,   443,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   319,   442,     0,
     319,     0,   780,     0,     0,     0,     0,     0,     0,   207,
     443,     0,  1104,     0,     0,     0,     0,   797,     0,     0,
       0,     0,   797,   797,   797,   797,   797,   797,   797,   797,
     797,   797,   797,   797,   797,   797,   797,   797,   797,   797,
     797,   797,   797,   797,   797,   797,   797,   797,   797,   797,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   799,   205,   797,     0,   799,     0,   799,     0,
       0,   799,     0,     0,     0,     0,     0,     0,     0,     0,
    1402,     0,     0,  1415,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   319,     0,   319,     0,     0,     0,
       0,     0,  1127,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   319,   442,     0,   319,     0,     0,     0,
       0,     0,   205,     0,     0,   207,   443,     0,     0,     0,
       0,     0,     0,     0,   207,     0,     0,   799,     0,     0,
       0,   207,     0,     0,     0,     0,  1473,  1474,   207,     0,
       0,     0,     0,     0,     0,     0,  1415,     0,   797,   223,
       0,     0,     0,     0,     0,     0,     0,   319,     0,     0,
       0,   319,     0,     0,   797,     0,   797,   997,   998,   999,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   797,     0,     0,     0,  1000,     0,  1001,  1002,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
       0,   799,   799,     0,   319,   319,     0,   799,     0,  1618,
     223,     0,     0,  1023,     0,     0,     0,  1415,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     416,   417,   418,   485,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,     0,     0,     0,   419,
     207,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,     0,   442,     0,     0,     0,     0,     0,
       0,     0,     0,   470,   471,     0,   443,     0,     0,     0,
     319,     0,   319,     0,     0,     0,     0,   797,     0,     0,
       0,   797,     0,   797,     0,   800,   797,     0,     0,     0,
       0,     0,     0,     0,   319,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   319,     0,     0,     0,     0,
       0,     0,     0,     0,  1167,     0,   800,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   472,
     473,     0,     0,     0,     0,     0,   799,     0,     0,     0,
     796,     0,     0,     0,     0,     0,     0,   799,     0,     0,
       0,     0,     0,   799,     0,     0,     0,   799,     0,     0,
       0,     0,   797,     0,     0,     0,     0,     0,     0,     0,
       0,   796,     0,     0,     0,     0,     0,     0,     0,   319,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1438,     0,   207,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   319,     0,   319,     0,     0,     0,     0,
       0,   319,     0,     0,     0,     0,     0,   799,     0,     0,
       0,     0,     0,     0,     0,     0,  1729,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1402,   416,   417,   418,   797,   797,     0,   207,
       0,     0,   797,     0,     0,     0,     0,     0,     0,     0,
     319,     0,   419,     0,   420,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,     0,   442,     0,   207,
       0,   207,     0,     0,     0,     0,     0,     0,     0,   443,
       0,     0,     0,     0,     0,     0,     0,  1176,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   207,   800,
       0,     0,     0,     0,   786,   787,     0,     0,     0,     0,
     788,     0,   789,     0,   800,   800,   800,   800,   800,   801,
       0,     0,   800,     0,   790,     0,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,     0,     0,     0,     0,
       0,   319,     0,     0,   199,     0,     0,     0,     0,     0,
     821,   207,     0,     0,   796,     0,    48,     0,   319,     0,
       0,     0,     0,     0,     0,     0,   207,   207,     0,   796,
     796,   796,   796,   796,     0,  1663,     0,   796,     0,     0,
       0,   797,     0,     0,     0,     0,     0,     0,     0,     0,
     223,   791,   797,    74,    75,    76,    77,    78,   797,     0,
       0,     0,   797,  1439,   201,     0,     0,     0,     0,   168,
      82,    83,    84,   792,     0,    86,    87,   800,    88,   169,
      90,     0,     0,   319,    92,     0,     0,     0,     0,     0,
       0,     0,     0,   793,     0,     0,     0,     0,    97,     0,
       0,     0,   223,   794,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,   797,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   796,     0,     0,  1023,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   207,   207,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   319,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   319,     0,     0,     0,     0,     0,     0,     0,   800,
       0,   223,     0,     0,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,     0,   962,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   800,     0,   984,   985,
     986,   987,     0,     0,   796,   958,   994,     0,     0,   796,
     796,   796,   796,   796,   796,   796,   796,   796,   796,   796,
     796,   796,   796,   796,   796,   796,   796,   796,   796,   796,
     796,   796,   796,   796,   796,   796,   796,    28,   207,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,     0,
     198,   796,     0,     0,     0,     0,     0,   199,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,   223,     0,
       0,     0,     0,   207,     0,     0,     0,     0,     0,     0,
     200,     0,     0,     0,     0,     0,     0,   207,   207,     0,
     800,  1083,     0,   959,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,   800,   201,   800,     0,
       0,     0,   168,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   169,    90,   800,     0,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    97,     0,     0,     0,   796,   202,     0,     0,     0,
       0,   103,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   796,   207,   796,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   796,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1170,  1173,
    1174,  1175,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1187,  1188,  1189,  1190,  1191,  1192,  1193,  1194,
    1195,  1196,  1197,  1198,  1199,  1200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1206,     0,   416,   417,   418,     0,     0,     0,     0,   800,
     223,     0,     0,   800,     0,   800,     0,     0,   800,     0,
       0,   419,  1288,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,     0,   442,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   443,     0,
       0,     0,     0,     0,   796,     0,     0,     0,   796,     0,
     796,     0,     0,   796,     0,     0,     0,     0,     0,   223,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   800,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1283,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1298,     0,  1299,     0,     0,     0,     0,     0,     0,   416,
     417,   418,     0,     0,     0,     0,     0,     0,  1309,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   419,   796,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,     0,   442,     0,   416,   417,   418,   800,   800,
    1289,     0,     0,     0,   800,   443,     0,     0,     0,     0,
       0,     0,     0,  1623,   419,     0,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,     0,   442,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   443,     0,   796,   796,     0,     0,     0,     0,   796,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   416,   417,   418,     0,     0,     0,     0,     0,
       0,     0,     0,  1387,     0,     0,     0,  1389,     0,  1390,
       0,   419,  1391,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,     0,   442,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   444,   443,   419,
       0,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   800,   442,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   800,     0,   443,     0,  1469,     0,
     800,     0,     0,   526,   800,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,  1704,   442,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   796,     0,
     443,     5,     6,     7,     8,     9,     0,     0,     0,   796,
       0,    10,     0,     0,     0,   796,     0,     0,     0,   796,
       0,     0,     0,     0,   800,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     528,     0,  1612,  1613,    13,    14,    15,     0,  1617,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,   796,
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
      98,    99,     0,     0,   100,     0,   101,   102,  1053,   103,
     104,     0,   105,   106,     0,     0,     0,  1672,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,  1682,     0,
      10,     0,     0,     0,  1687,     0,     0,     0,  1689,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,  1722,    39,
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
      99,     0,     0,   100,     0,   101,   102,  1219,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
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
       0,   101,   102,   629,   103,   104,     0,   105,   106,     5,
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
      79,     0,     0,    80,     0,     0,     0,     0,   168,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   169,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,  1026,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
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
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1067,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
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
       0,     0,     0,     0,   168,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   169,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,  1133,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,  1135,    45,     0,    46,
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
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,  1284,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     168,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     169,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   101,   102,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
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
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,  1393,   103,   104,     0,   105,   106,     5,     6,     7,
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
       0,    80,     0,     0,     0,     0,   168,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   169,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,  1614,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,  1659,    47,     0,     0,    48,    49,     0,     0,
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
       0,     0,   168,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   169,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
    1693,   103,   104,     0,   105,   106,     5,     6,     7,     8,
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
      80,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,  1694,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,  1697,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,     0,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   168,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   169,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
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
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1713,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
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
       0,     0,     0,     0,   168,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   169,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,  1769,   103,   104,     0,   105,   106,     5,     6,
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
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1776,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
     168,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     169,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   101,   102,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,   512,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    11,    12,     0,   765,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,   164,   165,    65,     0,    66,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   168,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   169,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   964,     0,     0,
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
    1464,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,   164,   165,
      65,     0,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   168,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   169,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,  1607,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,   164,   165,    65,     0,    66,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   168,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   169,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   378,    12,     0,     0,
       0,     0,     0,     0,   701,     0,     0,     0,     0,     0,
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
      97,    98,    99,     0,     0,   100,     0,     0,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     164,   165,   166,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   167,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   168,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   169,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   170,     0,
     324,     0,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,     0,   442,
     644,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   443,    14,    15,     0,     0,     0,     0,    16,     0,
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
     645,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   170,     0,     0,     0,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   164,   165,   166,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   167,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     168,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     169,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   170,     0,     0,   760,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,     0,     0,  1080,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1023,    14,    15,
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
      87,     0,    88,   169,    90,     0,  1081,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   170,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   378,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   164,   165,   166,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   167,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   168,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   169,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   416,   417,   418,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   419,     0,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,     0,   442,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   443,     0,
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
     545,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   170,     0,     0,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  -966,  -966,  -966,  -966,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,     0,   442,     0,     0,   211,     0,     0,     0,     0,
       0,     0,     0,     0,   443,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   164,   165,
     166,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   167,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   168,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   169,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   170,     0,   416,   417,
     418,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   419,     0,   420,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,     0,   442,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   443,     0,    16,     0,    17,    18,
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
      92,     0,     0,    93,     0,     0,   549,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   170,
       0,   244,   417,   418,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     419,     0,   420,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,     0,   442,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   443,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   164,   165,   166,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   167,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   168,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   169,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   170,     0,   247,     0,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   378,     0,     0,     0,
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
      97,    98,    99,     0,     0,   100,     0,   416,   417,   418,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   419,     0,   420,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
       0,   442,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   443,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     164,   165,   166,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   167,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   168,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   169,    90,     0,     0,     0,    92,
       0,     0,    93,     0,   733,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   170,   510,
       0,     0,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   657,     0,     0,     0,     0,
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
       0,   170,     0,     0,     0,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  -966,  -966,  -966,  -966,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,     0,
       0,     0,     0,   701,     0,     0,     0,     0,     0,     0,
       0,     0,  1023,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   164,   165,   166,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   167,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     168,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     169,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   170,     0,     0,     0,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   741,     0,     0,     0,
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
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   743,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   164,   165,   166,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   167,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   168,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   169,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     170,     0,     0,     0,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1125,     0,     0,     0,     0,     0,     0,     0,
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
      99,     0,     0,   170,     0,   416,   417,   418,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   419,     0,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,     0,   442,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   443,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   164,   165,
     166,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   167,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   168,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   169,    90,     0,     0,     0,    92,     0,     0,
      93,     0,   757,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   170,     0,   416,   417,
     418,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   419,   897,   420,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,     0,   442,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   443,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,   591,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   164,   165,   166,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   167,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,     0,   249,   250,
      92,   251,   252,    93,     0,   253,   254,   255,   256,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   170,
       0,     0,   257,     0,   103,   104,     0,   105,   106,  1003,
    1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,   259,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1023,   261,   262,   263,   264,   265,   266,   267,
       0,     0,     0,    36,     0,   198,     0,     0,     0,     0,
       0,     0,     0,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,    48,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,     0,     0,
       0,   689,   303,   304,   305,     0,     0,     0,   306,   539,
     540,   249,   250,     0,   251,   252,     0,     0,   253,   254,
     255,   256,     0,     0,     0,     0,     0,   541,     0,     0,
       0,     0,     0,    86,    87,   257,    88,   169,    90,   311,
       0,   312,     0,     0,   313,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   259,     0,   690,     0,   103,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   261,   262,   263,   264,
     265,   266,   267,     0,     0,     0,    36,     0,   198,     0,
       0,     0,     0,     0,     0,     0,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,    48,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,     0,     0,     0,   302,   303,   304,   305,     0,     0,
       0,   306,   539,   540,     0,     0,     0,     0,     0,   249,
     250,     0,   251,   252,     0,     0,   253,   254,   255,   256,
     541,     0,     0,     0,     0,     0,    86,    87,     0,    88,
     169,    90,   311,   257,   312,   258,     0,   313,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   690,     0,   103,
     259,     0,   260,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   261,   262,   263,   264,   265,   266,
     267,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,    48,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,     0,
       0,     0,     0,   303,   304,   305,     0,     0,     0,   306,
     307,   308,     0,     0,     0,     0,     0,   249,   250,     0,
     251,   252,     0,     0,   253,   254,   255,   256,   309,     0,
       0,    84,   310,     0,    86,    87,     0,    88,   169,    90,
     311,   257,   312,   258,     0,   313,     0,     0,     0,     0,
       0,     0,   314,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   315,     0,     0,     0,  1587,     0,   259,     0,
     260,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   261,   262,   263,   264,   265,   266,   267,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,    48,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,     0,     0,     0,
       0,   303,   304,   305,     0,     0,     0,   306,   307,   308,
       0,     0,     0,     0,     0,   249,   250,     0,   251,   252,
       0,     0,   253,   254,   255,   256,   309,     0,     0,    84,
     310,     0,    86,    87,     0,    88,   169,    90,   311,   257,
     312,   258,     0,   313,     0,     0,     0,     0,     0,     0,
     314,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     315,     0,     0,     0,  1655,     0,   259,     0,   260,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     261,   262,   263,   264,   265,   266,   267,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,    48,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,     0,     0,     0,   302,   303,
     304,   305,     0,     0,     0,   306,   307,   308,     0,     0,
       0,     0,     0,   249,   250,     0,   251,   252,     0,     0,
     253,   254,   255,   256,   309,     0,     0,    84,   310,     0,
      86,    87,     0,    88,   169,    90,   311,   257,   312,   258,
       0,   313,     0,     0,     0,     0,     0,     0,   314,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   315,     0,
       0,     0,     0,     0,   259,     0,   260,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   261,   262,
     263,   264,   265,   266,   267,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,    48,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,     0,     0,     0,     0,   303,   304,   305,
       0,     0,     0,   306,   307,   308,     0,     0,     0,     0,
       0,   249,   250,     0,   251,   252,     0,     0,   253,   254,
     255,   256,   309,     0,     0,    84,   310,     0,    86,    87,
       0,    88,   169,    90,   311,   257,   312,   258,     0,   313,
       0,     0,     0,     0,     0,     0,   314,  1397,     0,     0,
       0,     0,     0,     0,     0,     0,   315,     0,     0,     0,
       0,     0,   259,     0,   260,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   261,   262,   263,   264,
     265,   266,   267,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,    48,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,     0,     0,     0,     0,   303,   304,   305,     0,     0,
       0,   306,   307,   308,    33,    34,    35,    36,     0,   198,
       0,     0,     0,     0,     0,     0,   199,     0,     0,     0,
     309,     0,     0,    84,   310,     0,    86,    87,    48,    88,
     169,    90,   311,     0,   312,     0,     0,   313,  1488,  1489,
    1490,  1491,  1492,     0,   314,  1493,  1494,  1495,  1496,   216,
       0,     0,     0,     0,   315,     0,     0,     0,     0,     0,
       0,     0,  1497,  1498,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   201,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,  1499,
      88,   169,    90,     0,     0,     0,    92,     0,     0,     0,
       0,     0,     0,  1500,  1501,  1502,  1503,  1504,  1505,  1506,
      97,     0,     0,    36,     0,   217,     0,     0,     0,     0,
     103,     0,     0,  1507,  1508,  1509,  1510,  1511,  1512,  1513,
    1514,  1515,  1516,  1517,    48,  1518,  1519,  1520,  1521,  1522,
    1523,  1524,  1525,  1526,  1527,  1528,  1529,  1530,  1531,  1532,
    1533,  1534,  1535,  1536,  1537,  1538,  1539,  1540,  1541,  1542,
    1543,  1544,  1545,  1546,  1547,     0,     0,     0,  1548,  1549,
       0,  1550,  1551,  1552,  1553,  1554,   249,   250,     0,   251,
     252,     0,     0,   253,   254,   255,   256,  1555,  1556,  1557,
       0,     0,     0,    86,    87,     0,    88,   169,    90,  1558,
     257,  1559,  1560,     0,  1561,     0,     0,     0,     0,     0,
       0,  1562,  1563,     0,  1564,     0,  1565,  1566,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   259,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   261,   262,   263,   264,   265,   266,   267,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,    48,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,     0,     0,     0,   302,
     303,   304,   305,     0,     0,     0,   306,   539,   540,   249,
     250,     0,   251,   252,     0,     0,   253,   254,   255,   256,
       0,     0,     0,     0,     0,   541,     0,     0,     0,     0,
       0,    86,    87,   257,    88,   169,    90,   311,     0,   312,
       0,     0,   313,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     259,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   261,   262,   263,   264,   265,   266,
     267,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,    48,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,     0,
       0,     0,  1168,   303,   304,   305,     0,     0,     0,   306,
     539,   540,   249,   250,     0,   251,   252,     0,     0,   253,
     254,   255,   256,     0,     0,     0,     0,     0,   541,     0,
       0,     0,     0,     0,    86,    87,   257,    88,   169,    90,
     311,     0,   312,     0,     0,   313,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   259,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   261,   262,   263,
     264,   265,   266,   267,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,    48,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,     0,     0,     0,     0,   303,   304,   305,     0,
       0,     0,   306,   539,   540,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   786,   787,     0,     0,     0,     0,
     788,   541,   789,     0,     0,     0,     0,    86,    87,     0,
      88,   169,    90,   311,   790,   312,     0,     0,   313,     0,
       0,     0,    33,    34,    35,    36,   416,   417,   418,     0,
       0,     0,     0,     0,   199,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   419,    48,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,     0,
     442,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   791,   443,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   201,     0,     0,     0,     0,   168,
      82,    83,    84,   792,     0,    86,    87,    28,    88,   169,
      90,     0,     0,     0,    92,    33,    34,    35,    36,     0,
     198,     0,     0,   793,     0,     0,     0,   199,    97,     0,
       0,     0,     0,   794,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,   488,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     200,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   201,     0,     0,
       0,     0,   168,    82,    83,    84,    85,     0,    86,    87,
      28,    88,   169,    90,     0,     0,     0,    92,    33,    34,
      35,    36,     0,   198,     0,     0,     0,     0,     0,     0,
     199,    97,     0,     0,     0,     0,   202,     0,     0,   562,
       0,   103,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   582,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     201,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,    28,     0,   914,
      92,     0,     0,     0,     0,    33,    34,    35,    36,     0,
     198,     0,     0,     0,    97,     0,     0,   199,     0,   202,
       0,     0,     0,     0,   103,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     200,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   201,     0,     0,
       0,     0,   168,    82,    83,    84,    85,     0,    86,    87,
      28,    88,   169,    90,     0,     0,     0,    92,    33,    34,
      35,    36,     0,   198,     0,     0,     0,     0,     0,     0,
     199,    97,     0,     0,     0,     0,   202,     0,     0,     0,
       0,   103,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1048,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     201,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,    28,    88,   169,    90,     0,     0,     0,
      92,    33,    34,    35,    36,     0,   198,     0,     0,     0,
       0,     0,     0,   199,    97,     0,     0,     0,     0,   202,
       0,     0,     0,     0,   103,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   201,     0,     0,     0,     0,   168,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   169,    90,
       0,     0,     0,    92,     0,     0,     0,   416,   417,   418,
       0,     0,     0,     0,     0,     0,     0,    97,     0,     0,
       0,     0,   202,     0,     0,     0,   419,   103,   420,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
       0,   442,   416,   417,   418,     0,     0,     0,     0,     0,
       0,     0,     0,   443,     0,     0,     0,     0,     0,     0,
       0,   419,     0,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,     0,   442,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   443,   416,
     417,   418,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   419,   497,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,     0,   442,   416,   417,   418,     0,     0,     0,
       0,     0,     0,     0,     0,   443,     0,     0,     0,     0,
       0,     0,     0,   419,   900,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,     0,   442,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     443,   997,   998,   999,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1000,   944,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,     0,     0,   997,   998,   999,     0,
       0,     0,     0,     0,     0,     0,     0,  1023,     0,     0,
       0,     0,     0,     0,     0,  1000,  1248,  1001,  1002,  1003,
    1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1023,   997,   998,   999,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1000,  1158,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,     0,     0,   997,   998,
     999,     0,     0,     0,     0,     0,     0,     0,     0,  1023,
       0,     0,     0,     0,     0,     0,     0,  1000,  1305,  1001,
    1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,     0,     0,     0,    33,    34,    35,    36,     0,   198,
       0,     0,     0,     0,  1023,     0,   605,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,  1386,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   201,     0,     0,     0,
    1471,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,     0,     0,     0,
     416,   417,   418,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   606,     0,     0,   769,   419,
     103,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,     0,   442,   416,   417,   418,     0,     0,
       0,     0,     0,     0,     0,     0,   443,     0,     0,     0,
       0,     0,     0,     0,   419,     0,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   770,   442,
     997,   998,   999,     0,     0,     0,     0,     0,     0,     0,
       0,   443,     0,     0,     0,     0,     0,     0,     0,  1000,
    1310,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,   997,   998,   999,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1023,     0,     0,     0,
       0,     0,  1000,     0,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,   998,   999,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1023,
       0,     0,     0,     0,  1000,     0,  1001,  1002,  1003,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,   418,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1023,     0,     0,     0,   419,     0,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   999,
     442,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   443,     0,     0,     0,  1000,     0,  1001,  1002,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1000,  1023,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1023,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1023,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,     0,   442,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     443,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1023
};

static const yytype_int16 yycheck[] =
{
       5,     6,   122,     8,     9,    10,    11,    12,   147,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     4,   172,    28,    29,   100,    91,    54,   647,    32,
      95,    96,     4,     4,  1070,     4,   152,    42,   371,    30,
     873,    44,   482,   214,    55,    50,    49,    52,   218,   371,
      55,   698,    57,   173,    54,   147,   121,    30,     4,   862,
     371,   617,   478,   479,   100,   442,    30,    30,  1057,   598,
     775,   100,    42,    58,    79,   768,   512,   100,   893,     9,
     474,     9,   474,   957,     9,   618,   228,     9,     9,     9,
     506,   735,     9,    30,   909,   100,    81,    14,     9,    84,
       9,     9,     9,    14,    80,    14,     9,    67,     9,    79,
     546,     9,     9,   229,   508,     9,   508,     4,     9,     9,
       9,     9,    80,     9,     9,     9,     9,   111,     4,     9,
       9,     9,     9,    46,   170,     9,    80,     9,    80,    35,
       9,   170,    80,     9,    67,    98,    80,   170,     9,    30,
     172,    46,    14,    98,   151,   172,    33,    67,   151,    29,
       0,   151,   103,    98,   186,   170,   202,    98,    30,   186,
      46,    35,   177,   202,    67,   172,    80,    47,   186,   202,
      50,   217,   172,   619,    80,   169,    48,    46,   217,    46,
     993,    46,   189,   186,   505,     8,   186,   202,     4,   189,
     186,    35,   155,   118,   186,    26,    27,   130,   131,    35,
     155,   126,   217,    67,   356,   191,    80,   151,    67,    67,
     155,   165,    67,   165,   155,   230,   186,   165,   233,   189,
      67,    67,   108,   191,    67,   240,   241,   113,    67,   115,
     116,   117,   118,   119,   120,   121,    80,    67,   173,    67,
      51,   173,   189,   173,    80,   172,  1245,   187,   408,   187,
     147,   165,   967,  1252,   969,  1254,  1140,   188,   189,   165,
      67,   188,  1066,    67,   187,    67,   186,   188,    67,   188,
     188,   188,   158,   159,   187,   161,  1101,   188,   234,   184,
     188,   188,   238,   186,   188,   173,   323,   188,   188,   188,
     188,   165,   188,   188,   188,   188,   182,   188,   188,   188,
     187,    67,   348,   187,   190,   187,   122,  1120,   187,   348,
     491,   187,   851,   323,   760,   348,   187,   184,   187,   765,
     187,   165,   187,   151,   484,   189,   456,   402,   186,   165,
     189,   189,   347,   348,   189,   330,   331,   332,    67,   354,
     186,  1601,   189,   189,   359,   156,   189,   890,   151,   151,
     189,   186,   151,   186,    35,    35,   186,   173,   186,   189,
     171,   189,   399,   378,  1363,    67,   156,   347,   363,   172,
     189,   386,   457,    51,   102,   103,   451,   452,   453,   454,
     211,    80,   189,   398,   377,   189,    80,   189,   838,   399,
     189,   186,    86,   102,   103,   186,  1656,   186,  1055,    80,
      80,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   608,   443,     4,
     445,   446,   447,  1136,   130,   131,   457,    54,   448,  1243,
    1318,   442,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   149,   150,   186,   187,   442,
     475,   476,   190,   478,   479,   480,   481,  1121,   442,   442,
     485,    80,   624,   488,   626,   879,    51,   879,   156,    54,
     906,   190,   497,    80,   499,   495,   635,   189,   385,  1302,
     189,   506,   191,   514,    86,   172,    71,   448,   155,   514,
     194,   516,    67,  1069,   505,   157,   337,   688,    67,   186,
     130,   131,   692,    86,    89,   346,    91,   348,   964,   862,
      95,    96,   353,   474,    98,  1403,   186,   914,    80,   360,
     862,   188,   189,   635,    86,    80,   186,   189,    98,   186,
     700,   862,  1085,   723,   495,  1088,   121,   562,  1352,  1427,
    1354,  1429,   149,   150,   505,   550,   148,   508,  1371,   554,
     606,   119,     4,   519,   559,   130,   131,    80,   126,   385,
     190,   130,   131,    86,   183,   148,    80,    30,   653,   186,
     189,   155,    86,    80,    26,    27,   157,   739,   740,    86,
     186,   606,   151,   745,   746,   155,  1253,   149,   150,   170,
      29,   782,   155,   148,   149,   150,   172,  1261,    80,  1263,
     185,   101,   102,   103,    86,   795,    80,    54,   189,   745,
     186,    35,    86,   803,   189,   618,   446,   186,    67,    66,
     645,   130,   131,    78,   172,    80,   149,   150,   186,   455,
      67,   258,   657,   260,   148,   149,   150,  1451,   186,    78,
     993,   482,   149,   150,    99,   475,    47,    48,   100,   234,
     480,   993,    29,   238,    72,    73,  1232,   242,  1094,   188,
      99,   188,   993,    72,    73,   690,  1333,   149,   150,  1105,
      47,   188,   195,    50,   148,   149,   150,   186,   188,  1139,
     128,   129,   121,   188,  1740,   101,   102,   103,   315,   115,
     116,   117,   188,   718,  1247,   134,   135,   188,   189,  1755,
      78,   156,   189,   158,   159,  1369,   161,   162,   163,   151,
      50,    51,    52,   152,    54,   186,   155,   156,   170,   158,
     159,    99,   161,   162,   163,   186,    66,   752,   635,    50,
      51,    52,    98,    99,   189,    67,   191,  1404,   323,   151,
      78,   764,   186,   768,   155,    66,    45,  1203,   939,    66,
     202,   104,   105,   106,  1214,    75,    76,    77,    78,   211,
     188,    99,   357,   188,   189,   217,   361,  1120,   172,   396,
    1630,  1631,   399,   186,   152,  1626,  1627,   155,  1120,    99,
     158,   159,   234,   161,   162,   163,   238,   784,   785,  1120,
    1604,   151,   983,   388,  1458,   390,   391,   392,   393,   990,
     385,   186,   193,   644,   809,     9,   151,   773,   813,   151,
     395,  1364,   190,  1249,   399,   186,     8,   402,  1274,   188,
     158,   159,   847,   161,   162,   163,   151,   186,   913,    14,
     188,   126,   151,   126,  1290,    14,   861,   187,   158,   159,
      14,   161,   162,   163,   172,   865,    98,   850,   186,   192,
     187,   758,   187,   187,   187,  1708,   186,   107,   850,   850,
     701,   850,   887,   448,   449,   450,   451,   452,   453,   454,
     873,   186,   897,   186,  1727,   900,     9,   902,    78,   148,
      80,   906,  1735,   187,   850,   337,  1076,   890,    90,   474,
     187,   187,     9,   187,   346,    14,   348,   188,   805,    99,
     741,   353,   743,   914,   865,   172,   533,   186,   360,     9,
     495,   186,    80,   187,  1350,   187,   877,  1373,   879,   944,
    1734,   914,   187,   508,  1115,   188,  1382,   950,   186,   770,
     914,   914,   758,   385,   519,  1749,  1392,   128,   187,    67,
      30,   129,  1606,   850,   171,   151,   132,     9,   187,  1302,
     151,   184,     9,   538,    14,  1050,   156,     9,   158,   159,
    1302,   161,   162,   163,   871,   173,    29,   187,     9,    78,
      14,  1302,   992,   128,  1165,   995,   561,   193,   193,   805,
     190,  1172,   823,     9,   945,   951,    14,   614,   615,   189,
      99,   191,   187,    56,   193,   193,   623,   838,   839,   151,
     187,    98,   587,   588,   186,   188,   187,    87,  1464,   132,
     188,  1650,   151,     9,   187,    78,   186,   151,  1371,  1050,
     186,   151,     4,   188,   850,  1050,   189,   189,    14,  1371,
     482,   189,   193,    14,    14,   189,    99,   187,   184,   188,
    1371,    30,   186,   186,   107,   871,   953,    30,   955,   158,
     159,  1054,   161,   162,   163,    14,  1081,   186,    14,   186,
     186,    49,  1054,  1054,    46,  1054,   186,   519,   653,  1094,
       9,   134,   135,   187,   132,   188,   188,   186,   186,  1270,
    1105,  1106,  1085,    14,   132,  1088,  1725,    78,  1054,   152,
       9,   187,   155,   156,    66,   158,   159,     9,   161,   162,
     163,   193,    80,     9,   186,  1128,    14,   132,    99,   186,
     188,  1136,    80,   176,   187,   189,   957,   958,   186,   186,
     189,  1146,   187,   186,   132,   189,   108,   953,     9,   955,
      87,   113,   188,   115,   116,   117,   118,   119,   120,   121,
    1596,   148,  1598,   193,    30,    74,   188,  1054,   187,   173,
     735,  1607,   737,   188,    30,   132,   187,   784,   785,   132,
     187,     9,    78,     9,   155,  1131,   187,   158,   159,   187,
     161,   162,   163,   758,   190,  1334,   158,   159,   190,   161,
     189,    80,    14,    99,   186,   188,    78,   772,   773,  1329,
     187,   189,   644,    26,    27,   187,   186,  1653,   189,  1224,
     182,     9,   187,  1228,   132,  1230,    30,    99,   190,  1212,
     189,   187,   187,  1238,   188,   107,   108,   187,  1221,   188,
     805,   187,     4,  1248,  1249,   852,   811,   187,  1054,   108,
     815,   816,   156,   188,   160,   188,  1143,    14,    80,  1080,
     156,   868,   158,   159,  1247,   161,   162,   163,   107,   701,
     835,   113,   187,   187,   881,  1425,   115,   116,   117,   118,
     119,   120,   189,   155,    46,   850,   158,   159,   132,   161,
     162,   163,   187,    14,    47,    48,    49,    50,    51,    52,
     865,    54,   132,   910,  1125,   172,   871,   189,   188,   741,
      80,   743,   877,    66,   879,    14,    14,  1753,  1139,  1140,
      80,   189,   187,   186,  1760,  1212,   758,   187,    78,   132,
      80,    81,  1397,   188,  1221,   188,    14,  1143,   770,    14,
     188,   773,  1347,   182,    14,  1350,   108,     9,   913,    99,
     190,   113,    29,   115,   116,   117,   118,   119,   120,   121,
     925,   926,   927,    56,   971,   189,    80,   974,    29,   186,
    1353,    78,   172,   805,    80,     9,  1359,   188,  1361,    56,
     945,  1364,   111,    98,   151,    98,   951,   163,   953,    33,
     955,   823,    99,  1214,    14,   186,   158,   159,   211,   161,
    1400,    78,   187,    26,    27,   188,   838,   839,   158,   159,
     975,   161,   162,   163,   186,   169,    80,    78,   850,   166,
     182,   173,    99,   187,     9,  1312,    87,   992,   190,    80,
     995,   189,   188,  1379,    14,    80,  1586,  1442,    99,   871,
      14,   187,   187,    80,    14,   152,    14,  1334,   155,   156,
    1621,   158,   159,    80,   161,   162,   163,   134,   135,  1024,
     813,  1068,    80,  1070,   559,  1030,  1353,   809,  1033,  1716,
     454,   451,  1359,  1456,  1361,   152,   449,   907,   155,   156,
     853,   158,   159,  1731,   161,   162,   163,  1374,   165,  1054,
    1097,   152,   912,  1100,   155,     4,  1383,   158,   159,   176,
     161,   162,   163,  1137,  1463,  1287,  1312,  1727,   564,   186,
      47,    48,    49,    50,    51,    52,  1462,  1463,  1454,   951,
    1486,   953,  1328,   955,   337,   957,   958,  1570,  1324,    66,
    1405,  1759,  1747,   346,  1582,  1058,  1320,    46,  1450,   991,
     353,  1103,  1116,   926,  1151,  1117,   941,   360,  1155,   877,
      78,   784,  1117,   354,  1581,  1313,  1121,   115,   116,   117,
     118,   119,   120,  1702,   399,  1684,  1131,  1041,  1374,  1456,
      -1,    99,    -1,   976,  1461,  1024,    -1,  1383,  1143,  1644,
    1467,  1581,    -1,    -1,    -1,  1472,  1591,    -1,   211,    -1,
      -1,    -1,    -1,    26,    27,    -1,    -1,    30,    -1,   108,
    1720,  1208,  1209,    -1,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    54,  1054,    -1,   182,    87,    -1,  1433,   156,    -1,
     158,   159,   160,   161,   162,   163,   115,   116,   117,   118,
     119,   120,  1207,    -1,    -1,    -1,    -1,    -1,  1080,   158,
     159,    78,   161,    -1,    -1,  1461,    -1,    -1,   186,    -1,
      -1,  1467,    -1,    -1,    -1,    -1,  1472,    -1,    -1,   482,
      -1,    -1,    99,   182,   136,   137,   138,   139,   140,     4,
      -1,   190,    -1,    -1,    -1,   147,    -1,  1294,    -1,  1296,
      -1,   153,   154,  1125,    -1,    -1,  1261,    -1,  1263,  1131,
      -1,    -1,    -1,   182,    -1,   167,    -1,  1139,  1140,   188,
      -1,  1143,    -1,    -1,   337,    -1,    -1,    78,    -1,   181,
      -1,    46,  1329,   346,  1611,  1708,    -1,    -1,   155,    -1,
     353,   158,   159,    -1,   161,   162,   163,   360,    99,    -1,
      -1,    -1,    -1,    -1,  1727,    -1,    -1,  1312,   371,    -1,
      -1,    -1,  1735,  1318,    -1,    -1,    -1,    -1,    -1,  1324,
    1765,    -1,    -1,    -1,  1651,  1652,    -1,    -1,  1773,    -1,
      -1,  1658,    -1,    -1,  1779,    -1,    -1,  1782,   211,    -1,
      -1,    -1,  1214,   108,    -1,    -1,    -1,    -1,   113,    -1,
     115,   116,   117,   118,   119,   120,   121,   158,   159,    -1,
     161,   162,   163,    -1,  1369,  1611,    -1,    -1,  1695,  1374,
      -1,    -1,    -1,    -1,  1379,  1702,    -1,    -1,  1383,    -1,
      -1,    -1,    -1,    -1,    -1,   258,    -1,   260,    -1,    -1,
      -1,   644,  1397,   158,   159,  1400,   161,    -1,  1403,    -1,
      -1,    -1,    -1,    -1,    -1,  1651,  1652,    -1,  1413,    -1,
      -1,    -1,  1658,    -1,    -1,  1420,    -1,   182,    -1,   482,
      -1,    -1,  1427,    -1,  1429,   190,    -1,    -1,    -1,    -1,
    1435,    -1,    -1,    -1,  1761,    -1,    -1,  1484,    -1,    -1,
    1312,  1768,   315,    -1,    -1,    -1,    -1,    78,   701,  1695,
       4,    -1,    -1,  1458,    -1,    -1,  1461,  1462,  1463,    -1,
      -1,    -1,  1467,    78,   337,    -1,    -1,  1472,    99,    -1,
      -1,    -1,    -1,   346,    -1,    -1,    -1,    -1,    -1,    -1,
     353,    -1,    -1,    -1,    99,    -1,    -1,   360,   741,    -1,
     743,    -1,    46,   124,    -1,    -1,    -1,    -1,   371,    -1,
      -1,    -1,  1374,    -1,    -1,    -1,   121,  1379,    -1,    -1,
      -1,  1383,    -1,    -1,    -1,  1761,    -1,   770,    -1,   134,
     135,    -1,  1768,   396,    -1,    -1,   399,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,    -1,   152,  1585,    -1,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      78,    -1,    -1,    -1,   108,   186,    -1,    -1,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,    -1,   442,
     823,    99,    -1,    -1,    -1,    -1,  1581,    -1,    -1,    -1,
      -1,   644,    -1,    -1,    -1,   838,   839,    -1,    -1,  1461,
    1462,  1463,    78,    -1,    -1,  1467,    29,  1602,    -1,    -1,
    1472,  1606,    -1,    -1,   158,   159,  1611,   161,    -1,   482,
      -1,    -1,    -1,    99,  1619,    -1,    -1,    -1,    -1,    -1,
    1667,  1626,  1627,    56,    -1,  1630,  1631,   155,   182,    -1,
     158,   159,    -1,   161,   162,   163,   190,    -1,   701,  1644,
      -1,    -1,    -1,    -1,    -1,    78,  1651,  1652,    -1,    -1,
      -1,    -1,    -1,  1658,    -1,    -1,    -1,    -1,    -1,    -1,
     533,   534,    29,    -1,   537,    -1,    99,    -1,    -1,   155,
      -1,    -1,   158,   159,   107,   161,   162,   163,   741,    -1,
     743,    -1,   115,   116,   117,   118,   119,   120,    -1,    56,
    1695,    -1,    -1,  1740,    -1,   568,  1701,    -1,    -1,    -1,
      -1,   134,   135,    -1,   957,   958,    -1,   770,  1755,    -1,
      -1,    78,    -1,    -1,  1719,    -1,    -1,    -1,    -1,   152,
      -1,    -1,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    99,   115,   116,   117,   118,   119,   120,  1611,
      -1,   614,   615,   176,   126,   127,    -1,    -1,    -1,   182,
     623,    -1,    -1,   186,    -1,    -1,  1761,    -1,    -1,    -1,
     823,    -1,    -1,  1768,    -1,    -1,    -1,   134,   135,    -1,
      -1,   644,    -1,    -1,    -1,   838,   839,    -1,    -1,  1651,
    1652,    -1,   164,    -1,    -1,   152,  1658,    -1,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,   165,   862,
     182,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,  1695,    -1,    -1,    -1,  1080,   701,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,   741,    -1,
     743,    -1,  1125,    -1,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1139,  1140,    -1,  1761,
      -1,    -1,    -1,    -1,   957,   958,  1768,   770,   771,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,   784,   785,   786,   787,   788,   789,   790,    -1,    -1,
      -1,   794,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,
     993,    -1,    -1,   806,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    99,    -1,    -1,
     823,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1214,    -1,   836,    -1,   838,   839,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    27,   852,
     853,    30,   134,   135,    64,    65,    -1,    -1,    -1,   862,
      -1,    -1,    -1,   193,    -1,   868,    -1,    -1,    -1,    -1,
     152,    -1,    -1,   155,   156,    -1,   158,   159,   881,   161,
     162,   163,    -1,    -1,    -1,    -1,   889,  1080,    -1,   892,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,   910,    -1,    -1,
      -1,   914,   115,   116,   117,   118,   119,   120,    -1,    -1,
     130,   131,    -1,   126,   127,    -1,    -1,  1120,    -1,    -1,
      -1,    -1,  1125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1139,  1140,    -1,    -1,
      -1,    -1,    -1,    -1,   957,   958,    -1,    -1,    -1,   162,
      -1,   164,    -1,    -1,    -1,    -1,    -1,    -1,   971,    -1,
      -1,   974,    -1,   976,   177,    -1,   179,   187,    -1,   182,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   991,    -1,
     993,    -1,    -1,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1214,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   211,    -1,    -1,  1038,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,  1068,    54,  1070,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,  1080,    66,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,  1097,    67,    -1,  1100,    -1,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    -1,    -1,  1302,
      -1,    -1,    -1,    -1,    -1,    87,    -1,  1120,    -1,    -1,
      -1,    -1,  1125,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      64,    65,    -1,    -1,    -1,    -1,  1139,  1140,    -1,  1142,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1151,    -1,
      -1,    -1,  1155,    -1,    -1,  1158,    -1,  1160,   337,    -1,
      -1,    -1,   134,    -1,    -1,    -1,    -1,   346,    -1,    -1,
      -1,    -1,    -1,  1176,   353,   147,    -1,    -1,  1371,    -1,
      -1,   360,    -1,    -1,    -1,    -1,   158,   159,    -1,   161,
     162,   163,   371,    10,    11,    12,   130,   131,    -1,    -1,
      -1,    -1,   190,    -1,   176,  1208,  1209,    -1,  1211,    -1,
      -1,  1214,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    66,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   442,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,  1294,    -1,  1296,    -1,    -1,    -1,    -1,  1301,  1302,
      -1,    66,  1305,   482,  1307,    -1,    -1,  1310,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1318,  1319,    -1,    -1,  1322,
      -1,    -1,    -1,    -1,    -1,    -1,  1329,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,   258,    54,   260,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,   537,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1371,    -1,
      -1,    -1,    -1,   190,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1386,    -1,    -1,    -1,    -1,    -1,   568,
      -1,    -1,  1395,  1396,    -1,    -1,    -1,    -1,    -1,    -1,
    1403,   315,  1405,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1427,   190,  1429,    -1,    -1,    -1,
      -1,    -1,  1435,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,   644,    -1,  1470,  1471,    -1,
      66,    -1,    -1,  1476,    -1,  1478,    10,    11,    12,    -1,
      -1,  1484,   396,  1486,    -1,   399,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,   701,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,   741,    -1,   743,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,  1585,    64,    65,    -1,    -1,    -1,    -1,    -1,
      -1,   770,   771,    -1,   190,    66,    64,    65,    -1,  1602,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   786,   787,   788,
     789,   790,    -1,    -1,    -1,   794,  1619,    -1,    -1,   533,
     534,    -1,  1625,   537,    -1,    -1,    -1,   806,    -1,    -1,
      -1,    -1,    -1,  1636,    -1,    -1,    -1,    -1,    -1,  1642,
      -1,    -1,    -1,  1646,   823,    -1,    -1,    -1,    -1,   130,
     131,    -1,    -1,    -1,   568,    -1,   190,   836,    -1,   838,
     839,    -1,   130,   131,  1667,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   853,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   862,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
     614,   615,    -1,  1706,    -1,    -1,   187,    -1,    -1,   623,
     889,    66,  1715,   892,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,  1731,    -1,
      -1,    -1,    10,    11,    12,   914,    -1,  1740,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,  1755,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,   957,   958,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   991,    -1,   993,    -1,    -1,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   771,    -1,  1038,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     784,   785,   786,   787,   788,   789,   790,    -1,    -1,    -1,
     794,    -1,    -1,    -1,    26,    27,    -1,    -1,    30,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,  1080,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   190,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,   852,    -1,
      -1,  1120,    -1,    -1,    -1,    -1,  1125,    -1,    66,    -1,
      -1,    -1,    -1,    -1,   868,    -1,    -1,    -1,    -1,    -1,
    1139,  1140,    -1,  1142,    -1,    -1,    -1,   881,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   889,    -1,    -1,    -1,  1158,
      -1,  1160,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   910,  1176,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,  1211,    -1,    -1,  1214,    -1,    -1,    66,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,   971,    54,    -1,
     974,    -1,   976,    -1,    -1,    -1,    -1,    -1,    -1,   211,
      66,    -1,   190,    -1,    -1,    -1,    -1,   991,    -1,    -1,
      -1,    -1,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1301,  1302,  1038,    -1,  1305,    -1,  1307,    -1,
      -1,  1310,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1319,    -1,    -1,  1322,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1068,    -1,  1070,    -1,    -1,    -1,
      -1,    -1,   190,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,  1097,    54,    -1,  1100,    -1,    -1,    -1,
      -1,    -1,  1371,    -1,    -1,   337,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   346,    -1,    -1,  1386,    -1,    -1,
      -1,   353,    -1,    -1,    -1,    -1,  1395,  1396,   360,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1405,    -1,  1142,   371,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1151,    -1,    -1,
      -1,  1155,    -1,    -1,  1158,    -1,  1160,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1176,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,  1470,  1471,    -1,  1208,  1209,    -1,  1476,    -1,  1478,
     442,    -1,    -1,    66,    -1,    -1,    -1,  1486,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    29,
     482,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    65,    -1,    66,    -1,    -1,    -1,
    1294,    -1,  1296,    -1,    -1,    -1,    -1,  1301,    -1,    -1,
      -1,  1305,    -1,  1307,    -1,   537,  1310,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1318,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1329,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   187,    -1,   568,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,  1625,    -1,    -1,    -1,
     537,    -1,    -1,    -1,    -1,    -1,    -1,  1636,    -1,    -1,
      -1,    -1,    -1,  1642,    -1,    -1,    -1,  1646,    -1,    -1,
      -1,    -1,  1386,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   568,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1403,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     190,    -1,   644,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1427,    -1,  1429,    -1,    -1,    -1,    -1,
      -1,  1435,    -1,    -1,    -1,    -1,    -1,  1706,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1715,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1731,    10,    11,    12,  1470,  1471,    -1,   701,
      -1,    -1,  1476,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1484,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,   741,
      -1,   743,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   770,   771,
      -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    -1,   786,   787,   788,   789,   790,   537,
      -1,    -1,   794,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,    -1,
      -1,  1585,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
     568,   823,    -1,    -1,   771,    -1,    99,    -1,  1602,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   838,   839,    -1,   786,
     787,   788,   789,   790,    -1,  1619,    -1,   794,    -1,    -1,
      -1,  1625,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     862,   134,  1636,   136,   137,   138,   139,   140,  1642,    -1,
      -1,    -1,  1646,   190,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,   889,   161,   162,
     163,    -1,    -1,  1667,   167,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,    -1,
      -1,    -1,   914,   186,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,  1706,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   889,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   957,   958,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1740,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1755,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   991,
      -1,   993,    -1,    -1,   996,   997,   998,   999,  1000,  1001,
    1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,    -1,   771,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1038,    -1,   786,   787,
     788,   789,    -1,    -1,   991,    35,   794,    -1,    -1,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,    67,  1080,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    -1,
      80,  1038,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1120,    -1,
      -1,    -1,    -1,  1125,    -1,    -1,    -1,    -1,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,  1139,  1140,    -1,
    1142,   889,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,  1158,   147,  1160,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,  1176,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,    -1,  1142,   186,    -1,    -1,    -1,
      -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1158,  1214,  1160,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1038,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,  1301,
    1302,    -1,    -1,  1305,    -1,  1307,    -1,    -1,  1310,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,  1301,    -1,    -1,    -1,  1305,    -1,
    1307,    -1,    -1,  1310,    -1,    -1,    -1,    -1,    -1,  1371,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1386,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1158,    -1,  1160,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,  1176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1386,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    10,    11,    12,  1470,  1471,
     188,    -1,    -1,    -1,  1476,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1485,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,  1470,  1471,    -1,    -1,    -1,    -1,  1476,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1301,    -1,    -1,    -1,  1305,    -1,  1307,
      -1,    29,  1310,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   188,    66,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,  1625,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1636,    -1,    66,    -1,  1386,    -1,
    1642,    -1,    -1,   188,  1646,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,  1669,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1625,    -1,
      66,     3,     4,     5,     6,     7,    -1,    -1,    -1,  1636,
      -1,    13,    -1,    -1,    -1,  1642,    -1,    -1,    -1,  1646,
      -1,    -1,    -1,    -1,  1706,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     188,    -1,  1470,  1471,    46,    47,    48,    -1,  1476,    -1,
      -1,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    71,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,  1706,
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
     182,   183,    -1,    -1,   186,    -1,   188,   189,   190,   191,
     192,    -1,   194,   195,    -1,    -1,    -1,  1625,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,  1636,    -1,
      13,    -1,    -1,    -1,  1642,    -1,    -1,    -1,  1646,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,  1706,    82,
      83,    -1,    -1,    -1,    87,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,   107,   108,   109,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,   164,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,   177,    -1,   179,    -1,   181,   182,
     183,    -1,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,   195,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,
      -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
     100,    -1,    -1,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,   115,   116,   117,   118,   119,
     120,    -1,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,
      -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,   189,
      -1,   191,   192,    -1,   194,   195,     3,     4,     5,     6,
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
      -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,
      -1,   188,   189,   190,   191,   192,    -1,   194,   195,     3,
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
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,
      -1,    -1,   186,    -1,   188,   189,   190,   191,   192,    -1,
     194,   195,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     181,   182,   183,    -1,    -1,   186,    -1,   188,   189,   190,
     191,   192,    -1,   194,   195,     3,     4,     5,     6,     7,
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
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,
     188,   189,   190,   191,   192,    -1,   194,   195,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    71,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    80,    -1,    82,    83,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    -1,    94,
      -1,    96,    -1,    -1,    99,   100,    -1,    -1,    -1,   104,
     105,   106,   107,    -1,   109,   110,    -1,   112,    -1,   114,
     115,   116,   117,   118,   119,   120,    -1,   122,   123,   124,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,   144,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,   164,
      -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,   186,    -1,   188,   189,    -1,   191,   192,    -1,   194,
     195,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,
      -1,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    71,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    -1,
      82,    83,    -1,    -1,    -1,    87,    88,    89,    90,    -1,
      92,    -1,    94,    -1,    96,    97,    -1,    99,   100,    -1,
      -1,    -1,   104,   105,   106,   107,    -1,   109,   110,    -1,
     112,    -1,   114,   115,   116,   117,   118,   119,   120,    -1,
     122,   123,   124,    -1,   126,   127,    -1,    -1,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,   164,    -1,    -1,   167,    -1,    -1,   170,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,
     182,   183,    -1,    -1,   186,    -1,   188,   189,    -1,   191,
     192,    -1,   194,   195,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,
     189,   190,   191,   192,    -1,   194,   195,     3,     4,     5,
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
     176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,
     186,    -1,   188,   189,   190,   191,   192,    -1,   194,   195,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,    82,
      83,    -1,    -1,    -1,    87,    88,    89,    90,    -1,    92,
      -1,    94,    95,    96,    -1,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,    -1,   112,
      -1,   114,   115,   116,   117,   118,   119,   120,    -1,   122,
     123,   124,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,   164,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
     183,    -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,   195,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,   189,
     190,   191,   192,    -1,   194,   195,     3,     4,     5,     6,
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
      -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,
      -1,   188,   189,   190,   191,   192,    -1,   194,   195,     3,
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
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,
      -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,    -1,
     194,   195,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     181,   182,   183,    -1,    -1,   186,    -1,   188,   189,   190,
     191,   192,    -1,   194,   195,     3,     4,     5,     6,     7,
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
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,
     188,   189,   190,   191,   192,    -1,   194,   195,     3,     4,
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
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,   186,    -1,   188,   189,   190,   191,   192,    -1,   194,
     195,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
     182,   183,    -1,    -1,   186,    -1,   188,   189,    -1,   191,
     192,    -1,   194,   195,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,
     189,    -1,   191,   192,    -1,   194,   195,     3,     4,     5,
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
     176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,
     186,    -1,   188,   189,    -1,   191,   192,    -1,   194,   195,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,    82,
      83,    -1,    -1,    -1,    87,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,    -1,   112,
      -1,   114,   115,   116,   117,   118,   119,   120,    -1,   122,
     123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
     183,    -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,
      -1,   194,   195,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,
      -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
     100,    -1,    -1,    -1,   104,   105,   106,   107,    -1,   109,
     110,    -1,   112,    -1,   114,   115,   116,   117,   118,   119,
     120,    -1,   122,   123,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,   189,
      -1,   191,   192,    -1,   194,   195,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    71,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    80,    -1,    82,    83,    -1,    -1,    -1,
      87,    88,    89,    90,    -1,    92,    -1,    94,    -1,    96,
      -1,    -1,    99,   100,    -1,    -1,    -1,   104,   105,   106,
     107,    -1,   109,   110,    -1,   112,    -1,   114,   115,   116,
     117,   118,   119,   120,    -1,   122,   123,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
     167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,
      -1,   188,   189,    -1,   191,   192,    -1,   194,   195,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,    53,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    71,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    80,    -1,    82,    83,
      -1,    -1,    -1,    87,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,    -1,
     104,   105,   106,   107,    -1,   109,   110,    -1,   112,    -1,
     114,   115,   116,   117,   118,   119,   120,    -1,   122,   123,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
     144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,
      -1,    -1,   186,    -1,   188,   189,    -1,   191,   192,    -1,
     194,   195,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
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
     181,   182,   183,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,    -1,   194,   195,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,
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
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,
     188,    -1,    -1,   191,   192,    -1,   194,   195,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,
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
     165,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,    -1,   194,
     195,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    -1,
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
     182,   183,    -1,    -1,   186,    -1,    -1,   189,    -1,   191,
     192,    -1,   194,   195,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    47,    48,
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
     159,    -1,   161,   162,   163,    -1,   165,    -1,   167,    -1,
      -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,    -1,   194,   195,     3,     4,     5,
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
     176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,
     186,    -1,    10,    11,    12,   191,   192,    -1,   194,   195,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    66,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,   104,    -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   115,   116,   117,   118,   119,   120,    -1,    -1,
     123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
     188,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
     183,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
      -1,   194,   195,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,   181,   182,   183,    -1,    -1,   186,    -1,    10,    11,
      12,   191,   192,    -1,   194,   195,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    -1,    -1,    66,    -1,    53,    -1,    55,    56,
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
     167,    -1,    -1,   170,    -1,    -1,   188,    -1,    -1,   176,
      -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,
      -1,   188,    11,    12,   191,   192,    -1,   194,   195,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    -1,    -1,    66,    -1,    53,
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
      -1,    -1,   186,    -1,   188,    -1,    -1,   191,   192,    -1,
     194,   195,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
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
     181,   182,   183,    -1,    -1,   186,    -1,    10,    11,    12,
     191,   192,    -1,   194,   195,     3,     4,     5,     6,     7,
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
      -1,    -1,   170,    -1,   187,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,   187,
      -1,    -1,    -1,   191,   192,    -1,   194,   195,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,
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
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,    -1,   194,
     195,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    47,    48,    -1,    -1,    -1,
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
     182,   183,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
     192,    -1,   194,   195,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
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
      -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,    -1,   194,   195,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,
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
     176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,    -1,   194,   195,
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
     183,    -1,    -1,   186,    -1,    10,    11,    12,   191,   192,
      -1,   194,   195,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,
      -1,    66,    -1,    53,    -1,    55,    56,    57,    58,    59,
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
     170,    -1,   187,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,   181,   182,   183,    -1,    -1,   186,    -1,    10,    11,
      12,   191,   192,    -1,   194,   195,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    -1,    -1,    66,    -1,    53,    -1,    55,    56,
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
      -1,   158,   159,    -1,   161,   162,   163,    -1,     3,     4,
     167,     6,     7,   170,    -1,    10,    11,    12,    13,   176,
      -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,
      -1,    -1,    27,    -1,   191,   192,    -1,   194,   195,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    68,    69,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   127,   128,   129,    -1,    -1,    -1,   133,   134,
     135,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,
      -1,    -1,    -1,   158,   159,    27,   161,   162,   163,   164,
      -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    -1,   189,    -1,   191,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,   126,   127,   128,   129,    -1,    -1,
      -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
     152,    -1,    -1,    -1,    -1,    -1,   158,   159,    -1,   161,
     162,   163,   164,    27,   166,    29,    -1,   169,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   189,    -1,   191,
      54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,    -1,   127,   128,   129,    -1,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   152,    -1,
      -1,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
     164,    27,   166,    29,    -1,   169,    -1,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,   190,    -1,    54,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    69,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
      -1,   127,   128,   129,    -1,    -1,    -1,   133,   134,   135,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,   152,    -1,    -1,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    27,
     166,    29,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,   190,    -1,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    69,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,   126,   127,
     128,   129,    -1,    -1,    -1,   133,   134,   135,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,   152,    -1,    -1,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,   164,    27,   166,    29,
      -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,    -1,    54,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,    -1,   127,   128,   129,
      -1,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,   152,    -1,    -1,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,   164,    27,   166,    29,    -1,   169,
      -1,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,    -1,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,    -1,   127,   128,   129,    -1,    -1,
      -1,   133,   134,   135,    75,    76,    77,    78,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
     152,    -1,    -1,   155,   156,    -1,   158,   159,    99,   161,
     162,   163,   164,    -1,   166,    -1,    -1,   169,     3,     4,
       5,     6,     7,    -1,   176,    10,    11,    12,    13,   120,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    54,
     161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    69,    70,    71,    72,    73,    74,
     181,    -1,    -1,    78,    -1,   186,    -1,    -1,    -1,    -1,
     191,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   152,   153,   154,
      -1,    -1,    -1,   158,   159,    -1,   161,   162,   163,   164,
      27,   166,   167,    -1,   169,    -1,    -1,    -1,    -1,    -1,
      -1,   176,   177,    -1,   179,    -1,   181,   182,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    69,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,   126,
     127,   128,   129,    -1,    -1,    -1,   133,   134,   135,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,
      -1,   158,   159,    27,   161,   162,   163,   164,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,   126,   127,   128,   129,    -1,    -1,    -1,   133,
     134,   135,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   152,    -1,
      -1,    -1,    -1,    -1,   158,   159,    27,   161,   162,   163,
     164,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,    -1,    -1,    -1,   127,   128,   129,    -1,
      -1,    -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    -1,    -1,
      53,   152,    55,    -1,    -1,    -1,    -1,   158,   159,    -1,
     161,   162,   163,   164,    67,   166,    -1,    -1,   169,    -1,
      -1,    -1,    75,    76,    77,    78,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    99,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   134,    66,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    67,   161,   162,
     163,    -1,    -1,    -1,   167,    75,    76,    77,    78,    -1,
      80,    -1,    -1,   176,    -1,    -1,    -1,    87,   181,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      67,   161,   162,   163,    -1,    -1,    -1,   167,    75,    76,
      77,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      87,   181,    -1,    -1,    -1,    -1,   186,    -1,    -1,   189,
      -1,   191,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    67,    -1,    69,
     167,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    -1,
      80,    -1,    -1,    -1,   181,    -1,    -1,    87,    -1,   186,
      -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      67,   161,   162,   163,    -1,    -1,    -1,   167,    75,    76,
      77,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      87,   181,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    67,   161,   162,   163,    -1,    -1,    -1,
     167,    75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    87,   181,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,   167,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    29,   191,    31,    32,
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
      51,    52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   132,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   132,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   132,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   132,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   132,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    75,    76,    77,    78,    -1,    80,
      -1,    -1,    -1,    -1,    66,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
     132,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    -1,   186,    -1,    -1,    28,    29,
     191,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    98,    54,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    12,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    66,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   197,   198,     0,   199,     3,     4,     5,     6,     7,
      13,    27,    28,    46,    47,    48,    53,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    67,    68,
      69,    70,    71,    75,    76,    77,    78,    79,    80,    82,
      83,    87,    88,    89,    90,    92,    94,    96,    99,   100,
     104,   105,   106,   107,   108,   109,   110,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   122,   123,   124,   125,
     126,   127,   133,   134,   136,   137,   138,   139,   140,   144,
     147,   152,   153,   154,   155,   156,   158,   159,   161,   162,
     163,   164,   167,   170,   176,   177,   179,   181,   182,   183,
     186,   188,   189,   191,   192,   194,   195,   200,   203,   212,
     213,   214,   215,   216,   217,   220,   236,   237,   241,   244,
     249,   255,   315,   316,   324,   328,   329,   330,   331,   332,
     333,   334,   335,   337,   340,   352,   353,   354,   356,   357,
     359,   369,   370,   371,   373,   378,   381,   400,   408,   410,
     411,   412,   413,   414,   415,   416,   417,   418,   419,   420,
     421,   435,   437,   439,   118,   119,   120,   133,   152,   162,
     186,   203,   236,   315,   334,   412,   334,   186,   334,   334,
     334,   104,   334,   334,   398,   399,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,    80,    87,
     120,   147,   186,   213,   353,   370,   373,   378,   412,   415,
     412,    35,   334,   426,   427,   334,   120,   186,   213,   370,
     371,   372,   374,   378,   409,   410,   411,   419,   423,   424,
     186,   325,   375,   186,   325,   344,   326,   334,   222,   325,
     186,   186,   186,   325,   188,   334,   203,   188,   334,     3,
       4,     6,     7,    10,    11,    12,    13,    27,    29,    54,
      56,    68,    69,    70,    71,    72,    73,    74,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   126,   127,   128,   129,   133,   134,   135,   152,
     156,   164,   166,   169,   176,   186,   203,   204,   205,   216,
     440,   455,   456,   458,   188,   331,   334,   189,   229,   334,
     107,   108,   155,   206,   209,   212,    80,   191,   281,   282,
     119,   126,   118,   126,    80,   283,   186,   186,   186,   186,
     203,   253,   443,   186,   186,   326,    80,    86,   148,   149,
     150,   432,   433,   155,   189,   212,   212,   203,   254,   443,
     156,   186,   443,   443,    80,   183,   189,   345,    27,   324,
     328,   334,   335,   412,   416,   218,   189,   421,    86,   376,
     432,    86,   432,   432,    30,   155,   172,   444,   186,     9,
     188,    35,   235,   156,   252,   443,   120,   182,   236,   316,
     188,   188,   188,   188,   188,   188,    10,    11,    12,    29,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    54,    66,   188,    67,    67,   189,   151,   127,
     162,   164,   177,   179,   255,   314,   315,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      64,    65,   130,   131,   402,    67,   189,   407,   186,   186,
      67,   189,   186,   235,   236,    14,   334,   188,   132,    45,
     203,   397,    86,   324,   335,   151,   412,   132,   193,     9,
     383,   324,   335,   412,   444,   151,   186,   377,   402,   407,
     187,   334,    30,   220,     8,   346,     9,   188,   220,   221,
     326,   327,   334,   203,   267,   224,   188,   188,   188,   134,
     135,   458,   458,   172,   186,   107,   458,    14,   151,   134,
     135,   152,   203,   205,   188,   188,   230,   111,   169,   188,
     155,   207,   210,   212,   155,   208,   211,   212,   212,     9,
     188,    98,   189,   412,     9,   188,   126,   126,    14,     9,
     188,   412,   436,   326,   324,   335,   412,   415,   416,   187,
     172,   247,   133,   412,   425,   426,   188,    67,   402,   148,
     433,    79,   334,   412,    86,   148,   433,   212,   202,   188,
     189,   242,   250,   360,   362,    87,   186,   347,   348,   350,
     373,   418,   420,   437,    14,    98,   438,   341,   342,   343,
     277,   278,   400,   401,   187,   187,   187,   187,   187,   190,
     219,   220,   237,   244,   249,   400,   334,   192,   194,   195,
     203,   445,   446,   458,    35,   165,   279,   280,   334,   440,
     186,   443,   245,   235,   334,   334,   334,    30,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   374,   334,   334,   422,   422,   334,   428,   429,   126,
     189,   204,   205,   421,   253,   203,   254,   443,   443,   252,
     236,    35,   328,   331,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   156,   189,   203,
     403,   404,   405,   406,   421,   422,   334,   279,   279,   422,
     334,   425,   235,   187,   334,   186,   396,     9,   383,   187,
     187,    35,   334,    35,   334,   187,   187,   187,   419,   420,
     421,   279,   189,   203,   403,   404,   421,   187,   218,   271,
     189,   331,   334,   334,    90,    30,   220,   265,   188,    28,
      98,    14,     9,   187,    30,   189,   268,   458,    29,    87,
     216,   452,   453,   454,   186,     9,    47,    48,    53,    55,
      67,   134,   156,   176,   186,   213,   214,   216,   355,   370,
     378,   379,   380,   203,   457,   218,   186,   228,   212,     9,
     188,    98,   212,     9,   188,    98,    98,   209,   203,   334,
     282,   379,    80,     9,   187,   187,   187,   187,   187,   187,
     187,   188,    47,    48,   450,   451,   128,   258,   186,     9,
     187,   187,    80,    81,   203,   434,   203,    67,   190,   190,
     199,   201,    30,   129,   257,   171,    51,   156,   171,   364,
     335,   132,     9,   383,   187,   151,   458,   458,    14,   346,
     277,   218,   184,     9,   384,   458,   459,   402,   407,   402,
     190,     9,   383,   173,   412,   334,   187,     9,   384,    14,
     338,   238,   128,   256,   186,   443,   334,    30,   193,   193,
     132,   190,     9,   383,   334,   444,   186,   248,   243,   251,
      14,   438,   246,   235,    69,   412,   334,   444,   193,   190,
     187,   187,   193,   190,   187,    47,    48,    67,    75,    76,
      77,    87,   134,   147,   176,   203,   386,   388,   389,   392,
     395,   203,   412,   412,   132,   402,   407,   187,   334,   272,
      72,    73,   273,   218,   325,   218,   327,    98,    35,   133,
     262,   412,   379,   203,    30,   220,   266,   188,   269,   188,
     269,     9,   173,    87,   132,   151,     9,   383,   187,   165,
     445,   446,   447,   445,   379,   379,   379,   379,   379,   382,
     385,   186,   151,   186,   379,   151,   189,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    66,   151,   444,   190,   370,   189,   232,
      98,   210,   203,    98,   211,   203,   203,   190,    14,   412,
     188,     9,   173,   203,   259,   370,   189,   425,   133,   412,
      14,   193,   334,   190,   199,   458,   259,   189,   363,    14,
     187,   334,   347,   421,   188,   458,   184,   190,    30,   448,
     401,    35,    80,   165,   403,   404,   406,   403,   404,   458,
      35,   165,   334,   379,   277,   186,   370,   257,   339,   239,
     334,   334,   334,   190,   186,   279,   258,    30,   257,   458,
      14,   256,   443,   374,   190,   186,    14,    75,    76,    77,
     203,   387,   387,   389,   390,   391,    49,   186,    86,   148,
     186,     9,   383,   187,   396,    35,   334,   190,    72,    73,
     274,   325,   220,   190,   188,    91,   188,   262,   412,   186,
     132,   261,    14,   218,   269,   101,   102,   103,   269,   190,
     458,   132,   458,   203,   452,     9,   187,   383,   132,   193,
       9,   383,   382,   204,   347,   349,   351,   187,   126,   204,
     379,   430,   431,   379,   379,   379,    30,   379,   379,   379,
     379,   379,   379,   379,   379,   379,   379,   379,   379,   379,
     379,   379,   379,   379,   379,   379,   379,   379,   379,   379,
     379,   457,    80,   233,   203,   203,   379,   451,    98,    99,
     449,     9,   287,   187,   186,   328,   331,   334,   193,   190,
     438,   287,   157,   170,   189,   359,   366,   157,   189,   365,
     132,   188,   448,   458,   346,   459,    80,   165,    14,    80,
     444,   412,   334,   187,   277,   189,   277,   186,   132,   186,
     279,   187,   189,   458,   189,   188,   458,   257,   240,   377,
     279,   132,   193,     9,   383,   388,   390,   148,   347,   393,
     394,   389,   412,   325,    30,    74,   220,   188,   327,   261,
     425,   262,   187,   379,    97,   101,   188,   334,    30,   188,
     270,   190,   173,   458,   132,   165,    30,   187,   379,   379,
     187,   132,     9,   383,   187,   132,   190,     9,   383,   379,
      30,   187,   218,   203,   458,   458,   370,     4,   108,   113,
     119,   121,   158,   159,   161,   190,   288,   313,   314,   315,
     320,   321,   322,   323,   400,   425,   190,   189,   190,    51,
     334,   334,   334,   346,    35,    80,   165,    14,    80,   334,
     186,   448,   187,   287,   187,   277,   334,   279,   187,   287,
     438,   287,   188,   189,   186,   187,   389,   389,   187,   132,
     187,     9,   383,    30,   218,   188,   187,   187,   187,   225,
     188,   188,   270,   218,   458,   458,   132,   379,   347,   379,
     379,   379,   189,   190,   449,   128,   129,   177,   204,   441,
     458,   260,   370,   108,   323,    29,   121,   134,   135,   156,
     162,   297,   298,   299,   300,   370,   160,   305,   306,   124,
     186,   203,   307,   308,   289,   236,   458,     9,   188,     9,
     188,   188,   438,   314,   187,   284,   156,   361,   190,   190,
      80,   165,    14,    80,   334,   279,   113,   336,   448,   190,
     448,   187,   187,   190,   189,   190,   287,   277,   132,   389,
     347,   218,   223,   226,    30,   220,   264,   218,   187,   379,
     132,   132,   218,   370,   370,   443,    14,   204,     9,   188,
     189,   441,   438,   300,   172,   189,     9,   188,     3,     4,
       5,     6,     7,    10,    11,    12,    13,    27,    28,    54,
      68,    69,    70,    71,    72,    73,    74,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   133,   134,
     136,   137,   138,   139,   140,   152,   153,   154,   164,   166,
     167,   169,   176,   177,   179,   181,   182,   203,   367,   368,
       9,   188,   156,   160,   203,   308,   309,   310,   188,    80,
     319,   235,   290,   441,   441,    14,   236,   190,   285,   286,
     441,    14,    80,   334,   187,   186,   189,   188,   189,   311,
     336,   448,   284,   190,   187,   389,   132,    30,   220,   263,
     264,   218,   379,   379,   190,   188,   188,   379,   370,   293,
     458,   301,   302,   378,   298,    14,    30,    48,   303,   306,
       9,    33,   187,    29,    47,    50,    14,     9,   188,   205,
     442,   319,    14,   458,   235,   188,    14,   334,    35,    80,
     358,   218,   218,   189,   311,   190,   448,   389,   218,    95,
     231,   190,   203,   216,   294,   295,   296,     9,   173,     9,
     383,   190,   379,   368,   368,    56,   304,   309,   309,    29,
      47,    50,   379,    80,   172,   186,   188,   379,   443,   379,
      80,     9,   384,   190,   190,   218,   311,    93,   188,   111,
     227,   151,    98,   458,   378,   163,    14,   450,   291,   186,
      35,    80,   187,   190,   188,   186,   169,   234,   203,   314,
     315,   173,   379,   173,   275,   276,   401,   292,    80,   370,
     232,   166,   203,   188,   187,     9,   384,   115,   116,   117,
     317,   318,   275,    80,   260,   188,   448,   401,   459,   187,
     187,   188,   188,   189,   312,   317,    35,    80,   165,   448,
     189,   218,   459,    80,   165,    14,    80,   312,   218,   190,
      35,    80,   165,    14,    80,   334,   190,    80,   165,    14,
      80,   334,    14,    80,   334,   334
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
#line 725 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 728 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 735 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 736 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 739 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 740 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 741 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 742 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 743 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 745 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 848 "hphp.y"
    { ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 849 "hphp.y"
    { ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 854 "hphp.y"
    { ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 855 "hphp.y"
    { ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 860 "hphp.y"
    { ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 861 "hphp.y"
    { ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 865 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 866 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 867 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 869 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 885 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 894 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 897 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1030 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { (yyval).reset();;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1054 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1062 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1063 "hphp.y"
    { (yyval).reset();;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { (yyval).reset();;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1072 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1091 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1097 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1104 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1110 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1130 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1136 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
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

  case 196:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
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

  case 198:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1212 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1227 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { (yyval).reset();;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { (yyval).reset();;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyval).reset();;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { (yyval).reset();;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { (yyval).reset();;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { (yyval).reset();;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { (yyval).reset();;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { (yyval).reset();;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1318 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    { (yyval).reset();;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { (yyval).reset();;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1378 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1383 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1428 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { (yyval).reset();;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { (yyval).reset();;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1491 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { (yyval).reset();;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { (yyval).reset();;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { (yyval).reset();;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { (yyval).reset();;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { (yyval).reset();;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { (yyval).reset();;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval).reset();;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval).reset();;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { (yyval).reset();;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
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

  case 513:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
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

  case 515:

/* Line 1455 of yacc.c  */
#line 2017 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
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

  case 517:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2042 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
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

  case 552:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
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

  case 553:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval).reset();;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval).reset();;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval).reset();;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { (yyval).reset();;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval).reset();;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval).reset();;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { (yyval).reset();;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { (yyval).reset();;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval).reset();;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval).reset();;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval).reset();;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval).reset();;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval).reset();;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
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

  case 807:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
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

  case 809:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
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
#line 2726 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
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
#line 2750 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
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

  case 826:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
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
#line 2793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2816 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2868 "hphp.y"
    { (yyval).reset();;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
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

  case 860:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
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

  case 866:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { (yyval).reset();;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
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

  case 902:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3090 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3106 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    {;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    {;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    {;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3164 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3217 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3221 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3224 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3230 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3237 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3240 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3252 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3261 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13659 "hphp.7.tab.cpp"
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
#line 3264 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

