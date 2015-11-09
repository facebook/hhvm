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
#line 872 "hphp.5.tab.cpp"

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
#define YYLAST   16506

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  196
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  266
/* YYNRULES -- Number of rules.  */
#define YYNRULES  984
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1795

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
    1708,  1713,  1715,  1718,  1721,  1724,  1727,  1730,  1733,  1736,
    1739,  1742,  1744,  1746,  1748,  1752,  1755,  1757,  1763,  1764,
    1765,  1777,  1778,  1791,  1792,  1797,  1798,  1806,  1807,  1813,
    1814,  1818,  1819,  1826,  1829,  1832,  1837,  1839,  1841,  1847,
    1851,  1857,  1861,  1864,  1865,  1868,  1869,  1874,  1879,  1883,
    1888,  1893,  1898,  1903,  1905,  1907,  1909,  1911,  1915,  1919,
    1924,  1926,  1929,  1934,  1937,  1944,  1945,  1947,  1952,  1953,
    1956,  1957,  1959,  1961,  1965,  1967,  1971,  1973,  1975,  1979,
    1983,  1985,  1987,  1989,  1991,  1993,  1995,  1997,  1999,  2001,
    2003,  2005,  2007,  2009,  2011,  2013,  2015,  2017,  2019,  2021,
    2023,  2025,  2027,  2029,  2031,  2033,  2035,  2037,  2039,  2041,
    2043,  2045,  2047,  2049,  2051,  2053,  2055,  2057,  2059,  2061,
    2063,  2065,  2067,  2069,  2071,  2073,  2075,  2077,  2079,  2081,
    2083,  2085,  2087,  2089,  2091,  2093,  2095,  2097,  2099,  2101,
    2103,  2105,  2107,  2109,  2111,  2113,  2115,  2117,  2119,  2121,
    2123,  2125,  2127,  2129,  2131,  2133,  2135,  2137,  2139,  2141,
    2143,  2148,  2150,  2152,  2154,  2156,  2158,  2160,  2164,  2166,
    2170,  2172,  2174,  2178,  2180,  2182,  2184,  2187,  2189,  2190,
    2191,  2193,  2195,  2199,  2200,  2202,  2204,  2206,  2208,  2210,
    2212,  2214,  2216,  2218,  2220,  2222,  2224,  2226,  2230,  2233,
    2235,  2237,  2242,  2246,  2251,  2253,  2255,  2259,  2263,  2267,
    2271,  2275,  2279,  2283,  2287,  2291,  2295,  2299,  2303,  2307,
    2311,  2315,  2319,  2323,  2327,  2330,  2333,  2336,  2339,  2343,
    2347,  2351,  2355,  2359,  2363,  2367,  2371,  2375,  2381,  2386,
    2390,  2394,  2398,  2400,  2402,  2404,  2406,  2410,  2414,  2418,
    2421,  2422,  2424,  2425,  2427,  2428,  2434,  2438,  2442,  2444,
    2446,  2448,  2450,  2454,  2457,  2459,  2461,  2463,  2465,  2467,
    2471,  2473,  2475,  2477,  2480,  2483,  2488,  2492,  2497,  2500,
    2501,  2507,  2511,  2515,  2517,  2521,  2523,  2526,  2527,  2533,
    2537,  2540,  2541,  2545,  2546,  2551,  2554,  2555,  2559,  2563,
    2565,  2566,  2568,  2570,  2572,  2574,  2578,  2580,  2582,  2584,
    2588,  2590,  2592,  2596,  2600,  2603,  2608,  2611,  2616,  2622,
    2628,  2634,  2640,  2642,  2644,  2646,  2648,  2650,  2652,  2656,
    2660,  2665,  2670,  2674,  2676,  2678,  2680,  2682,  2686,  2688,
    2693,  2697,  2699,  2701,  2703,  2705,  2707,  2711,  2715,  2720,
    2725,  2729,  2731,  2733,  2741,  2751,  2759,  2766,  2775,  2777,
    2780,  2785,  2790,  2792,  2794,  2799,  2801,  2802,  2804,  2807,
    2809,  2811,  2813,  2817,  2821,  2825,  2826,  2828,  2830,  2834,
    2838,  2841,  2845,  2852,  2853,  2855,  2860,  2863,  2864,  2870,
    2874,  2878,  2880,  2887,  2892,  2897,  2900,  2903,  2904,  2910,
    2914,  2918,  2920,  2923,  2924,  2930,  2934,  2938,  2940,  2943,
    2946,  2948,  2951,  2953,  2958,  2962,  2966,  2973,  2977,  2979,
    2981,  2983,  2988,  2993,  2998,  3003,  3008,  3013,  3016,  3019,
    3024,  3027,  3030,  3032,  3036,  3040,  3044,  3045,  3048,  3054,
    3061,  3068,  3076,  3078,  3081,  3083,  3086,  3088,  3093,  3095,
    3100,  3104,  3105,  3107,  3111,  3114,  3118,  3120,  3122,  3123,
    3124,  3127,  3130,  3133,  3138,  3141,  3147,  3151,  3153,  3155,
    3156,  3160,  3165,  3171,  3175,  3177,  3180,  3181,  3186,  3188,
    3192,  3195,  3198,  3201,  3203,  3205,  3207,  3209,  3213,  3218,
    3225,  3227,  3236,  3243,  3245
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     197,     0,    -1,    -1,   198,   199,    -1,   199,   200,    -1,
      -1,   219,    -1,   236,    -1,   243,    -1,   240,    -1,   250,
      -1,   441,    -1,   125,   186,   187,   188,    -1,   152,   212,
     188,    -1,    -1,   152,   212,   189,   201,   199,   190,    -1,
      -1,   152,   189,   202,   199,   190,    -1,   113,   206,   188,
      -1,   113,   107,   207,   188,    -1,   113,   108,   208,   188,
      -1,   216,   188,    -1,    78,    -1,    99,    -1,   158,    -1,
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
      -1,   213,   446,    -1,   213,   446,    -1,   216,     9,   442,
      14,   380,    -1,   108,   442,    14,   380,    -1,   217,   218,
      -1,    -1,   219,    -1,   236,    -1,   243,    -1,   250,    -1,
     189,   217,   190,    -1,    71,   326,   219,   272,   274,    -1,
      71,   326,    30,   217,   273,   275,    74,   188,    -1,    -1,
      90,   326,   220,   266,    -1,    -1,    89,   221,   219,    90,
     326,   188,    -1,    -1,    92,   186,   328,   188,   328,   188,
     328,   187,   222,   264,    -1,    -1,   100,   326,   223,   269,
      -1,   104,   188,    -1,   104,   335,   188,    -1,   106,   188,
      -1,   106,   335,   188,    -1,   109,   188,    -1,   109,   335,
     188,    -1,    27,   104,   188,    -1,   114,   282,   188,    -1,
     120,   284,   188,    -1,    88,   327,   188,    -1,   144,   327,
     188,    -1,   122,   186,   438,   187,   188,    -1,   188,    -1,
      82,    -1,    83,    -1,    -1,    94,   186,   335,    98,   263,
     262,   187,   224,   265,    -1,    -1,    94,   186,   335,    28,
      98,   263,   262,   187,   225,   265,    -1,    96,   186,   268,
     187,   267,    -1,    -1,   110,   228,   111,   186,   371,    80,
     187,   189,   217,   190,   230,   226,   233,    -1,    -1,   110,
     228,   169,   227,   231,    -1,   112,   335,   188,    -1,   105,
     203,   188,    -1,   335,   188,    -1,   329,   188,    -1,   330,
     188,    -1,   331,   188,    -1,   332,   188,    -1,   333,   188,
      -1,   109,   332,   188,    -1,   334,   188,    -1,   203,    30,
      -1,    -1,   189,   229,   217,   190,    -1,   230,   111,   186,
     371,    80,   187,   189,   217,   190,    -1,    -1,    -1,   189,
     232,   217,   190,    -1,   169,   231,    -1,    -1,    35,    -1,
      -1,   107,    -1,    -1,   235,   234,   445,   237,   186,   278,
     187,   450,   312,    -1,    -1,   316,   235,   234,   445,   238,
     186,   278,   187,   450,   312,    -1,    -1,   401,   315,   235,
     234,   445,   239,   186,   278,   187,   450,   312,    -1,    -1,
     162,   203,   241,    30,   460,   440,   189,   285,   190,    -1,
      -1,   401,   162,   203,   242,    30,   460,   440,   189,   285,
     190,    -1,    -1,   256,   253,   244,   257,   258,   189,   288,
     190,    -1,    -1,   401,   256,   253,   245,   257,   258,   189,
     288,   190,    -1,    -1,   127,   254,   246,   259,   189,   288,
     190,    -1,    -1,   401,   127,   254,   247,   259,   189,   288,
     190,    -1,    -1,   126,   249,   378,   257,   258,   189,   288,
     190,    -1,    -1,   164,   255,   251,   258,   189,   288,   190,
      -1,    -1,   401,   164,   255,   252,   258,   189,   288,   190,
      -1,   445,    -1,   156,    -1,   445,    -1,   445,    -1,   126,
      -1,   119,   126,    -1,   119,   118,   126,    -1,   118,   119,
     126,    -1,   118,   126,    -1,   128,   371,    -1,    -1,   129,
     260,    -1,    -1,   128,   260,    -1,    -1,   371,    -1,   260,
       9,   371,    -1,   371,    -1,   261,     9,   371,    -1,   132,
     263,    -1,    -1,   413,    -1,    35,   413,    -1,   133,   186,
     427,   187,    -1,   219,    -1,    30,   217,    93,   188,    -1,
     219,    -1,    30,   217,    95,   188,    -1,   219,    -1,    30,
     217,    91,   188,    -1,   219,    -1,    30,   217,    97,   188,
      -1,   203,    14,   380,    -1,   268,     9,   203,    14,   380,
      -1,   189,   270,   190,    -1,   189,   188,   270,   190,    -1,
      30,   270,   101,   188,    -1,    30,   188,   270,   101,   188,
      -1,   270,   102,   335,   271,   217,    -1,   270,   103,   271,
     217,    -1,    -1,    30,    -1,   188,    -1,   272,    72,   326,
     219,    -1,    -1,   273,    72,   326,    30,   217,    -1,    -1,
      73,   219,    -1,    -1,    73,    30,   217,    -1,    -1,   277,
       9,   402,   318,   461,   165,    80,    -1,   277,     9,   402,
     318,   461,    35,   165,    80,    -1,   277,     9,   402,   318,
     461,   165,    -1,   277,   385,    -1,   402,   318,   461,   165,
      80,    -1,   402,   318,   461,    35,   165,    80,    -1,   402,
     318,   461,   165,    -1,    -1,   402,   318,   461,    80,    -1,
     402,   318,   461,    35,    80,    -1,   402,   318,   461,    35,
      80,    14,   335,    -1,   402,   318,   461,    80,    14,   335,
      -1,   277,     9,   402,   318,   461,    80,    -1,   277,     9,
     402,   318,   461,    35,    80,    -1,   277,     9,   402,   318,
     461,    35,    80,    14,   335,    -1,   277,     9,   402,   318,
     461,    80,    14,   335,    -1,   279,     9,   402,   461,   165,
      80,    -1,   279,     9,   402,   461,    35,   165,    80,    -1,
     279,     9,   402,   461,   165,    -1,   279,   385,    -1,   402,
     461,   165,    80,    -1,   402,   461,    35,   165,    80,    -1,
     402,   461,   165,    -1,    -1,   402,   461,    80,    -1,   402,
     461,    35,    80,    -1,   402,   461,    35,    80,    14,   335,
      -1,   402,   461,    80,    14,   335,    -1,   279,     9,   402,
     461,    80,    -1,   279,     9,   402,   461,    35,    80,    -1,
     279,     9,   402,   461,    35,    80,    14,   335,    -1,   279,
       9,   402,   461,    80,    14,   335,    -1,   281,   385,    -1,
      -1,   335,    -1,    35,   413,    -1,   165,   335,    -1,   281,
       9,   335,    -1,   281,     9,   165,   335,    -1,   281,     9,
      35,   413,    -1,   282,     9,   283,    -1,   283,    -1,    80,
      -1,   191,   413,    -1,   191,   189,   335,   190,    -1,   284,
       9,    80,    -1,   284,     9,    80,    14,   380,    -1,    80,
      -1,    80,    14,   380,    -1,   285,   286,    -1,    -1,   287,
     188,    -1,   443,    14,   380,    -1,   288,   289,    -1,    -1,
      -1,   314,   290,   320,   188,    -1,    -1,   316,   460,   291,
     320,   188,    -1,   321,   188,    -1,   322,   188,    -1,   323,
     188,    -1,    -1,   315,   235,   234,   444,   186,   292,   276,
     187,   450,   313,    -1,    -1,   401,   315,   235,   234,   445,
     186,   293,   276,   187,   450,   313,    -1,   158,   298,   188,
      -1,   159,   306,   188,    -1,   161,   308,   188,    -1,     4,
     128,   371,   188,    -1,     4,   129,   371,   188,    -1,   113,
     261,   188,    -1,   113,   261,   189,   294,   190,    -1,   294,
     295,    -1,   294,   296,    -1,    -1,   215,   151,   203,   166,
     261,   188,    -1,   297,    98,   315,   203,   188,    -1,   297,
      98,   316,   188,    -1,   215,   151,   203,    -1,   203,    -1,
     299,    -1,   298,     9,   299,    -1,   300,   368,   304,   305,
      -1,   156,    -1,    29,   301,    -1,   301,    -1,   134,    -1,
     134,   172,   460,   173,    -1,   134,   172,   460,     9,   460,
     173,    -1,   371,    -1,   121,    -1,   162,   189,   303,   190,
      -1,   135,    -1,   379,    -1,   302,     9,   379,    -1,   302,
     384,    -1,    14,   380,    -1,    -1,    56,   163,    -1,    -1,
     307,    -1,   306,     9,   307,    -1,   160,    -1,   309,    -1,
     203,    -1,   124,    -1,   186,   310,   187,    -1,   186,   310,
     187,    50,    -1,   186,   310,   187,    29,    -1,   186,   310,
     187,    47,    -1,   309,    -1,   311,    -1,   311,    50,    -1,
     311,    29,    -1,   311,    47,    -1,   310,     9,   310,    -1,
     310,    33,   310,    -1,   203,    -1,   156,    -1,   160,    -1,
     188,    -1,   189,   217,   190,    -1,   188,    -1,   189,   217,
     190,    -1,   316,    -1,   121,    -1,   316,    -1,    -1,   317,
      -1,   316,   317,    -1,   115,    -1,   116,    -1,   117,    -1,
     120,    -1,   119,    -1,   118,    -1,   182,    -1,   319,    -1,
      -1,   115,    -1,   116,    -1,   117,    -1,   320,     9,    80,
      -1,   320,     9,    80,    14,   380,    -1,    80,    -1,    80,
      14,   380,    -1,   321,     9,   443,    14,   380,    -1,   108,
     443,    14,   380,    -1,   322,     9,   443,    -1,   119,   108,
     443,    -1,   119,   324,   440,    -1,   324,   440,    14,   460,
      -1,   108,   177,   445,    -1,   186,   325,   187,    -1,    69,
     375,   378,    -1,    69,   248,    -1,    68,   335,    -1,   360,
      -1,   355,    -1,   186,   335,   187,    -1,   327,     9,   335,
      -1,   335,    -1,   327,    -1,    -1,    27,    -1,    27,   335,
      -1,    27,   335,   132,   335,    -1,   186,   329,   187,    -1,
     413,    14,   329,    -1,   133,   186,   427,   187,    14,   329,
      -1,    28,   335,    -1,   413,    14,   332,    -1,   133,   186,
     427,   187,    14,   332,    -1,   336,    -1,   413,    -1,   325,
      -1,   417,    -1,   416,    -1,   133,   186,   427,   187,    14,
     335,    -1,   413,    14,   335,    -1,   413,    14,    35,   413,
      -1,   413,    14,    35,    69,   375,   378,    -1,   413,    26,
     335,    -1,   413,    25,   335,    -1,   413,    24,   335,    -1,
     413,    23,   335,    -1,   413,    22,   335,    -1,   413,    21,
     335,    -1,   413,    20,   335,    -1,   413,    19,   335,    -1,
     413,    18,   335,    -1,   413,    17,   335,    -1,   413,    16,
     335,    -1,   413,    15,   335,    -1,   413,    65,    -1,    65,
     413,    -1,   413,    64,    -1,    64,   413,    -1,   335,    31,
     335,    -1,   335,    32,   335,    -1,   335,    10,   335,    -1,
     335,    12,   335,    -1,   335,    11,   335,    -1,   335,    33,
     335,    -1,   335,    35,   335,    -1,   335,    34,   335,    -1,
     335,    49,   335,    -1,   335,    47,   335,    -1,   335,    48,
     335,    -1,   335,    50,   335,    -1,   335,    51,   335,    -1,
     335,    66,   335,    -1,   335,    52,   335,    -1,   335,    46,
     335,    -1,   335,    45,   335,    -1,    47,   335,    -1,    48,
     335,    -1,    53,   335,    -1,    55,   335,    -1,   335,    37,
     335,    -1,   335,    36,   335,    -1,   335,    39,   335,    -1,
     335,    38,   335,    -1,   335,    40,   335,    -1,   335,    44,
     335,    -1,   335,    41,   335,    -1,   335,    43,   335,    -1,
     335,    42,   335,    -1,   335,    54,   375,    -1,   186,   336,
     187,    -1,   335,    29,   335,    30,   335,    -1,   335,    29,
      30,   335,    -1,   437,    -1,    63,   335,    -1,    62,   335,
      -1,    61,   335,    -1,    60,   335,    -1,    59,   335,    -1,
      58,   335,    -1,    57,   335,    -1,    70,   376,    -1,    56,
     335,    -1,   382,    -1,   354,    -1,   353,    -1,   192,   377,
     192,    -1,    13,   335,    -1,   357,    -1,   113,   186,   359,
     385,   187,    -1,    -1,    -1,   235,   234,   186,   339,   278,
     187,   450,   337,   189,   217,   190,    -1,    -1,   316,   235,
     234,   186,   340,   278,   187,   450,   337,   189,   217,   190,
      -1,    -1,   182,    80,   342,   347,    -1,    -1,   182,   183,
     343,   278,   184,   450,   347,    -1,    -1,   182,   189,   344,
     217,   190,    -1,    -1,    80,   345,   347,    -1,    -1,   183,
     346,   278,   184,   450,   347,    -1,     8,   335,    -1,     8,
     332,    -1,     8,   189,   217,   190,    -1,    87,    -1,   439,
      -1,   349,     9,   348,   132,   335,    -1,   348,   132,   335,
      -1,   350,     9,   348,   132,   380,    -1,   348,   132,   380,
      -1,   349,   384,    -1,    -1,   350,   384,    -1,    -1,   176,
     186,   351,   187,    -1,   134,   186,   428,   187,    -1,    67,
     428,   193,    -1,   371,   189,   430,   190,    -1,   371,   189,
     432,   190,    -1,   357,    67,   423,   193,    -1,   358,    67,
     423,   193,    -1,   354,    -1,   439,    -1,   416,    -1,    87,
      -1,   186,   336,   187,    -1,   359,     9,    80,    -1,   359,
       9,    35,    80,    -1,    80,    -1,    35,    80,    -1,   170,
     156,   361,   171,    -1,   363,    51,    -1,   363,   171,   364,
     170,    51,   362,    -1,    -1,   156,    -1,   363,   365,    14,
     366,    -1,    -1,   364,   367,    -1,    -1,   156,    -1,   157,
      -1,   189,   335,   190,    -1,   157,    -1,   189,   335,   190,
      -1,   360,    -1,   369,    -1,   368,    30,   369,    -1,   368,
      48,   369,    -1,   203,    -1,    70,    -1,   107,    -1,   108,
      -1,   109,    -1,    27,    -1,    28,    -1,   110,    -1,   111,
      -1,   169,    -1,   112,    -1,    71,    -1,    72,    -1,    74,
      -1,    73,    -1,    90,    -1,    91,    -1,    89,    -1,    92,
      -1,    93,    -1,    94,    -1,    95,    -1,    96,    -1,    97,
      -1,    54,    -1,    98,    -1,   100,    -1,   101,    -1,   102,
      -1,   103,    -1,   104,    -1,   106,    -1,   105,    -1,    88,
      -1,    13,    -1,   126,    -1,   127,    -1,   128,    -1,   129,
      -1,    69,    -1,    68,    -1,   121,    -1,     5,    -1,     7,
      -1,     6,    -1,     4,    -1,     3,    -1,   152,    -1,   113,
      -1,   114,    -1,   123,    -1,   124,    -1,   125,    -1,   120,
      -1,   119,    -1,   118,    -1,   117,    -1,   116,    -1,   115,
      -1,   182,    -1,   122,    -1,   133,    -1,   134,    -1,    10,
      -1,    12,    -1,    11,    -1,   136,    -1,   138,    -1,   137,
      -1,   139,    -1,   140,    -1,   154,    -1,   153,    -1,   181,
      -1,   164,    -1,   167,    -1,   166,    -1,   177,    -1,   179,
      -1,   176,    -1,   214,   186,   280,   187,    -1,   215,    -1,
     156,    -1,   371,    -1,   379,    -1,   120,    -1,   421,    -1,
     186,   336,   187,    -1,   372,    -1,   373,   151,   420,    -1,
     372,    -1,   419,    -1,   374,   151,   420,    -1,   371,    -1,
     120,    -1,   425,    -1,   186,   187,    -1,   326,    -1,    -1,
      -1,    86,    -1,   434,    -1,   186,   280,   187,    -1,    -1,
      75,    -1,    76,    -1,    77,    -1,    87,    -1,   139,    -1,
     140,    -1,   154,    -1,   136,    -1,   167,    -1,   137,    -1,
     138,    -1,   153,    -1,   181,    -1,   147,    86,   148,    -1,
     147,   148,    -1,   379,    -1,   213,    -1,   134,   186,   383,
     187,    -1,    67,   383,   193,    -1,   176,   186,   352,   187,
      -1,   381,    -1,   356,    -1,   186,   380,   187,    -1,   380,
      31,   380,    -1,   380,    32,   380,    -1,   380,    10,   380,
      -1,   380,    12,   380,    -1,   380,    11,   380,    -1,   380,
      33,   380,    -1,   380,    35,   380,    -1,   380,    34,   380,
      -1,   380,    49,   380,    -1,   380,    47,   380,    -1,   380,
      48,   380,    -1,   380,    50,   380,    -1,   380,    51,   380,
      -1,   380,    52,   380,    -1,   380,    46,   380,    -1,   380,
      45,   380,    -1,   380,    66,   380,    -1,    53,   380,    -1,
      55,   380,    -1,    47,   380,    -1,    48,   380,    -1,   380,
      37,   380,    -1,   380,    36,   380,    -1,   380,    39,   380,
      -1,   380,    38,   380,    -1,   380,    40,   380,    -1,   380,
      44,   380,    -1,   380,    41,   380,    -1,   380,    43,   380,
      -1,   380,    42,   380,    -1,   380,    29,   380,    30,   380,
      -1,   380,    29,    30,   380,    -1,   215,   151,   204,    -1,
     156,   151,   204,    -1,   215,   151,   126,    -1,   213,    -1,
      79,    -1,   439,    -1,   379,    -1,   194,   434,   194,    -1,
     195,   434,   195,    -1,   147,   434,   148,    -1,   386,   384,
      -1,    -1,     9,    -1,    -1,     9,    -1,    -1,   386,     9,
     380,   132,   380,    -1,   386,     9,   380,    -1,   380,   132,
     380,    -1,   380,    -1,    75,    -1,    76,    -1,    77,    -1,
     147,    86,   148,    -1,   147,   148,    -1,    75,    -1,    76,
      -1,    77,    -1,   203,    -1,    87,    -1,    87,    49,   389,
      -1,   387,    -1,   389,    -1,   203,    -1,    47,   388,    -1,
      48,   388,    -1,   134,   186,   391,   187,    -1,    67,   391,
     193,    -1,   176,   186,   394,   187,    -1,   392,   384,    -1,
      -1,   392,     9,   390,   132,   390,    -1,   392,     9,   390,
      -1,   390,   132,   390,    -1,   390,    -1,   393,     9,   390,
      -1,   390,    -1,   395,   384,    -1,    -1,   395,     9,   348,
     132,   390,    -1,   348,   132,   390,    -1,   393,   384,    -1,
      -1,   186,   396,   187,    -1,    -1,   398,     9,   203,   397,
      -1,   203,   397,    -1,    -1,   400,   398,   384,    -1,    46,
     399,    45,    -1,   401,    -1,    -1,   130,    -1,   131,    -1,
     203,    -1,   156,    -1,   189,   335,   190,    -1,   404,    -1,
     420,    -1,   203,    -1,   189,   335,   190,    -1,   406,    -1,
     420,    -1,    67,   423,   193,    -1,   189,   335,   190,    -1,
     414,   408,    -1,   186,   325,   187,   408,    -1,   426,   408,
      -1,   186,   325,   187,   408,    -1,   186,   325,   187,   403,
     405,    -1,   186,   336,   187,   403,   405,    -1,   186,   325,
     187,   403,   404,    -1,   186,   336,   187,   403,   404,    -1,
     420,    -1,   370,    -1,   418,    -1,   419,    -1,   409,    -1,
     411,    -1,   413,   403,   405,    -1,   374,   151,   420,    -1,
     415,   186,   280,   187,    -1,   416,   186,   280,   187,    -1,
     186,   413,   187,    -1,   370,    -1,   418,    -1,   419,    -1,
     409,    -1,   413,   403,   404,    -1,   412,    -1,   415,   186,
     280,   187,    -1,   186,   413,   187,    -1,   420,    -1,   409,
      -1,   370,    -1,   354,    -1,   379,    -1,   186,   413,   187,
      -1,   186,   336,   187,    -1,   416,   186,   280,   187,    -1,
     415,   186,   280,   187,    -1,   186,   417,   187,    -1,   338,
      -1,   341,    -1,   413,   403,   407,   446,   186,   280,   187,
      -1,   186,   325,   187,   403,   407,   446,   186,   280,   187,
      -1,   374,   151,   205,   446,   186,   280,   187,    -1,   374,
     151,   420,   186,   280,   187,    -1,   374,   151,   189,   335,
     190,   186,   280,   187,    -1,   421,    -1,   424,   421,    -1,
     421,    67,   423,   193,    -1,   421,   189,   335,   190,    -1,
     422,    -1,    80,    -1,   191,   189,   335,   190,    -1,   335,
      -1,    -1,   191,    -1,   424,   191,    -1,   420,    -1,   410,
      -1,   411,    -1,   425,   403,   405,    -1,   373,   151,   420,
      -1,   186,   413,   187,    -1,    -1,   410,    -1,   412,    -1,
     425,   403,   404,    -1,   186,   413,   187,    -1,   427,     9,
      -1,   427,     9,   413,    -1,   427,     9,   133,   186,   427,
     187,    -1,    -1,   413,    -1,   133,   186,   427,   187,    -1,
     429,   384,    -1,    -1,   429,     9,   335,   132,   335,    -1,
     429,     9,   335,    -1,   335,   132,   335,    -1,   335,    -1,
     429,     9,   335,   132,    35,   413,    -1,   429,     9,    35,
     413,    -1,   335,   132,    35,   413,    -1,    35,   413,    -1,
     431,   384,    -1,    -1,   431,     9,   335,   132,   335,    -1,
     431,     9,   335,    -1,   335,   132,   335,    -1,   335,    -1,
     433,   384,    -1,    -1,   433,     9,   380,   132,   380,    -1,
     433,     9,   380,    -1,   380,   132,   380,    -1,   380,    -1,
     434,   435,    -1,   434,    86,    -1,   435,    -1,    86,   435,
      -1,    80,    -1,    80,    67,   436,   193,    -1,    80,   403,
     203,    -1,   149,   335,   190,    -1,   149,    79,    67,   335,
     193,   190,    -1,   150,   413,   190,    -1,   203,    -1,    81,
      -1,    80,    -1,   123,   186,   327,   187,    -1,   124,   186,
     413,   187,    -1,   124,   186,   336,   187,    -1,   124,   186,
     417,   187,    -1,   124,   186,   416,   187,    -1,   124,   186,
     325,   187,    -1,     7,   335,    -1,     6,   335,    -1,     5,
     186,   335,   187,    -1,     4,   335,    -1,     3,   335,    -1,
     413,    -1,   438,     9,   413,    -1,   374,   151,   204,    -1,
     374,   151,   126,    -1,    -1,    98,   460,    -1,   177,   445,
      14,   460,   188,    -1,   401,   177,   445,    14,   460,   188,
      -1,   179,   445,   440,    14,   460,   188,    -1,   401,   179,
     445,   440,    14,   460,   188,    -1,   205,    -1,   460,   205,
      -1,   204,    -1,   460,   204,    -1,   205,    -1,   205,   172,
     452,   173,    -1,   203,    -1,   203,   172,   452,   173,    -1,
     172,   448,   173,    -1,    -1,   460,    -1,   447,     9,   460,
      -1,   447,   384,    -1,   447,     9,   165,    -1,   448,    -1,
     165,    -1,    -1,    -1,    30,   460,    -1,    98,   460,    -1,
      99,   460,    -1,   452,     9,   453,   203,    -1,   453,   203,
      -1,   452,     9,   453,   203,   451,    -1,   453,   203,   451,
      -1,    47,    -1,    48,    -1,    -1,    87,   132,   460,    -1,
      29,    87,   132,   460,    -1,   215,   151,   203,   132,   460,
      -1,   455,     9,   454,    -1,   454,    -1,   455,   384,    -1,
      -1,   176,   186,   456,   187,    -1,   215,    -1,   203,   151,
     459,    -1,   203,   446,    -1,    29,   460,    -1,    56,   460,
      -1,   215,    -1,   134,    -1,   135,    -1,   457,    -1,   458,
     151,   459,    -1,   134,   172,   460,   173,    -1,   134,   172,
     460,     9,   460,   173,    -1,   156,    -1,   186,   107,   186,
     449,   187,    30,   460,   187,    -1,   186,   460,     9,   447,
     384,   187,    -1,   460,    -1,    -1
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
     884,   889,   890,   894,   895,   897,   901,   908,   915,   919,
     925,   927,   930,   931,   932,   933,   936,   937,   941,   946,
     946,   952,   952,   959,   958,   964,   964,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   987,   985,   994,   992,   999,  1007,  1001,  1011,
    1009,  1013,  1014,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1034,  1034,  1039,  1045,  1049,  1049,  1057,  1058,
    1062,  1063,  1067,  1073,  1071,  1086,  1083,  1099,  1096,  1113,
    1112,  1121,  1119,  1131,  1130,  1149,  1147,  1166,  1165,  1174,
    1172,  1183,  1183,  1190,  1189,  1201,  1199,  1212,  1213,  1217,
    1220,  1223,  1224,  1225,  1228,  1229,  1232,  1234,  1237,  1238,
    1241,  1242,  1245,  1246,  1250,  1251,  1256,  1257,  1260,  1261,
    1262,  1266,  1267,  1271,  1272,  1276,  1277,  1281,  1282,  1287,
    1288,  1293,  1294,  1295,  1296,  1299,  1302,  1304,  1307,  1308,
    1312,  1314,  1317,  1320,  1323,  1324,  1327,  1328,  1332,  1338,
    1344,  1351,  1353,  1358,  1363,  1369,  1373,  1377,  1381,  1386,
    1391,  1396,  1401,  1407,  1416,  1421,  1426,  1432,  1434,  1438,
    1442,  1447,  1451,  1454,  1457,  1461,  1465,  1469,  1473,  1478,
    1486,  1488,  1491,  1492,  1493,  1494,  1496,  1498,  1503,  1504,
    1507,  1508,  1509,  1513,  1514,  1516,  1517,  1521,  1523,  1526,
    1530,  1536,  1538,  1541,  1541,  1545,  1544,  1548,  1550,  1553,
    1556,  1554,  1570,  1566,  1580,  1582,  1584,  1586,  1588,  1590,
    1592,  1596,  1597,  1598,  1601,  1607,  1611,  1617,  1620,  1625,
    1627,  1632,  1637,  1641,  1642,  1646,  1647,  1649,  1651,  1657,
    1658,  1660,  1664,  1665,  1670,  1674,  1675,  1679,  1680,  1684,
    1686,  1692,  1697,  1698,  1700,  1704,  1705,  1706,  1707,  1711,
    1712,  1713,  1714,  1715,  1716,  1718,  1723,  1726,  1727,  1731,
    1732,  1736,  1737,  1740,  1741,  1744,  1745,  1748,  1749,  1753,
    1754,  1755,  1756,  1757,  1758,  1759,  1763,  1764,  1767,  1768,
    1769,  1772,  1774,  1776,  1777,  1780,  1782,  1786,  1788,  1792,
    1796,  1800,  1805,  1806,  1808,  1809,  1810,  1811,  1814,  1818,
    1819,  1823,  1824,  1828,  1829,  1830,  1831,  1835,  1839,  1844,
    1848,  1852,  1857,  1858,  1859,  1860,  1861,  1865,  1867,  1868,
    1869,  1872,  1873,  1874,  1875,  1876,  1877,  1878,  1879,  1880,
    1881,  1882,  1883,  1884,  1885,  1886,  1887,  1888,  1889,  1890,
    1891,  1892,  1893,  1894,  1895,  1896,  1897,  1898,  1899,  1900,
    1901,  1902,  1903,  1904,  1905,  1906,  1907,  1908,  1909,  1910,
    1911,  1912,  1913,  1914,  1916,  1917,  1919,  1920,  1922,  1923,
    1924,  1925,  1926,  1927,  1928,  1929,  1930,  1931,  1932,  1933,
    1934,  1935,  1936,  1937,  1938,  1939,  1940,  1944,  1948,  1953,
    1952,  1967,  1965,  1983,  1982,  2001,  2000,  2019,  2018,  2036,
    2036,  2051,  2051,  2069,  2070,  2071,  2076,  2078,  2082,  2086,
    2092,  2096,  2102,  2104,  2108,  2110,  2114,  2118,  2119,  2123,
    2130,  2137,  2139,  2144,  2145,  2146,  2147,  2149,  2153,  2154,
    2155,  2156,  2160,  2166,  2175,  2188,  2189,  2192,  2195,  2198,
    2199,  2202,  2206,  2209,  2212,  2219,  2220,  2224,  2225,  2227,
    2231,  2232,  2233,  2234,  2235,  2236,  2237,  2238,  2239,  2240,
    2241,  2242,  2243,  2244,  2245,  2246,  2247,  2248,  2249,  2250,
    2251,  2252,  2253,  2254,  2255,  2256,  2257,  2258,  2259,  2260,
    2261,  2262,  2263,  2264,  2265,  2266,  2267,  2268,  2269,  2270,
    2271,  2272,  2273,  2274,  2275,  2276,  2277,  2278,  2279,  2280,
    2281,  2282,  2283,  2284,  2285,  2286,  2287,  2288,  2289,  2290,
    2291,  2292,  2293,  2294,  2295,  2296,  2297,  2298,  2299,  2300,
    2301,  2302,  2303,  2304,  2305,  2306,  2307,  2308,  2309,  2310,
    2314,  2319,  2320,  2324,  2325,  2326,  2327,  2329,  2333,  2334,
    2345,  2346,  2348,  2360,  2361,  2362,  2366,  2367,  2368,  2372,
    2373,  2374,  2377,  2379,  2383,  2384,  2385,  2386,  2388,  2389,
    2390,  2391,  2392,  2393,  2394,  2395,  2396,  2397,  2400,  2405,
    2406,  2407,  2409,  2410,  2412,  2413,  2414,  2415,  2417,  2419,
    2421,  2423,  2425,  2426,  2427,  2428,  2429,  2430,  2431,  2432,
    2433,  2434,  2435,  2436,  2437,  2438,  2439,  2440,  2441,  2443,
    2445,  2447,  2449,  2450,  2453,  2454,  2458,  2462,  2464,  2468,
    2471,  2474,  2480,  2481,  2482,  2483,  2484,  2485,  2486,  2491,
    2493,  2497,  2498,  2501,  2502,  2506,  2509,  2511,  2513,  2517,
    2518,  2519,  2520,  2523,  2527,  2528,  2529,  2530,  2534,  2536,
    2543,  2544,  2545,  2546,  2547,  2548,  2550,  2551,  2556,  2558,
    2561,  2564,  2566,  2568,  2571,  2573,  2577,  2579,  2582,  2585,
    2591,  2593,  2596,  2597,  2602,  2605,  2609,  2609,  2614,  2617,
    2618,  2622,  2623,  2627,  2628,  2629,  2633,  2635,  2643,  2644,
    2648,  2650,  2658,  2659,  2663,  2664,  2669,  2671,  2676,  2687,
    2701,  2713,  2728,  2729,  2730,  2731,  2732,  2733,  2734,  2744,
    2753,  2755,  2757,  2761,  2762,  2763,  2764,  2765,  2781,  2782,
    2784,  2793,  2794,  2795,  2796,  2797,  2798,  2799,  2800,  2802,
    2807,  2811,  2812,  2816,  2819,  2826,  2830,  2839,  2846,  2848,
    2854,  2856,  2857,  2861,  2862,  2869,  2870,  2875,  2876,  2881,
    2882,  2883,  2884,  2895,  2898,  2901,  2902,  2903,  2904,  2915,
    2919,  2920,  2921,  2923,  2924,  2925,  2929,  2931,  2934,  2936,
    2937,  2938,  2939,  2942,  2944,  2945,  2949,  2951,  2954,  2956,
    2957,  2958,  2962,  2964,  2967,  2970,  2972,  2974,  2978,  2979,
    2981,  2982,  2988,  2989,  2991,  3001,  3003,  3005,  3008,  3009,
    3010,  3014,  3015,  3016,  3017,  3018,  3019,  3020,  3021,  3022,
    3023,  3024,  3028,  3029,  3033,  3035,  3043,  3045,  3049,  3053,
    3058,  3062,  3070,  3071,  3075,  3076,  3082,  3083,  3092,  3093,
    3101,  3104,  3108,  3111,  3116,  3121,  3123,  3124,  3125,  3129,
    3130,  3134,  3135,  3138,  3141,  3143,  3147,  3153,  3154,  3155,
    3159,  3163,  3173,  3181,  3183,  3187,  3189,  3194,  3200,  3203,
    3208,  3216,  3219,  3222,  3223,  3226,  3229,  3230,  3235,  3238,
    3242,  3246,  3252,  3262,  3263
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
     211,   212,   212,   213,   213,   213,   214,   215,   216,   216,
     217,   217,   218,   218,   218,   218,   219,   219,   219,   220,
     219,   221,   219,   222,   219,   223,   219,   219,   219,   219,
     219,   219,   219,   219,   219,   219,   219,   219,   219,   219,
     219,   219,   224,   219,   225,   219,   219,   226,   219,   227,
     219,   219,   219,   219,   219,   219,   219,   219,   219,   219,
     219,   219,   229,   228,   230,   230,   232,   231,   233,   233,
     234,   234,   235,   237,   236,   238,   236,   239,   236,   241,
     240,   242,   240,   244,   243,   245,   243,   246,   243,   247,
     243,   249,   248,   251,   250,   252,   250,   253,   253,   254,
     255,   256,   256,   256,   256,   256,   257,   257,   258,   258,
     259,   259,   260,   260,   261,   261,   262,   262,   263,   263,
     263,   264,   264,   265,   265,   266,   266,   267,   267,   268,
     268,   269,   269,   269,   269,   270,   270,   270,   271,   271,
     272,   272,   273,   273,   274,   274,   275,   275,   276,   276,
     276,   276,   276,   276,   276,   276,   277,   277,   277,   277,
     277,   277,   277,   277,   278,   278,   278,   278,   278,   278,
     278,   278,   279,   279,   279,   279,   279,   279,   279,   279,
     280,   280,   281,   281,   281,   281,   281,   281,   282,   282,
     283,   283,   283,   284,   284,   284,   284,   285,   285,   286,
     287,   288,   288,   290,   289,   291,   289,   289,   289,   289,
     292,   289,   293,   289,   289,   289,   289,   289,   289,   289,
     289,   294,   294,   294,   295,   296,   296,   297,   297,   298,
     298,   299,   299,   300,   300,   301,   301,   301,   301,   301,
     301,   301,   302,   302,   303,   304,   304,   305,   305,   306,
     306,   307,   308,   308,   308,   309,   309,   309,   309,   310,
     310,   310,   310,   310,   310,   310,   311,   311,   311,   312,
     312,   313,   313,   314,   314,   315,   315,   316,   316,   317,
     317,   317,   317,   317,   317,   317,   318,   318,   319,   319,
     319,   320,   320,   320,   320,   321,   321,   322,   322,   323,
     323,   324,   325,   325,   325,   325,   325,   325,   326,   327,
     327,   328,   328,   329,   329,   329,   329,   330,   331,   332,
     333,   334,   335,   335,   335,   335,   335,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   337,   337,   339,
     338,   340,   338,   342,   341,   343,   341,   344,   341,   345,
     341,   346,   341,   347,   347,   347,   348,   348,   349,   349,
     350,   350,   351,   351,   352,   352,   353,   354,   354,   355,
     356,   357,   357,   358,   358,   358,   358,   358,   359,   359,
     359,   359,   360,   361,   361,   362,   362,   363,   363,   364,
     364,   365,   366,   366,   367,   367,   367,   368,   368,   368,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     370,   371,   371,   372,   372,   372,   372,   372,   373,   373,
     374,   374,   374,   375,   375,   375,   376,   376,   376,   377,
     377,   377,   378,   378,   379,   379,   379,   379,   379,   379,
     379,   379,   379,   379,   379,   379,   379,   379,   379,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   381,
     381,   381,   382,   382,   382,   382,   382,   382,   382,   383,
     383,   384,   384,   385,   385,   386,   386,   386,   386,   387,
     387,   387,   387,   387,   388,   388,   388,   388,   389,   389,
     390,   390,   390,   390,   390,   390,   390,   390,   391,   391,
     392,   392,   392,   392,   393,   393,   394,   394,   395,   395,
     396,   396,   397,   397,   398,   398,   400,   399,   401,   402,
     402,   403,   403,   404,   404,   404,   405,   405,   406,   406,
     407,   407,   408,   408,   409,   409,   410,   410,   411,   411,
     412,   412,   413,   413,   413,   413,   413,   413,   413,   413,
     413,   413,   413,   414,   414,   414,   414,   414,   414,   414,
     414,   415,   415,   415,   415,   415,   415,   415,   415,   415,
     416,   417,   417,   418,   418,   419,   419,   419,   420,   420,
     421,   421,   421,   422,   422,   423,   423,   424,   424,   425,
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
       4,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     3,     2,     1,     5,     0,     0,
      11,     0,    12,     0,     4,     0,     7,     0,     5,     0,
       3,     0,     6,     2,     2,     4,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     4,
       4,     4,     4,     1,     1,     1,     1,     3,     3,     4,
       1,     2,     4,     2,     6,     0,     1,     4,     0,     2,
       0,     1,     1,     3,     1,     3,     1,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       1,     1,     3,     1,     1,     1,     2,     1,     0,     0,
       1,     1,     3,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       1,     4,     3,     4,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     4,     3,
       3,     3,     1,     1,     1,     1,     3,     3,     3,     2,
       0,     1,     0,     1,     0,     5,     3,     3,     1,     1,
       1,     1,     3,     2,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     2,     2,     4,     3,     4,     2,     0,
       5,     3,     3,     1,     3,     1,     2,     0,     5,     3,
       2,     0,     3,     0,     4,     2,     0,     3,     3,     1,
       0,     1,     1,     1,     1,     3,     1,     1,     1,     3,
       1,     1,     3,     3,     2,     4,     2,     4,     5,     5,
       5,     5,     1,     1,     1,     1,     1,     1,     3,     3,
       4,     4,     3,     1,     1,     1,     1,     3,     1,     4,
       3,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     1,     1,     7,     9,     7,     6,     8,     1,     2,
       4,     4,     1,     1,     4,     1,     0,     1,     2,     1,
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
       0,   423,     0,   786,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   877,     0,
     865,   668,     0,   674,   675,   676,    22,   733,   853,   150,
     151,   677,     0,   131,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   182,     0,     0,     0,     0,     0,     0,
     389,   390,   391,   394,   393,   392,     0,     0,     0,     0,
     211,     0,     0,     0,   681,   683,   684,   678,   679,     0,
       0,     0,   685,   680,     0,   652,    24,    25,    26,    28,
      27,     0,   682,     0,     0,     0,     0,   686,   395,   521,
       0,   149,   121,   857,   669,     0,     0,     4,   111,   113,
     732,     0,   651,     0,     6,   181,     7,     9,     8,    10,
       0,     0,   387,   434,     0,     0,     0,     0,     0,     0,
       0,   432,   841,   842,   503,   502,   417,   506,     0,   416,
     813,   653,   660,     0,   735,   501,   386,   816,   817,   828,
     433,     0,     0,   436,   435,   814,   815,   812,   848,   852,
       0,   491,   734,    11,   394,   393,   392,     0,     0,    28,
       0,   111,   181,     0,   921,   433,   920,     0,   918,   917,
     505,     0,   424,   429,     0,     0,   474,   475,   476,   477,
     500,   498,   497,   496,   495,   494,   493,   492,   853,   677,
     655,     0,     0,   941,   834,   653,     0,   654,   456,     0,
     454,     0,   881,     0,   742,   415,   664,   201,     0,   941,
     414,   663,   658,     0,   673,   654,   860,   861,   867,   859,
     665,     0,     0,   667,   499,     0,     0,     0,     0,   420,
       0,   129,   422,     0,     0,   135,   137,     0,     0,   139,
       0,    69,    68,    63,    62,    54,    55,    46,    66,    77,
       0,    49,     0,    61,    53,    59,    79,    72,    71,    44,
      67,    86,    87,    45,    82,    42,    83,    43,    84,    41,
      88,    76,    80,    85,    73,    74,    48,    75,    78,    40,
      70,    56,    89,    64,    57,    47,    39,    38,    37,    36,
      35,    34,    58,    90,    92,    51,    32,    33,    60,   974,
     975,    52,   980,    31,    50,    81,     0,     0,   111,    91,
     932,   973,     0,   976,     0,     0,   141,     0,     0,   172,
       0,     0,     0,     0,     0,     0,    94,    99,   300,     0,
       0,   299,     0,   215,     0,   212,   305,     0,     0,     0,
       0,     0,   938,   197,   209,   873,   877,     0,   902,     0,
     688,     0,     0,     0,   900,     0,    16,     0,   115,   189,
     203,   210,   558,   533,     0,   926,   513,   515,   517,   790,
     423,   434,     0,     0,   432,   433,   435,     0,     0,   670,
       0,   671,     0,     0,     0,   171,     0,     0,   117,   291,
       0,    21,   180,     0,   208,   193,   207,   392,   395,   181,
     388,   164,   165,   166,   167,   168,   170,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   865,     0,   163,   856,   856,   887,     0,
       0,     0,     0,     0,     0,     0,     0,   385,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   455,   453,   791,   792,     0,   856,     0,   804,   291,
     291,   856,     0,   858,   849,   873,     0,   181,     0,     0,
     143,     0,   788,   783,   742,     0,   434,   432,     0,   885,
       0,   538,   741,   876,   673,   434,   432,   433,   117,     0,
     291,   413,     0,   806,   666,     0,   121,   251,     0,   520,
       0,   146,     0,     0,   421,     0,     0,     0,     0,     0,
     138,   162,   140,   974,   975,   971,   972,     0,   966,     0,
       0,     0,     0,    65,    30,    52,    29,   933,   169,   142,
     121,     0,   159,   161,     0,     0,    96,   103,     0,     0,
      98,   107,   100,     0,    18,     0,     0,   301,     0,   144,
     214,   213,     0,     0,   145,   922,     0,     0,   434,   432,
     433,   436,   435,     0,   959,   221,     0,   874,     0,     0,
     147,     0,     0,   687,   901,   733,     0,     0,   899,   738,
     898,   114,     5,    13,    14,     0,   219,     0,     0,   526,
       0,     0,     0,   742,     0,     0,   661,   656,   527,     0,
       0,     0,     0,   790,   121,     0,   744,   789,   984,   412,
     426,   488,   822,   840,   126,   120,   122,   123,   124,   125,
     386,     0,   504,   736,   737,   112,   742,     0,   942,     0,
       0,     0,   744,   292,     0,   509,   183,   217,     0,   459,
     461,   460,     0,     0,   457,   458,   462,   464,   463,   479,
     478,   481,   480,   482,   484,   486,   485,   483,   473,   472,
     466,   467,   465,   468,   469,   471,   487,   470,   855,     0,
       0,   891,     0,   742,   925,     0,   924,   941,   819,   848,
     199,   191,   205,     0,   926,   195,   181,     0,   427,   430,
     438,   452,   451,   450,   449,   448,   447,   446,   445,   444,
     443,   442,   441,   794,     0,   793,   796,   818,   800,   941,
     797,     0,     0,     0,     0,     0,     0,     0,     0,   919,
     425,   781,   785,   741,   787,     0,   657,     0,   880,     0,
     879,   217,     0,   657,   864,   863,     0,     0,   793,   796,
     862,   797,   418,   253,   255,   121,   524,   523,   419,     0,
     121,   235,   130,   422,     0,     0,     0,     0,     0,   247,
     247,   136,     0,     0,     0,     0,   964,   742,     0,   948,
       0,     0,     0,     0,     0,   740,     0,   652,     0,     0,
     690,   651,   695,     0,   689,   119,   694,   941,   977,     0,
       0,     0,   104,     0,    19,     0,   108,     0,    20,     0,
       0,    93,   101,     0,   298,   306,   303,     0,     0,   911,
     916,   913,   912,   915,   914,    12,   957,   958,     0,     0,
       0,     0,   873,   870,     0,   537,   910,   909,   908,     0,
     904,     0,   905,   907,     0,     5,     0,     0,     0,   552,
     553,   561,   560,     0,   432,     0,   741,   532,   536,     0,
       0,   927,     0,   514,     0,     0,   949,   790,   277,   983,
       0,     0,   805,     0,   854,   741,   944,   940,   293,   294,
     650,   743,   290,     0,   790,     0,     0,   219,   511,   185,
     490,     0,   541,   542,     0,   539,   741,   886,     0,     0,
     291,   221,     0,   219,     0,     0,   217,     0,   865,   439,
       0,     0,   802,   803,   820,   821,   850,   851,     0,     0,
       0,   769,   749,   750,   751,   758,     0,     0,     0,   762,
     760,   761,   775,   742,     0,   783,   884,   883,     0,   219,
       0,   807,   672,     0,   257,     0,     0,   127,     0,     0,
       0,     0,     0,     0,     0,   227,   228,   239,     0,   121,
     237,   156,   247,     0,   247,     0,     0,   978,     0,     0,
       0,   741,   965,   967,   947,   742,   946,     0,   742,   716,
     717,   714,   715,   748,     0,   742,   740,     0,   535,     0,
       0,   893,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     970,   173,     0,   176,   160,     0,    95,   105,     0,    97,
     109,   102,   302,     0,   923,   148,   959,   939,   954,   220,
     222,   312,     0,     0,   871,     0,   903,     0,    17,     0,
     926,   218,   312,     0,     0,   657,   529,     0,   662,   928,
       0,   949,   518,     0,     0,   984,     0,   282,   280,   796,
     808,   941,   796,   809,   943,     0,     0,   295,   118,     0,
     790,   216,     0,   790,     0,   489,   890,   889,     0,   291,
       0,     0,     0,     0,     0,     0,   219,   187,   673,   795,
     291,     0,   754,   755,   756,   757,   763,   764,   773,     0,
     742,     0,   769,     0,   753,   777,   741,   780,   782,   784,
       0,   878,     0,   795,     0,     0,     0,     0,   254,   525,
     132,     0,   422,   227,   229,   873,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   241,     0,     0,   960,     0,
     963,   741,     0,     0,     0,   692,   741,   739,     0,   730,
       0,   742,     0,   696,   731,   729,   897,     0,   742,   699,
     701,   700,     0,     0,   697,   698,   702,   704,   703,   719,
     718,   721,   720,   722,   724,   726,   725,   723,   712,   711,
     706,   707,   705,   708,   709,   710,   713,   969,     0,   121,
     106,   110,   304,     0,     0,     0,   956,     0,   386,   875,
     873,   428,   431,   437,     0,    15,     0,   386,   564,     0,
       0,   566,   559,   562,     0,   557,     0,   930,     0,   950,
     522,     0,   283,     0,     0,   278,     0,   297,   296,   949,
       0,   312,     0,   790,     0,   291,     0,   846,   312,   926,
     312,   929,     0,     0,     0,   440,     0,     0,   766,   741,
     768,   759,     0,   752,     0,     0,   742,   774,   882,   312,
       0,   121,     0,   250,   236,     0,     0,     0,   226,   152,
     240,     0,     0,   243,     0,   248,   249,   121,   242,   979,
     961,     0,   945,     0,   982,   747,   746,   691,     0,   741,
     534,   693,     0,   540,   741,   892,   728,     0,     0,     0,
     953,   951,   952,   223,     0,     0,     0,   393,   384,     0,
       0,     0,   198,   311,   313,     0,   383,     0,     0,     0,
     926,   386,     0,   906,   308,   204,   555,     0,     0,   528,
     516,     0,   286,   276,     0,   279,   285,   291,   508,   949,
     386,   949,     0,   888,     0,   845,   386,     0,   386,   931,
     312,   790,   843,   772,   771,   765,     0,   767,   741,   776,
     386,   121,   256,   128,   133,   154,   230,     0,   238,   244,
     121,   246,   962,     0,     0,   531,     0,   896,   895,   727,
     121,   177,   955,     0,     0,     0,   934,     0,     0,     0,
     224,     0,   926,     0,   349,   345,   351,   652,    28,     0,
     339,     0,   344,   348,   361,     0,   359,   364,     0,   363,
       0,   362,     0,   181,   315,     0,   317,     0,   318,   319,
       0,     0,   872,     0,   556,   554,   565,   563,   287,     0,
       0,   274,   284,     0,     0,     0,     0,   194,   508,   949,
     847,   200,   308,   206,   386,     0,     0,   779,     0,   202,
     252,     0,     0,   121,   233,   153,   245,   981,   745,     0,
       0,     0,     0,     0,   411,     0,   935,     0,   329,   333,
     408,   409,   343,     0,     0,     0,   324,   616,   615,   612,
     614,   613,   633,   635,   634,   604,   575,   576,   594,   610,
     609,   571,   581,   582,   584,   583,   603,   587,   585,   586,
     588,   589,   590,   591,   592,   593,   595,   596,   597,   598,
     599,   600,   602,   601,   572,   573,   574,   577,   578,   580,
     618,   619,   628,   627,   626,   625,   624,   623,   611,   630,
     620,   621,   622,   605,   606,   607,   608,   631,   632,   636,
     638,   637,   639,   640,   617,   642,   641,   644,   646,   645,
     579,   649,   647,   648,   643,   629,   570,   356,   567,     0,
     325,   377,   378,   376,   369,     0,   370,   326,   403,     0,
       0,     0,     0,   407,     0,   181,   190,   307,     0,     0,
       0,   275,   289,   844,     0,   121,   379,   121,   184,     0,
       0,     0,   196,   949,   770,     0,   121,   231,   134,   155,
       0,   530,   894,   175,   327,   328,   406,   225,     0,     0,
     742,     0,   352,   340,     0,     0,     0,   358,   360,     0,
       0,   365,   372,   373,   371,     0,     0,   314,   936,     0,
       0,     0,   410,     0,   309,     0,   288,     0,   550,   744,
       0,     0,   121,   186,   192,     0,   778,     0,     0,   157,
     330,   111,     0,   331,   332,     0,     0,   346,   741,   354,
     350,   355,   568,   569,     0,   341,   374,   375,   367,   368,
     366,   404,   401,   959,   320,   316,   405,     0,   310,   551,
     743,     0,   510,   380,     0,   188,     0,   234,     0,   179,
       0,   386,     0,   353,   357,     0,     0,   790,   322,     0,
     548,   507,   512,   232,     0,     0,   158,   337,     0,   385,
     347,   402,   937,     0,   744,   397,   790,   549,     0,   178,
       0,     0,   336,   949,   790,   261,   398,   399,   400,   984,
     396,     0,     0,     0,   335,     0,   397,     0,   949,     0,
     334,   381,   121,   321,   984,     0,   266,   264,     0,   121,
       0,     0,   267,     0,     0,   262,   323,     0,   382,     0,
     270,   260,     0,   263,   269,   174,   271,     0,     0,   258,
     268,     0,   259,   273,   272
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   855,   602,   171,  1406,   697,
     335,   555,   559,   336,   556,   560,   109,   110,   111,   112,
     113,   387,   635,   636,   523,   240,  1471,   529,  1387,  1472,
    1709,   811,   330,   550,  1669,  1034,  1209,  1726,   403,   172,
     637,   895,  1094,  1264,   117,   605,   912,   638,   657,   916,
     585,   911,   220,   504,   639,   606,   913,   405,   353,   370,
     120,   897,   858,   841,  1049,  1409,  1147,   965,  1618,  1475,
     772,   971,   528,   781,   973,  1297,   764,   954,   957,  1136,
    1733,  1734,   625,   626,   651,   652,   340,   341,   347,  1443,
    1597,  1598,  1218,  1333,  1432,  1591,  1717,  1736,  1628,  1673,
    1674,  1675,  1419,  1420,  1421,  1422,  1630,  1631,  1637,  1685,
    1425,  1426,  1430,  1584,  1585,  1586,  1608,  1763,  1334,  1335,
     173,   122,  1749,  1750,  1589,  1337,  1338,  1339,  1340,   123,
     233,   524,   525,   124,   125,   126,   127,   128,   129,   130,
     131,  1455,   132,   894,  1093,   133,   622,   623,   624,   237,
     379,   519,   612,   613,  1171,   614,  1172,   134,   135,   136,
     802,   137,   138,  1659,   139,   607,  1445,   608,  1063,   863,
    1235,  1232,  1577,  1578,   140,   141,   142,   223,   143,   224,
     234,   390,   511,   144,   993,   806,   145,   994,   886,   878,
     995,   940,  1116,   941,  1118,  1119,  1120,   943,  1275,  1276,
     944,   742,   494,   184,   185,   640,   628,   475,  1079,  1080,
     728,   729,   882,   147,   226,   148,   149,   175,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   689,   160,   230,
     231,   588,   213,   214,   692,   693,  1177,  1178,   363,   364,
     849,   161,   576,   162,   621,   163,   322,  1599,  1649,   354,
     398,   646,   647,   987,  1074,  1216,   838,   839,   786,   787,
     788,   323,   324,   808,  1408,   880
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1455
static const yytype_int16 yypact[] =
{
   -1455,   177, -1455, -1455,  5104, 12824, 12824,     6, 12824, 12824,
   12824, 10701, 12824, -1455, 12824, 12824, 12824, 12824, 12824, 12824,
   12824, 12824, 12824, 12824, 12824, 12824, 15431, 15431, 10894, 12824,
    3945,    16,    23, -1455, -1455, -1455, -1455, -1455,   158, -1455,
   -1455,   150, 12824, -1455,    23,   190,   212,   218, -1455,    23,
   11087,   675, 11280, -1455, 13726,  9736,   172, 12824,  1093,   164,
   -1455, -1455, -1455,   262,    63,    72,   221,   224,   260,   330,
   -1455,   675,   338,   343, -1455, -1455, -1455, -1455, -1455, 12824,
     498,   825, -1455, -1455,   675, -1455, -1455, -1455, -1455,   675,
   -1455,   675, -1455,   226,   346,   675,   675, -1455,   307, -1455,
   11473, -1455, -1455,   362,   555,   615,   615, -1455,   410,   426,
     207,   383, -1455,    89, -1455,   552, -1455, -1455, -1455, -1455,
     475,   499, -1455, -1455,   401,   409,   433,   476,   480,   488,
    3120, -1455, -1455, -1455, -1455,    83, -1455,   584,   612, -1455,
     143,   495, -1455,   536,    47, -1455,  1998,   153, -1455, -1455,
    2652,   141,   512,   149, -1455,   148,    61,   517,   211, -1455,
     193, -1455,   640, -1455, -1455, -1455,   563,   532,   568, -1455,
   12824, -1455,   552,   499, 16085,  2886, 16085, 12824, 16085, 16085,
   14383,   540, 14821, 14383,   704,   675,   690,   690,   314,   690,
     690,   690,   690,   690,   690,   690,   690,   690, -1455, -1455,
   -1455,    52, 12824,   579, -1455, -1455,   611,   580,   414,   590,
     414, 15431, 15031,   589,   775, -1455,   563, -1455, 12824,   579,
   -1455,   635, -1455,   637,   605, -1455,   159, -1455, -1455, -1455,
     414,   141, 11666, -1455, -1455, 12824,  8385,   792,    91, 16085,
    9350, -1455, 12824, 12824,   675, -1455, -1455,  4825,   618, -1455,
   10686, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
     964, -1455,   964, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,    76,
      78,   568, -1455, -1455, -1455, -1455,   623,   554,    82, -1455,
   -1455,   664,   805, -1455,   674, 14386, -1455,   644, 11072, -1455,
      49, 11651,  1185,  1301,   675,    92, -1455,   251, -1455, 15055,
      93, -1455,   702, -1455,   714, -1455,   831,    94, 15431, 12824,
   12824,   665,   681, -1455, -1455, 15148, 10894,    97,   473,    57,
   -1455, 13017, 15431,   543, -1455,   675, -1455,   370,   426, -1455,
   -1455, -1455, -1455, 14063,   841,   758, -1455, -1455, -1455,   129,
   12824,   676,   680, 16085,   682,  1158,   688,  5297, 12824,   319,
     684,   622,   319,   450,   463, -1455,   675,   964,   686,  9929,
   13726, -1455, -1455,  1199, -1455, -1455, -1455, -1455, -1455,   552,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, 12824, 12824, 12824,
   11859, 12824, 12824, 12824, 12824, 12824, 12824, 12824, 12824, 12824,
   12824, 12824, 12824, 12824, 12824, 12824, 12824, 12824, 12824, 12824,
   12824, 12824, 12824, 15879, 12824, -1455, 12824, 12824, 12824, 13179,
     675,   675,   675,   675,   675,   475,   761,   921,  9543, 12824,
   12824, 12824, 12824, 12824, 12824, 12824, 12824, 12824, 12824, 12824,
   12824, -1455, -1455, -1455, -1455,   703, 12824, 12824, -1455,  9929,
    9929, 12824, 12824,   362,   161, 15148,   691,   552, 12052, 12809,
   -1455, 12824, -1455,   692,   873,   735,   708,   710, 13312,   414,
   12245, -1455, 12438, -1455,   605,   713,   715,  1981, -1455,   217,
    9929, -1455,  1372, -1455, -1455, 14329, -1455, -1455, 10122, -1455,
   12824, -1455,   800,  8578,   882,   705, 16040,   890,   114,   125,
   -1455, -1455, -1455,   738, -1455, -1455, -1455,   964,   668,   725,
     903, 14962,   675, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455,   727, -1455, -1455,   675,   100, -1455,   254,   675,   102,
   -1455,   349,   353,  1494, -1455,   675, 12824,   414,   164, -1455,
   -1455, -1455, 14962,   835, -1455,   414,   115,   116,   730,   732,
    2294,   173,   739,   740,   678,   808,   751,   414,   118,   760,
   -1455,   807,   675, -1455, -1455,   883,  2779,    31, -1455, -1455,
   -1455,   426, -1455, -1455, -1455,   919,   822,   781,   323,   810,
   12824,   362,   826,   954,   780,   821, -1455,   161, -1455,   964,
     964,   961,   792,   129, -1455,   795,   976, -1455,   964,    40,
   -1455,   442,   157, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
    2180,  2831, -1455, -1455, -1455, -1455,   980,   817, -1455, 15431,
   12824,   812,   985, 16085,   983, -1455, -1455,   875,  1232, 11265,
   16256, 14383, 12824, 13002, 16440, 13180,  9909, 10873,  4533, 12223,
   12223, 12223, 12223,  2640,  2640,  2640,  2640,  2640,  1428,  1428,
     967,   967,   967,   314,   314,   314, -1455,   690, 16085,   813,
     818, 15592,   815,   998,    10, 12824,   355,   579,   361,   161,
   -1455, -1455, -1455,   996,   758, -1455,   552, 15245, -1455, -1455,
   14383, 14383, 14383, 14383, 14383, 14383, 14383, 14383, 14383, 14383,
   14383, 14383, 14383, -1455, 12824,   390,   174, -1455, -1455,   579,
     396,   820,  2943,   828,   829,   833,  3035,   120,   836, -1455,
   16085,  3140, -1455,   675, -1455,    40,    33, 15431, 16085, 15431,
   15637,   875,    40,   414,   175,   877,   870, 12824, -1455,   176,
   -1455, -1455, -1455,  8192,   666, -1455, -1455, 16085, 16085,    23,
   -1455, -1455, -1455, 12824,   960, 14845, 14962,   675,  8771,   876,
     878, -1455,    80,   974,   935,   922, -1455,  1061,   891,  3190,
     964, 14962, 14962, 14962, 14962, 14962,   894,   932,   901, 14962,
     366,   937, -1455,   904, -1455, 16173, -1455,    18, -1455,  5490,
    2094,   906,   395,  1185, -1455,   675,   402,  1301, -1455,   675,
     675, -1455, -1455,  3424, -1455, 16173,  1076, 15431,   908, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,    84,   675,
    2094,   911, 15148, 15338,  1078, -1455, -1455, -1455, -1455,   915,
   -1455, 12824, -1455, -1455,  4627, -1455,   964,  2094,   920, -1455,
   -1455, -1455, -1455,  1090,   923, 12824, 14063, -1455, -1455, 13179,
     926, -1455,   964, -1455,   933,  5683,  1088,   134, -1455, -1455,
      87,   703, -1455,  1372, -1455,   964, -1455, -1455,   414, 16085,
   -1455, 10315, -1455, 14962,    64,   942,  2094,   822, -1455, -1455,
   16369, 12824, -1455, -1455, 12824, -1455, 12824, -1455,  3469,   953,
    9929,   808,  1112,   822,   964,  1127,   875,   675, 15879,   414,
    3558,   957, -1455, -1455,   168,   962, -1455, -1455,  1132,  1808,
    1808,  3140, -1455, -1455, -1455,  1103,   968,    55,   969, -1455,
   -1455, -1455, -1455,  1147,   971,   692,   414,   414, 12631,   822,
    1372, -1455, -1455,  3807,   706,    23,  9350, -1455,  5876,   978,
    6069,   979, 14845, 15431,   982,  1032,   414, 16173,  1155, -1455,
   -1455, -1455, -1455,   524, -1455,    32,   964, -1455,  1053,   964,
     675,   668, -1455, -1455, -1455,  1177, -1455,  1000,   980,   604,
     604,  1122,  1122, 15739,   997,  1184, 14962, 14652, 14063,  3605,
   14519, 14962, 14962, 14962, 14962, 14752, 14962, 14962, 14962, 14962,
   14962, 14962, 14962, 14962, 14962, 14962, 14962, 14962, 14962, 14962,
   14962, 14962, 14962, 14962, 14962, 14962, 14962, 14962, 14962,   675,
   -1455, -1455,  1115, -1455, -1455,   675, -1455, -1455,   675, -1455,
   -1455, -1455, -1455, 14962,   414, -1455,   678, -1455,   699,  1187,
   -1455, -1455,   122,  1011,   414, 10508, -1455,  2518, -1455,  4910,
     758,  1187, -1455,   482,    36, -1455, 16085,  1066,  1016, -1455,
    1017,  1088, -1455,   964,   792,   964,    66,  1192,  1129,   222,
   -1455,   579,   256, -1455, -1455, 15431, 12824, 16085, 16173,  1020,
      64, -1455,  1026,    64,  1033, 16369, 16085, 15694,  1034,  9929,
    1031,  1035,   964,  1037,  1039,   964,   822, -1455,   605,   508,
    9929, 12824, -1455, -1455, -1455, -1455, -1455, -1455,  1089,  1040,
    1219,  1143,  3140,  1086, -1455, 14063,  3140, -1455, -1455, -1455,
   15431, 16085,  1047, -1455,    23,  1207,  1167,  9350, -1455, -1455,
   -1455,  1055, 12824,  1032,   414, 15148, 14845,  1057, 14962,  6262,
     619,  1058, 12824,    67,    42, -1455,  1072,   964, -1455,  1121,
   -1455,  3602,  1227,  1071, 14962, -1455, 14962, -1455,  1074, -1455,
    1133,  1250,  1077, -1455, -1455, -1455, 15796,  1081,  1258, 16215,
   16297, 16333, 14962, 16130,  4860, 15312, 10295, 12031, 12416, 12609,
   12609, 12609, 12609,  3076,  3076,  3076,  3076,  3076,  1083,  1083,
     604,   604,   604,  1122,  1122,  1122,  1122, -1455,  1091, -1455,
   -1455, -1455, 16173,   675,   964,   964, -1455,  2094,  1256, -1455,
   15148, -1455, -1455, 14383,  1095, -1455,  1079,  1461, -1455,   335,
   12824, -1455, -1455, -1455, 12824, -1455, 12824, -1455,   792, -1455,
   -1455,   119,  1261,  1196, 12824, -1455,  1094,   414, 16085,  1088,
    1096, -1455,  1099,    64, 12824,  9929,  1104, -1455, -1455,   758,
   -1455, -1455,  1102,  1092,  1106, -1455,  1107,  3140, -1455,  3140,
   -1455, -1455,  1108, -1455,  1168,  1114,  1290, -1455,   414, -1455,
    1273, -1455,  1117, -1455, -1455,  1120,  1125,   123, -1455, -1455,
   16173,  1130,  1136, -1455,  4232, -1455, -1455, -1455, -1455, -1455,
   -1455,   964, -1455,   964, -1455, 16173, 15841, -1455, 14962, 14063,
   -1455, -1455, 14962, -1455, 14962, -1455, 16405, 14962,  1124,  6455,
     699, -1455, -1455, -1455,   685, 13864,  2094,  1201, -1455,   913,
    1161,  1111, -1455, -1455, -1455,   761,  4256,   103,   105,  1137,
     758,   921,   124, -1455, -1455, -1455,  1170,  3933,  4138, 16085,
   -1455,    69,  1313,  1249, 12824, -1455, 16085,  9929,  1217,  1088,
    2015,  1088,  1145, 16085,  1146, -1455,  2163,  1150,  2324, -1455,
   -1455,    64, -1455, -1455,  1203, -1455,  3140, -1455, 14063, -1455,
    2351, -1455,  8192, -1455, -1455, -1455, -1455,  8964, -1455, -1455,
   -1455,  8192, -1455,  1149, 14962, 16173,  1218, 16173, 15898, 16405,
   -1455, -1455, -1455,  2094,  2094,   675, -1455,  1337, 14652,    70,
   -1455, 13864,   758,  2226, -1455,  1181, -1455,   106,  1166,   107,
   -1455, 14169, -1455, -1455, -1455,   108, -1455, -1455,   893, -1455,
    1175, -1455,  1276,   552, -1455, 14002, -1455, 14002, -1455, -1455,
    1351,   761, -1455, 13450, -1455, -1455, -1455, -1455,  1352,  1287,
   12824, -1455, 16085,  1183,  1194,  1189,   628, -1455,  1217,  1088,
   -1455, -1455, -1455, -1455,  2541,  1195,  3140, -1455,  1251, -1455,
    8192,  9157,  8964, -1455, -1455, -1455,  8192, -1455, 16173, 14962,
   14962,  6648,  1204,  1208, -1455, 14962, -1455,  2094, -1455, -1455,
   -1455, -1455, -1455,   964,  1584,   913, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455, -1455, -1455,   550, -1455,  1161,
   -1455, -1455, -1455, -1455, -1455,    75,   144, -1455,  1370,   110,
   14386,  1276,  1373, -1455,   964,   552, -1455, -1455,  1210,  1375,
   12824, -1455, 16085, -1455,   368, -1455, -1455, -1455, -1455,  1214,
     628, 13588, -1455,  1088, -1455,  3140, -1455, -1455, -1455, -1455,
    6841, 16173, 16173, -1455, -1455, -1455, 16173, -1455,   649,   131,
    1390,  1215, -1455, -1455, 14962, 14169, 14169,  1350, -1455,   893,
     893,   636, -1455, -1455, -1455, 14962,  1327, -1455,  1239,  1230,
     111, 14962, -1455,   675, -1455, 14962, 16085,  1342, -1455,  1414,
    7034,  7227, -1455, -1455, -1455,   628, -1455,  7420,  1237,  1315,
   -1455,  1329,  1277, -1455, -1455,  1331,   964, -1455,  1584, -1455,
   -1455, 16173, -1455, -1455,  1267, -1455,  1399, -1455, -1455, -1455,
   -1455, 16173,  1422,   678, -1455, -1455, 16173,  1255, 16173, -1455,
     457,  1264, -1455, -1455,  7613, -1455,  1260, -1455,  1257,  1268,
     675,   921,  1280, -1455, -1455, 14962,   139,    96, -1455,  1374,
   -1455, -1455, -1455, -1455,  2094,   906, -1455,  1289,   675,   814,
   -1455, 16173, -1455,  1271,  1457,   627,    96, -1455,  1388, -1455,
    2094,  1282, -1455,  1088,   130, -1455, -1455, -1455, -1455,   964,
   -1455,  1286,  1297,   112, -1455,   633,   627,   166,  1088,  1298,
   -1455, -1455, -1455, -1455,   964,    71,  1474,  1409,   633, -1455,
    7806,   169,  1477,  1412, 12824, -1455, -1455,  7999, -1455,   240,
    1479,  1418, 12824, -1455, 16085, -1455,  1485,  1420, 12824, -1455,
   16085, 12824, -1455, 16085, 16085
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1455, -1455, -1455,  -530, -1455, -1455, -1455,   282,     1,   -33,
   -1455, -1455, -1455,   939,   693,   694,   187,  1620, -1455,  2913,
   -1455,  -446, -1455,    25, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455, -1455, -1455,  -222, -1455, -1455,  -146,   155,
      26, -1455, -1455, -1455, -1455, -1455, -1455,    27, -1455, -1455,
   -1455, -1455, -1455, -1455,    30, -1455, -1455,  1050,  1059,  1056,
     -75,  -608,  -819,   599,   655,  -227,   371,  -879, -1455,    44,
   -1455, -1455, -1455, -1455,  -700,   229, -1455, -1455, -1455, -1455,
    -218, -1455,  -579, -1455,  -437, -1455, -1455,   952, -1455,    62,
   -1455, -1455, -1004, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455,    34, -1455,   113, -1455, -1455, -1455, -1455,
   -1455,   -54, -1455,   201,  -796, -1455, -1454,  -241, -1455,  -138,
     484,  -119,  -220, -1455,   -53, -1455, -1455, -1455,   213,   -34,
       4,    20,  -698,   -73, -1455, -1455,   -20, -1455, -1455,    -5,
     -44,    81, -1455, -1455, -1455, -1455, -1455, -1455, -1455, -1455,
   -1455,  -571,  -803, -1455, -1455, -1455, -1455, -1455,  1459, -1455,
   -1455, -1455, -1455, -1455,   478, -1455, -1455, -1455, -1455, -1455,
   -1455, -1455, -1455,  -786, -1455,  2401,    35, -1455,  1394,  -383,
   -1455, -1455,  -458,  3461,  1063, -1455, -1455,   546,  -147,  -627,
   -1455, -1455,   614,   424,  -677,   425, -1455, -1455, -1455, -1455,
   -1455,   603, -1455, -1455, -1455,   101,  -828,  -144,  -399,  -394,
   -1455,   669,   -92, -1455, -1455,    38,    39,   559, -1455, -1455,
    1085,   -23, -1455,  -335,    58,    79, -1455,   196, -1455, -1455,
   -1455,  -445,  1197, -1455, -1455, -1455, -1455, -1455,   654,  -206,
   -1455, -1455, -1455,  -332,  -665, -1455,  1152,  -815, -1455,   -63,
    -158,    68,   765, -1455,  -989,   235,  -137,   513,   577, -1455,
   -1455, -1455, -1455,   531,   525, -1053
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -969
static const yytype_int16 yytable[] =
{
     174,   176,   410,   178,   179,   180,   182,   183,   456,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   320,  1241,   212,   215,   892,   486,   382,   371,   114,
     116,   118,   374,   375,   119,   327,   236,   239,   616,   915,
     737,   618,   733,   734,   874,   247,   751,   250,   241,  1075,
     328,   873,   331,   245,   410,   319,   384,   406,  1227,   478,
     686,   508,   238,  1067,   942,   222,   381,   503,   227,   228,
     763,   455,   854,   756,   239,   961,   726,   386,  1092,  1487,
     975,   727,  1238,  1143,  1639,   -65,   512,   -30,   229,   976,
     -65,   -29,   -30,  1046,  1103,   383,   -29,  1295,   400,   357,
     520,   563,   568,   573,   809,   146,   520,   476,  1640,   813,
      13,   817,  1435,   759,  1437,  -342,  1495,  1579,   760,  1646,
    1646,  1487,  1076,   777,   827,   520,   384,   843,  -825,   843,
    1132,   843,   843,   843,  1152,  1153,   381,   358,   495,   513,
    1676,  1123,    13,   949,  1152,  1153,  1242,   386,  1046,  1448,
    -543,  1772,   346,   594,  1351,   779,  1663,   600,   497,   115,
     551,   473,   474,   473,   474,   383,  -519,  1077,   496,  1029,
     473,   474,   489,  1642,   506,    13,    13,     3,   875,   386,
      13,   344,   -92,   594,   505,   600,   594,   600,   600,   345,
     397,  1643,   177,  1233,  1644,  1170,   -92,   383,  -654,  1352,
     360,  1765,   232,  1124,  1779,   593,   361,   362,   476,   235,
    -823,  1705,  -661,   383,   592,  -824,  -545,  -546,   552,  -837,
    -826,   853,  1155,  -655,  -830,  1234,  -866,   515,   481,   477,
     515,  1243,  1298,  -835,  1449,  -829,  1773,   239,   526,   484,
    -545,  -827,  -869,  -868,   338,   337,  1766,  1360,   537,  1780,
    -825,  -281,  1078,   977,  1366,  1296,  1368,  1047,  1488,  1489,
    1358,   517,  1641,   658,   -65,   522,   -30,  1288,   367,  -834,
     -29,   368,  1150,   198,  1154,  1380,   409,   401,   481,   521,
     564,   569,   574,  -265,  1353,   590,   108,  1263,   814,  -810,
     818,  1436,   547,  1438,  -342,  1496,  1580,   198,  1647,  1695,
    1760,   778,   828,   829,  1677,   844,   579,   928,  1106,  1219,
    1386,  1442,  1732,  -281,   780,  1089,   578,  -743,  -743,   958,
    1786,  -743,  1274,  -811,   960,  1059,   319,   582,   487,  -833,
     477,  1767,  -823,   248,  1781,   480,   318,  -824,   410,  -832,
     656,   738,  -826,  -836,   239,   383,  -830,   744,  -866,   565,
     482,   212,   815,   352,  -839,   339,   596,  -829,  -941,   480,
     833,   329,  -656,  -827,  -869,  -868,  1464,   320,   443,   577,
    1456,   369,  1458,   352,   860,   182,   242,   352,   352,   397,
     444,   342,   372,   641,   483,   708,  1346,   376,   343,   371,
     703,   704,   406,  -941,   653,  1226,  -941,  1052,   243,   358,
     482,   319,   352,  1657,   244,  1787,   396,   348,   103,   396,
     349,  -810,   659,   660,   661,   663,   664,   665,   666,   667,
     668,   669,   670,   671,   672,   673,   674,   675,   676,   677,
     678,   679,   680,   681,   682,   683,   684,   685,   709,   687,
     395,   688,   688,   691,  1285,  -811,   350,   819,  1658,  1277,
     696,   820,   617,   710,   711,   712,   713,   714,   715,   716,
     717,   718,   719,   720,   721,   722,   867,   493,   361,   362,
    1610,   688,   732,  1100,   653,   653,   688,   736,   222,   861,
     627,   227,   228,   710,  1082,   881,   740,   883,   121,  1083,
     377,   372,  1719,  1035,   862,   748,   378,   750,   766,   319,
    1038,   229,   456,  1240,   396,   653,  1396,   698,   396,  -547,
    1407,  1250,  -662,   767,  1252,   768,   351,  -941,   108,   557,
     561,   562,   108,  1149,   355,   396,   527,   -91,   699,   356,
     358,   616,   373,   730,   618,  1108,   598,  1720,   397,   909,
     591,   -91,   115,   358,   473,   474,   907,   910,   771,   598,
     396,   388,   601,    36,   699,  -941,   698,   396,   603,   604,
     917,   823,  -798,   150,  1634,   455,   864,   755,  -801,   399,
     761,   921,   473,   474,    48,  1468,  -798,   699,   358,   325,
    1635,   396,  -801,   260,   359,   208,   210,   402,   699,   411,
    1373,   699,  1374,  -657,  1367,   899,  1490,   412,  1636,   361,
     362,   881,   883,   473,   474,   383,    53,   546,   950,   883,
     262,   706,   361,   362,    60,    61,    62,   164,   165,   407,
    1592,   413,  1593,   358,  1665,  1151,  1152,  1153,  -837,   598,
     457,   404,    36,    86,    87,   358,    88,   169,    90,  1228,
     982,   389,   508,   690,   643,   889,   360,   361,   362,  1030,
    1265,   446,  1229,    48,  1025,  1026,  1027,   900,   644,   385,
     951,   539,  1256,   616,   414,  1688,   618,  1350,   415,   108,
    1028,  1230,   731,  1266,  1362,  1440,   416,   735,   645,   447,
    -799,   408,   318,  1689,   448,   352,  1690,   449,   533,   534,
     908,   599,   361,   362,  -799,   358,  1757,   783,   479,  1467,
    1287,   392,   358,  -831,   361,   362,   168,  -544,   598,    84,
     312,  1771,    86,    87,  -655,    88,   169,    90,   485,   920,
    1292,  1152,  1153,   365,   627,   836,   837,    36,   490,   385,
     316,   546,   352,   701,   352,   352,   352,   352,   955,   956,
     317,   812,  1746,  1747,  1748,   816,    36,  1491,    48,   492,
     337,   397,   953,    36,  1755,   784,   444,   725,   391,   393,
     394,   385,   498,  1319,   361,   362,  -835,    48,   239,  1768,
     499,   361,   362,   959,    48,  1342,   480,   507,  1134,  1135,
     546,    36,   501,   198,   502,   535,  -653,   536,   509,  1614,
     616,   510,  1465,   618,   758,   150,  1127,  1214,  1215,   150,
     518,   168,    48,   970,    84,   108,   531,    86,    87,   538,
      88,   169,    90,  1403,  1404,  -968,  1606,  1607,  1364,   541,
     168,  1761,  1762,    84,   807,   542,    86,    87,   570,    88,
     169,    90,   548,    86,    87,  1382,    88,   169,    90,  1670,
     571,  1163,   540,  1686,  1687,   572,  1057,   822,  1167,  1682,
    1683,  1391,   583,   584,  1107,   619,   620,   985,   988,   723,
    1066,    86,    87,   629,    88,   169,    90,   630,    53,   631,
     696,   121,  -116,   848,   850,   633,   642,   655,   741,   114,
     116,   118,   743,   593,   119,    36,  1087,   846,   847,  1735,
     769,   520,   724,   773,   103,   745,  1095,   746,   567,  1096,
     752,  1097,   753,    36,   776,   653,    48,   575,  1735,   580,
     537,   789,   790,   810,   587,   826,  1756,   830,   115,   831,
    1453,   597,   648,  1246,    48,   325,   834,  1068,   835,    60,
      61,    62,   164,   165,   407,  1470,   840,   842,  1666,   730,
     352,   761,  1413,  1131,  1476,   617,   150,   845,   699,   856,
     851,   857,   859,   222,  1481,   146,   227,   228,   865,  1137,
     699,  -677,   699,   866,   115,    86,    87,   868,    88,   169,
      90,    36,   869,  1270,   616,   872,   229,   618,   627,   876,
     365,  1138,  1221,    86,    87,   877,    88,   169,    90,   885,
     887,    36,    48,   260,   891,   627,   408,   893,  1169,   890,
     557,  1175,  1742,   896,   561,   905,   902,   906,   761,   115,
     914,   903,    48,   922,   366,   924,   925,   440,   441,   442,
     262,   443,   898,   939,  1310,   945,   926,  1620,  -659,   699,
     115,  1315,  1701,   444,  1414,  1222,    60,    61,    62,   164,
     165,   407,    36,   616,   587,   108,   618,  1415,  1416,  1581,
    1223,    86,    87,  1582,    88,   169,    90,   952,   962,   968,
     108,   978,   782,    48,   972,   168,   974,   979,    84,  1417,
     981,    86,    87,   980,    88,  1418,    90,   617,   983,  1428,
     996,  1248,   150,   997,   114,   116,   118,   998,  1000,   119,
    1043,   108,  1055,  1001,   653,  1033,  1045,  1037,   533,   534,
    1051,  1040,  1041,   408,  1064,   653,  1223,  1745,  1056,  1062,
    1065,   209,   209,   115,  1069,   115,   168,  1071,  1073,    84,
     312,  1048,    86,    87,   457,    88,   169,    90,  1090,  1379,
    1022,  1023,  1024,  1025,  1026,  1027,   108,   239,  1280,  1099,
     316,  1105,  1102,  1110,   870,   871,  1111,  1294,  -838,  1028,
     317,   546,  1121,   879,  1122,  1125,  1126,   108,  1128,  1660,
     146,  1661,  1283,   725,  1146,   758,  1140,  1142,  1145,  1148,
    1667,    36,   488,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,  1157,  1161,  1162,  1028,    36,
    1165,   627,    48,  1166,   627,  1208,  1217,  1220,  1236,   352,
     332,   333,   910,  1441,   617,  1237,  1244,  1249,   888,  1245,
      48,  1115,  1115,   939,   115,  1251,  1704,   410,  1257,  1253,
    1255,  1267,   471,   472,  1258,  1347,  1260,  1261,  1269,  1348,
     935,  1349,   758,  1268,  1273,  1427,  1279,  1281,   108,  1356,
     108,  1282,   108,  1284,  1289,  1299,  1293,   121,   334,  1363,
     653,    86,    87,  1301,    88,   169,    90,  1303,  1304,  1309,
    1324,  1307,  1159,    36,  1311,  1308,   919,  1314,  1344,    86,
      87,  1313,    88,   169,    90,  1354,  1355,    36,  1318,   546,
    1357,  1370,   546,  1359,    48,  1343,  1361,  1590,   473,   474,
    1369,  1365,  1371,   121,  1372,  1375,   209,  1428,    48,  1378,
    1376,  1377,    13,  1381,   115,  1383,   946,  1384,   947,  1411,
      36,   807,  1385,  1400,   648,   648,  1770,  1210,  1388,  1341,
    1211,  1424,   150,  1777,  1389,  1439,  1444,  1450,  1341,  1451,
    1454,    48,  1459,  1460,   966,  1466,  1477,   150,   121,  1462,
     554,   108,  1484,    86,    87,   632,    88,   169,    90,  1452,
    1479,  1485,   653,  1493,   627,  1494,  1588,    86,    87,   121,
      88,   169,    90,  1587,  1325,  1594,  1600,  1601,   150,  1326,
    1603,    60,    61,    62,   164,  1327,   407,  1328,  1605,    36,
    1604,  1060,  1613,  1615,  1645,   655,  1044,  1651,   617,  1655,
      86,    87,  1624,    88,   169,    90,  1625,  1070,  1654,  1678,
      48,   587,  1054,  1662,   939,  1680,  1684,  1692,   939,  1486,
    1084,  1693,  1474,   150,  1329,  1330,  1694,  1331,   898,   108,
     206,   206,  1699,  1700,   209,  1707,  1708,  -338,  1710,  1711,
    1714,   108,  1640,   209,   150,   581,  1715,  1725,   408,  1104,
     209,  1718,   121,  1724,   121,  1602,  1332,   209,  1723,  1653,
      36,  1721,   198,  1730,  1737,  1740,   558,   617,  1743,    86,
      87,  1341,    88,   169,    90,  1324,  1744,  1341,  1752,  1341,
    1754,    48,   627,  1758,   115,   437,   438,   439,   440,   441,
     442,  1341,   443,  1679,  1759,   204,   204,  1769,  1774,  1775,
    1433,  1782,  1783,  1788,   444,  1320,  1617,  1474,  1789,  1791,
    1792,  1156,   821,  1739,  1158,   705,  1036,    13,   702,   700,
    1101,  1039,  1061,  1753,  1286,   150,  1619,   150,  1751,   150,
     824,   966,  1144,  1390,  1611,  1638,  1492,  1776,   723,  1633,
      86,    87,  1431,    88,   169,    90,  1764,   115,  1650,  1609,
    1412,  1231,  1168,   121,  1117,  1271,   115,  1272,  1129,   939,
    1081,   939,   654,   589,   986,  1402,  1716,  1648,  1160,  1213,
    1207,   757,     0,   103,     0,  1341,     0,     0,     0,  1325,
     209,     0,    36,  1728,  1326,     0,    60,    61,    62,   164,
    1327,   407,  1328,     0,     0,     0,     0,     0,     0,     0,
    1697,   319,     0,    48,     0,  1656,  1595,     0,  1239,     0,
     879,   108,     0,     0,   805,   206,     0,   318,     0,     0,
     410,     0,     0,  1429,     0,     0,     0,     0,   150,  1329,
    1330,     0,  1331,     0,     0,   115,     0,  1259,     0,     0,
    1262,   115,     0,   121,     0,   825,   115,     0,     0,     0,
       0,     0,     0,   408,  1247,     0,   203,   203,     0,   334,
     219,  1345,    86,    87,     0,    88,   169,    90,   939,    33,
      34,    35,     0,     0,   108,     0,     0,     0,     0,   108,
     204,   199,     0,   108,   219,     0,     0,     0,     0,     0,
       0,     0,  1300,     0,     0,     0,  1084,   352,     0,  1278,
     546,     0,     0,   318,     0,     0,   150,     0,     0,     0,
       0,     0,  1336,  1576,   587,   966,     0,     0,   150,     0,
    1583,  1336,     0,     0,     0,     0,     0,   318,     0,   318,
      74,    75,    76,    77,    78,   318,     0,     0,     0,     0,
       0,   201,     0,   206,   209,     0,     0,    82,    83,  1321,
    1322,     0,   206,     0,     0,     0,     0,     0,   939,   206,
       0,    92,   108,   108,   108,     0,   206,     0,   108,     0,
       0,     0,     0,   108,     0,    97,     0,   615,     0,  1784,
       0,     0,     0,     0,     0,   115,     0,  1790,     0,   587,
       0,     0,     0,  1793,     0,     0,  1794,     0,     0,     0,
       0,     0,   209,     0,     0,     0,     0,     0,   204,     0,
       0,     0,     0,   121,     0,     0,     0,   204,     0,     0,
       0,     0,     0,     0,   204,   115,   115,     0,   627,     0,
       0,   204,   115,     0,     0,   457,  1392,     0,  1393,     0,
       0,   203,   209,     0,   209,     0,     0,   627,     0,   967,
       0,     0,     0,     0,  1336,   627,     0,     0,     0,     0,
    1336,     0,  1336,     0,   989,   990,   991,   992,     0,   115,
     209,  1434,   999,     0,  1336,     0,   121,     0,     0,     0,
       0,     0,   546,     0,     0,   121,     0,     0,   150,   206,
     219,     0,   219,  1112,  1113,  1114,    36,     0,     0,     0,
       0,     0,     0,   318,     0,     0,     0,   939,     0,     0,
       0,     0,   108,     0,     0,     0,     0,    48,     0,     0,
    1671,     0,   209,     0,     0,     0,     0,  1576,  1576,     0,
       0,  1583,  1583,     0,     0,   115,     0,   209,   209,     0,
       0,     0,   115,     0,     0,   352,     0,   219,     0,     0,
       0,   150,   108,   108,   204,     0,   150,     0,  1336,   108,
     150,     0,     0,     0,   121,     0,  1088,     0,     0,   203,
     121,     0,     0,     0,     0,   121,    86,    87,   203,    88,
     169,    90,     0,     0,     0,   203,     0,     0,     0,     0,
       0,     0,   203,     0,     0,     0,   108,     0,     0,     0,
       0,     0,  1727,   219,     0,   488,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,     0,     0,
    1741,     0,     0,     0,     0,     0,     0,   219,  1629,  1324,
     219,     0,     0,     0,     0,     0,     0,     0,     0,   150,
     150,   150,     0,     0,     0,   150,     0,     0,     0,     0,
     150,     0,     0,   206,     0,   471,   472,   209,   209,     0,
       0,     0,   108,     0,     0,     0,     0,     0,     0,   108,
       0,    13,     0,   219,  1176,  1179,  1180,  1181,  1183,  1184,
    1185,  1186,  1187,  1188,  1189,  1190,  1191,  1192,  1193,  1194,
    1195,  1196,  1197,  1198,  1199,  1200,  1201,  1202,  1203,  1204,
    1205,  1206,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   206,     0,     0,   121,   203,  1212,     0,   204,     0,
       0,   473,   474,    60,    61,    62,    63,    64,   407,  1652,
       0,     0,     0,  1325,    70,   450,     0,     0,  1326,     0,
      60,    61,    62,   164,  1327,   407,  1328,     0,     0,     0,
       0,   206,     0,   206,   121,   121,     0,     0,     0,     0,
       0,   121,     0,     0,     0,     0,     0,   219,   219,     0,
     451,   800,   452,     0,     0,     0,   204,  1324,   754,   206,
     209,     0,    36,  1329,  1330,   453,  1331,   454,     0,   150,
     408,     0,     0,     0,     0,     0,     0,     0,   121,     0,
       0,     0,   800,    48,     0,  1729,     0,   408,     0,     0,
       0,  1712,     0,     0,     0,  1457,   204,     0,   204,    13,
       0,  1290,     0,     0,     0,   209,     0,     0,     0,   150,
     150,   206,     0,     0,     0,     0,   150,  1305,     0,  1306,
     209,   209,     0,     0,   204,     0,   206,   206,     0,   219,
     219,     0,     0,     0,     0,  1316,   168,     0,   219,    84,
      85,     0,    86,    87,   121,    88,   169,    90,     0,     0,
     615,   121,     0,   150,     0,     0,     0,     0,     0,   203,
       0,  1325,     0,     0,   879,     0,  1326,     0,    60,    61,
      62,   164,  1327,   407,  1328,     0,   204,     0,     0,   879,
       0,     0,     0,     0,     0,    60,    61,    62,    63,    64,
     407,   204,   204,     0,    36,   209,    70,   450,   488,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,  1329,  1330,     0,  1331,    48,     0,   203,  1324,   150,
       0,     0,     0,     0,     0,     0,   150,     0,     0,     0,
       0,     0,     0,     0,   452,   408,     0,  1414,     0,     0,
       0,     0,     0,  1461,     0,  1324,   206,   206,   471,   472,
    1415,  1416,   408,     0,     0,     0,     0,   203,     0,   203,
      13,  1395,     0,     0,     0,  1397,     0,  1398,   168,     0,
    1399,    84,    85,     0,    86,    87,     0,    88,  1418,    90,
       0,     0,   615,     0,     0,   203,   800,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   219,
     219,   800,   800,   800,   800,   800,     0,     0,     0,   800,
       0,   204,   204,     0,   473,   474,     0,   205,   205,     0,
     219,   221,  1325,     0,     0,     0,     0,  1326,     0,    60,
      61,    62,   164,  1327,   407,  1328,     0,   203,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1478,     0,  1325,
     219,     0,   203,   203,  1326,     0,    60,    61,    62,   164,
    1327,   407,  1328,     0,     0,     0,   219,   219,     0,   206,
       0,   832,  1329,  1330,     0,  1331,   219,     0,     0,     0,
       0,     0,   219,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   219,   408,     0,     0,  1329,
    1330,     0,  1331,   800,  1463,     0,   219,     0,     0,   615,
       0,     0,     0,     0,   206,     0,     0,     0,   417,   418,
     419,     0,     0,   408,   219,     0,     0,     0,   219,   206,
     206,  1469,  1621,  1622,   204,  1324,     0,   420,  1626,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,     0,   443,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   203,   203,   444,     0,     0,    13,     0,   204,
       0,     0,     0,     0,     0,     0,   219,     0,     0,   219,
       0,   219,     0,     0,   204,   204,     0,     0,     0,     0,
       0,     0,   205,     0,   206,     0,   800,     0,   219,     0,
       0,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,  1325,
       0,     0,     0,     0,  1326,     0,    60,    61,    62,   164,
    1327,   407,  1328,   800,     0,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   204,
    -969,  -969,  -969,  -969,  -969,   435,   436,   437,   438,   439,
     440,   441,   442,   219,   443,   219,     0,  1681,     0,  1329,
    1330,     0,  1331,   615,     0,   203,   444,     0,  1691,     0,
       0,  1224,     0,     0,  1696,     0,   471,   472,  1698,     0,
       0,     0,   219,   408,     0,   219,     0,     0,     0,     0,
       0,  1612,     0,     0,     0,     0,     0,     0,     0,     0,
     205,     0,     0,     0,     0,   219,     0,     0,     0,   205,
     203,     0,     0,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,   205,     0,   203,   203,     0,   800,     0,
       0,     0,   615,     0,   205,     0,     0,   219,  1731,     0,
       0,   219,   473,   474,   800,     0,   800,     0,     0,   417,
     418,   419,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   800,     0,     0,     0,     0,     0,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,     0,   443,   219,   219,     0,   219,     0,     0,
     203,   417,   418,   419,   221,   444,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,     0,   443,   205,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   444,     0,     0,
     488,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,     0,     0,     0,     0,     0,     0,     0,
       0,   219,     0,   219,     0,     0,     0,     0,   800,   219,
       0,     0,   800,     0,   800,     0,     0,   800,     0,     0,
       0,     0,   803,     0,     0,   219,   219,     0,     0,   219,
     471,   472,     0,   417,   418,   419,   219,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   321,     0,   852,
       0,     0,   420,   803,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,     0,   443,   219,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   444,
       0,     0,     0,     0,   800,     0,   473,   474,     0,     0,
       0,   884,     0,   219,   219,     0,     0,     0,     0,     0,
       0,   219,     0,   219,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   417,   418,   419,     0,     0,
     205,     0,     0,     0,     0,   219,     0,   219,     0,     0,
       0,     0,     0,   219,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,     0,   443,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   800,
     800,   444,     0,     0,     0,   800,     0,   219,   205,     0,
       0,     0,     0,   219,     0,   219,  -969,  -969,  -969,  -969,
    -969,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,     0,
     417,   418,   419,   923,     0,     0,     0,     0,     0,     0,
       0,     0,  1028,     0,     0,     0,     0,     0,   205,   420,
     205,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   321,   443,   321,   205,   803,     0,     0,
       0,     0,     0,     0,     0,     0,   444,   929,   930,     0,
       0,     0,   803,   803,   803,   803,   803,     0,     0,     0,
     803,     0,     0,     0,     0,     0,     0,   931,     0,     0,
       0,  1032,     0,     0,   219,   932,   933,   934,    36,   260,
       0,     0,     0,     0,     0,   927,     0,   935,   205,     0,
     321,   219,     0,     0,     0,     0,     0,     0,     0,    48,
       0,  1050,     0,   205,   205,     0,   262,     0,   219,     0,
       0,     0,     0,     0,   800,     0,     0,     0,  1050,     0,
       0,     0,     0,     0,     0,   800,     0,   205,    36,     0,
       0,   800,     0,     0,   936,   800,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   937,     0,    48,
       0,     0,     0,     0,   803,     0,   219,  1091,    86,    87,
       0,    88,   169,    90,     0,     0,     0,     0,   445,     0,
     321,     0,     0,   321,     0,     0,   938,     0,     0,   221,
       0,     0,     0,     0,   533,   534,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   800,     0,     0,     0,     0,
       0,     0,   168,     0,   219,    84,   312,     0,    86,    87,
       0,    88,   169,    90,     0,   984,     0,     0,     0,     0,
     219,     0,     0,   205,   205,     0,   316,     0,     0,   219,
       0,     0,     0,     0,     0,     0,   317,     0,     0,     0,
       0,     0,     0,     0,   219,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   803,     0,   205,
       0,     0,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
       0,     0,     0,     0,   417,   418,   419,     0,     0,     0,
       0,     0,     0,     0,   803,     0,     0,     0,     0,     0,
     321,   785,     0,   420,   801,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,     0,   443,   417,
     418,   419,     0,     0,     0,   801,   205,   207,   207,     0,
     444,   225,     0,     0,     0,     0,     0,     0,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,     0,   443,     0,     0,   205,     0,     0,     0,
       0,   205,   321,   321,     0,   444,     0,     0,     0,     0,
       0,   321,     0,     0,     0,     0,   205,   205,     0,   803,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   803,     0,   803,   417,   418,
     419,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   803,     0,     0,     0,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,     0,   443,     0,  1042,  1002,  1003,  1004,  1323,     0,
       0,   205,     0,     0,   444,     0,     0,     0,     0,     0,
       0,   260,     0,     0,  1005,     0,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,   262,  1098,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1028,   207,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,   801,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,   321,   321,   801,   801,   801,   801,   801,   803,
     205,     0,   801,   803,     0,   803,     0,     0,   803,     0,
       0,     0,     0,     0,     0,     0,     0,  1410,     0,     0,
    1423,     0,     0,     0,     0,     0,   533,   534,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1109,     0,
       0,     0,     0,     0,   168,     0,     0,    84,   312,     0,
      86,    87,     0,    88,   169,    90,     0,  1302,     0,   321,
       0,     0,     0,     0,     0,     0,     0,     0,   316,   205,
       0,     0,     0,     0,     0,   321,     0,     0,   317,     0,
       0,     0,  1173,     0,     0,   803,     0,     0,   321,     0,
     207,     0,     0,     0,  1482,  1483,   801,     0,     0,   207,
       0,     0,     0,     0,  1423,     0,   207,   417,   418,   419,
       0,     0,     0,   207,     0,     0,     0,   321,     0,     0,
       0,     0,     0,     0,   225,     0,   420,     0,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
       0,   443,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   444,     0,     0,     0,     0,     0,     0,
     803,   803,     0,     0,     0,     0,   803,     0,  1627,   321,
       0,     0,   321,     0,   785,     0,  1423,     0,     0,     0,
       0,     0,     0,     0,   225,     0,     0,     0,     0,   801,
       0,     0,     0,     0,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,     0,   417,   418,   419,   207,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   801,     0,     0,     0,
       0,     0,   420,     0,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   321,   443,   321,     0,
       0,     0,     0,     0,     0,     0,     0,  1133,     0,   444,
       0,     0,   804,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   321,     0,     0,   321,     0,
      33,    34,    35,    36,     0,   198,     0,     0,     0,     0,
       0,     0,   199,   804,     0,   803,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,   803,     0,     0,     0,
       0,     0,   803,     0,     0,     0,   803,     0,     0,     0,
       0,   801,     0,     0,     0,   216,     0,     0,     0,     0,
     321,   217,     0,     0,   321,     0,     0,   801,     0,   801,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   201,     0,     0,   801,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,     0,
     207,     0,    92,     0,     0,     0,   803,     0,     0,     0,
       0,     0,     0,  1446,     0,  1738,    97,   321,   321,     0,
       0,   218,     0,     0,     0,     0,   103,     0,     0,     0,
       0,  1410,     0,     0,     0,     0,     0,     0,   417,   418,
     419,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   420,   207,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,     0,   443,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   444,     0,     0,     0,   207,     0,
     207,     0,     0,     0,   321,     0,   321,     0,     0,     0,
       0,   801,     0,     0,     0,   801,     0,   801,     0,     0,
     801,     0,     0,     0,     0,     0,   207,   804,   321,     0,
       0,     0,   417,   418,   419,     0,     0,     0,     0,   321,
       0,     0,   804,   804,   804,   804,   804,     0,     0,     0,
     804,   420,  1295,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   260,   443,     0,   207,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   444,     0,
       0,     0,     0,   207,   207,     0,     0,   801,     0,     0,
       0,     0,   262,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   321,     0,     0,   225,  1447,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   321,     0,
     321,     0,     0,     0,   804,    48,   321,     0,     0,     0,
       0,     0,     0,  -385,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   164,   165,   407,     0,     0,   225,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     533,   534,   801,   801,     0,     0,     0,     0,   801,     0,
       0,     0,     0,     0,     0,     0,   321,     0,   168,     0,
       0,    84,   312,     0,    86,    87,     0,    88,   169,    90,
    1296,     0,     0,   207,   207,     0,     0,     0,     0,     0,
       0,     0,   316,     0,     0,     0,     0,     0,   408,     0,
       0,     0,   317,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   804,     0,   225,
       0,     0,   804,   804,   804,   804,   804,   804,   804,   804,
     804,   804,   804,   804,   804,   804,   804,   804,   804,   804,
     804,   804,   804,   804,   804,   804,   804,   804,   804,   804,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   804,     0,     0,   321,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   321,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1672,     0,     0,     0,     0,   207,   801,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   801,     0,
       0,     0,     0,     0,   801,     0,     0,     0,   801,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   225,   443,     0,   321,
       0,   207,     0,     0,     0,     0,     0,     0,     0,   444,
       0,     0,     0,     0,     0,     0,   207,   207,     0,   804,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   804,     0,   804,   801,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,   804,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,   321,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,   321,     0,     0,
      16,   207,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,   804,
     225,    79,     0,   804,    80,   804,     0,     0,   804,    81,
      82,    83,    84,    85,     0,    86,    87,     0,    88,    89,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,    95,     0,    96,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,  1058,   103,   104,
       0,   105,   106,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   417,   418,   419,     0,   225,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   420,   804,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,     0,   443,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   444,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,  1028,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     804,   804,     0,     0,     0,     0,   804,     0,     0,     0,
       0,     0,     0,     0,     0,  1632,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,   530,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,    81,    82,    83,    84,    85,     0,    86,    87,
       0,    88,    89,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,    95,     0,    96,
       0,    97,    98,    99,     0,   804,   100,     0,   101,   102,
    1225,   103,   104,     0,   105,   106,   804,     5,     6,     7,
       8,     9,   804,     0,     0,     0,   804,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,  1713,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,   804,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,    69,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,    81,    82,    83,    84,
      85,     0,    86,    87,     0,    88,    89,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,    95,     0,    96,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,     0,   103,   104,     0,   105,   106,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,   634,   103,   104,
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
    1031,   103,   104,     0,   105,   106,     5,     6,     7,     8,
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
       0,   101,   102,  1072,   103,   104,     0,   105,   106,     5,
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
       0,     0,   100,     0,   101,   102,  1139,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
    1141,    45,     0,    46,     0,    47,     0,     0,    48,    49,
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
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,  1291,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,     0,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   168,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   169,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,     0,   103,   104,     0,   105,   106,     5,     6,
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
       0,   100,     0,   101,   102,  1401,   103,   104,     0,   105,
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
      98,    99,     0,     0,   100,     0,   101,   102,  1623,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,  1668,    47,     0,     0,
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
     100,     0,   101,   102,  1702,   103,   104,     0,   105,   106,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,  1703,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,  1706,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,     0,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   168,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   169,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
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
       0,   101,   102,  1722,   103,   104,     0,   105,   106,     5,
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
       0,     0,   100,     0,   101,   102,  1778,   103,   104,     0,
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
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1785,
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
     101,   102,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,   516,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,   770,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,   164,   165,    65,     0,
      66,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     168,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     169,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   101,   102,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,   969,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    11,    12,     0,  1473,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    11,    12,     0,  1616,     0,     0,
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
     380,    12,     0,     0,     0,     0,     0,     0,   707,     0,
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
       0,     0,     0,     0,   103,   104,     0,   105,   106,     5,
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
       0,    60,    61,    62,   164,   165,   166,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   167,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   168,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   169,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   170,     0,   326,     0,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,     0,   443,   649,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   444,    14,    15,     0,     0,
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
      88,   169,    90,     0,   650,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   170,     0,     0,     0,     0,
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
       0,   765,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,     0,     0,
    1085,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1028,    14,    15,     0,     0,     0,     0,    16,     0,
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
    1086,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   170,     0,     0,     0,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   380,    12,     0,     0,     0,
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
      98,    99,     0,     0,   100,     0,   417,   418,   419,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   420,     0,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,     0,
     443,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   444,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,   181,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   164,
     165,   166,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   167,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,   532,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   170,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,     0,   443,     0,   211,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   444,
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
     170,     0,   417,   418,   419,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   420,     0,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,     0,   443,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   444,     0,
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
     549,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   170,     0,   246,   418,   419,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,     0,   443,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   444,     0,    16,     0,    17,    18,    19,    20,    21,
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
       0,    97,    98,    99,     0,     0,   170,     0,   249,     0,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     380,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     439,   440,   441,   442,     0,   443,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   444,     0,    16,
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
       0,     0,     0,    92,     0,     0,    93,     0,     0,   553,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   170,   514,     0,     0,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   662,
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
       0,     0,     0,     0,     0,    10,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,     0,     0,     0,   707,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1028,     0,    14,
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
       0,     0,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  -969,
    -969,  -969,  -969,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,     0,   443,     0,     0,
     747,     0,     0,     0,     0,     0,     0,     0,     0,   444,
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
       0,    10,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,     0,
       0,     0,     0,   749,     0,     0,     0,     0,     0,     0,
       0,     0,  1028,     0,     0,    14,    15,     0,     0,     0,
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
       0,     0,     0,     0,    10,  -969,  -969,  -969,  -969,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,     0,     0,     0,     0,  1130,     0,     0,     0,
       0,     0,     0,     0,     0,  1028,     0,     0,    14,    15,
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
     441,   442,     0,   443,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,   444,     0,    16,     0,    17,
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
       0,    92,     0,     0,    93,     0,   739,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     170,     0,   417,   418,   419,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   420,   901,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,     0,   443,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   444,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,   595,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   164,   165,   166,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     167,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,     0,   251,   252,    92,   253,   254,    93,     0,   255,
     256,   257,   258,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   170,     0,     0,   259,     0,   103,   104,
       0,   105,   106,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   261,   443,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   444,   263,   264,   265,
     266,   267,   268,   269,     0,     0,     0,    36,     0,   198,
       0,     0,     0,     0,     0,     0,     0,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,    48,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,     0,     0,     0,   694,   305,   306,   307,     0,
       0,     0,   308,   543,   544,   251,   252,     0,   253,   254,
       0,     0,   255,   256,   257,   258,     0,     0,     0,     0,
       0,   545,     0,     0,     0,     0,     0,    86,    87,   259,
      88,   169,    90,   313,     0,   314,     0,     0,   315,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   261,     0,   695,     0,
     103,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     263,   264,   265,   266,   267,   268,   269,     0,     0,     0,
      36,     0,   198,     0,     0,     0,     0,     0,     0,     0,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,    48,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,     0,     0,     0,   304,   305,
     306,   307,     0,     0,     0,   308,   543,   544,     0,     0,
       0,     0,     0,   251,   252,     0,   253,   254,     0,     0,
     255,   256,   257,   258,   545,     0,     0,     0,     0,     0,
      86,    87,     0,    88,   169,    90,   313,   259,   314,   260,
       0,   315,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   695,     0,   103,   261,     0,   262,     0,     0,     0,
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
    1596,     0,   261,     0,   262,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,   316,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   317,     0,     0,     0,  1664,     0,
     261,     0,   262,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   263,   264,   265,   266,   267,   268,
     269,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,    48,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,     0,
       0,     0,   304,   305,   306,   307,     0,     0,     0,   308,
     309,   310,     0,     0,     0,     0,     0,   251,   252,     0,
     253,   254,     0,     0,   255,   256,   257,   258,   311,     0,
       0,    84,   312,     0,    86,    87,     0,    88,   169,    90,
     313,   259,   314,   260,     0,   315,     0,     0,     0,     0,
       0,     0,   316,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   317,     0,     0,     0,     0,     0,   261,     0,
     262,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   263,   264,   265,   266,   267,   268,   269,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,    48,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,     0,     0,     0,
       0,   305,   306,   307,     0,     0,     0,   308,   309,   310,
       0,     0,     0,     0,     0,   251,   252,     0,   253,   254,
       0,     0,   255,   256,   257,   258,   311,     0,     0,    84,
     312,     0,    86,    87,     0,    88,   169,    90,   313,   259,
     314,   260,     0,   315,     0,     0,     0,     0,     0,     0,
     316,  1405,     0,     0,     0,     0,     0,     0,     0,     0,
     317,     0,     0,     0,     0,     0,   261,     0,   262,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     263,   264,   265,   266,   267,   268,   269,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,    48,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,     0,     0,     0,     0,   305,
     306,   307,     0,     0,     0,   308,   309,   310,    33,    34,
      35,    36,     0,   198,     0,     0,     0,     0,     0,     0,
     609,     0,     0,     0,   311,     0,     0,    84,   312,     0,
      86,    87,    48,    88,   169,    90,   313,     0,   314,     0,
       0,   315,  1497,  1498,  1499,  1500,  1501,     0,   316,  1502,
    1503,  1504,  1505,   200,     0,     0,     0,     0,   317,     0,
       0,     0,     0,     0,     0,     0,  1506,  1507,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     201,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,  1508,    88,   169,    90,     0,     0,     0,
      92,     0,     0,     0,     0,     0,     0,  1509,  1510,  1511,
    1512,  1513,  1514,  1515,    97,     0,     0,    36,     0,   610,
       0,     0,     0,     0,   611,     0,     0,  1516,  1517,  1518,
    1519,  1520,  1521,  1522,  1523,  1524,  1525,  1526,    48,  1527,
    1528,  1529,  1530,  1531,  1532,  1533,  1534,  1535,  1536,  1537,
    1538,  1539,  1540,  1541,  1542,  1543,  1544,  1545,  1546,  1547,
    1548,  1549,  1550,  1551,  1552,  1553,  1554,  1555,  1556,     0,
       0,     0,  1557,  1558,     0,  1559,  1560,  1561,  1562,  1563,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1564,  1565,  1566,     0,     0,     0,    86,    87,     0,
      88,   169,    90,  1567,     0,  1568,  1569,     0,  1570,   417,
     418,   419,     0,     0,     0,  1571,  1572,     0,  1573,     0,
    1574,  1575,     0,     0,     0,     0,     0,     0,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,     0,   443,     0,     0,     0,     0,     0,   251,
     252,     0,   253,   254,     0,   444,   255,   256,   257,   258,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   420,   259,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,     0,   443,     0,     0,
     261,     0,     0,     0,     0,     0,     0,     0,     0,   444,
       0,     0,     0,     0,   263,   264,   265,   266,   267,   268,
     269,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,    48,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,     0,
       0,     0,   304,   305,   306,   307,   762,     0,     0,   308,
     543,   544,   251,   252,     0,   253,   254,     0,     0,   255,
     256,   257,   258,     0,     0,     0,     0,     0,   545,     0,
       0,     0,     0,     0,    86,    87,   259,    88,   169,    90,
     313,     0,   314,     0,     0,   315,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   261,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   263,   264,   265,
     266,   267,   268,   269,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,    48,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,     0,     0,     0,  1174,   305,   306,   307,     0,
       0,     0,   308,   543,   544,   251,   252,     0,   253,   254,
       0,     0,   255,   256,   257,   258,     0,     0,     0,     0,
       0,   545,     0,     0,     0,     0,     0,    86,    87,   259,
      88,   169,    90,   313,     0,   314,     0,     0,   315,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   261,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     263,   264,   265,   266,   267,   268,   269,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,    48,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,     0,     0,     0,     0,   305,
     306,   307,  1182,     0,     0,   308,   543,   544,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   791,
     792,     0,     0,     0,   545,   793,     0,   794,     0,     0,
      86,    87,     0,    88,   169,    90,   313,     0,   314,   795,
       0,   315,     0,     0,     0,     0,     0,    33,    34,    35,
      36,   417,   418,   419,     0,     0,     0,     0,     0,   199,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     420,    48,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,     0,   443,     0,     0,     0,     0,
     963,     0,     0,     0,     0,     0,   796,   444,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   201,
       0,     0,     0,     0,   168,    82,    83,    84,   797,     0,
      86,    87,    28,    88,   169,    90,     0,     0,     0,    92,
      33,    34,    35,    36,     0,   198,     0,     0,   798,     0,
       0,     0,   199,    97,     0,     0,     0,     0,   799,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,   491,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   964,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   201,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,   791,
     792,     0,    92,     0,     0,   793,     0,   794,     0,     0,
       0,     0,     0,     0,     0,     0,    97,     0,     0,   795,
       0,   202,     0,     0,     0,     0,   103,    33,    34,    35,
      36,   417,   418,   419,     0,     0,     0,     0,     0,   199,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     420,    48,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,     0,   443,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   796,   444,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   201,
       0,     0,     0,     0,   168,    82,    83,    84,   797,     0,
      86,    87,    28,    88,   169,    90,     0,     0,     0,    92,
      33,    34,    35,    36,     0,   198,     0,     0,   798,     0,
       0,     0,   199,    97,     0,     0,     0,     0,   799,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,   500,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   201,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,    28,    88,   169,    90,     0,
       0,     0,    92,    33,    34,    35,    36,     0,   198,     0,
       0,     0,     0,     0,     0,   199,    97,     0,     0,     0,
       0,   202,     0,     0,   566,     0,   103,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   200,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   586,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   201,     0,     0,     0,     0,
     168,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     169,    90,    28,     0,   918,    92,     0,     0,     0,     0,
      33,    34,    35,    36,     0,   198,     0,     0,     0,    97,
       0,     0,   199,     0,   202,     0,     0,     0,     0,   103,
       0,     0,     0,     0,    48,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1027,   200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1028,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   201,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,    28,    88,   169,    90,     0,
       0,     0,    92,    33,    34,    35,    36,     0,   198,     0,
       0,     0,     0,     0,     0,   199,    97,     0,     0,     0,
       0,   202,     0,     0,     0,     0,   103,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   200,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1053,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   201,     0,     0,     0,     0,
     168,    82,    83,    84,    85,     0,    86,    87,    28,    88,
     169,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   198,     0,     0,     0,     0,     0,     0,   199,    97,
       0,     0,     0,     0,   202,     0,     0,     0,     0,   103,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   201,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   169,    90,     0,     0,     0,    92,     0,
       0,     0,   417,   418,   419,     0,     0,     0,     0,     0,
       0,     0,    97,     0,     0,     0,     0,   202,     0,     0,
       0,   420,   103,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,     0,   443,   417,   418,   419,
       0,     0,     0,     0,     0,     0,     0,     0,   444,     0,
       0,     0,     0,     0,     0,     0,   420,     0,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
       0,   443,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   444,   417,   418,   419,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   420,   904,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,     0,   443,  1002,
    1003,  1004,     0,     0,     0,     0,     0,     0,     0,     0,
     444,     0,     0,     0,     0,     0,     0,     0,  1005,   948,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1028,  1002,  1003,  1004,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1005,  1254,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,     0,
       0,  1002,  1003,  1004,     0,     0,     0,     0,     0,     0,
       0,     0,  1028,     0,     0,     0,     0,     0,     0,     0,
    1005,  1164,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1028,  1002,  1003,
    1004,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1005,  1312,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,     0,     0,     0,    33,    34,    35,    36,     0,   198,
       0,     0,     0,     0,  1028,     0,   199,     0,     0,     0,
       0,     0,     0,  1394,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   216,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   201,     0,     0,     0,
    1480,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,     0,     0,     0,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   218,     0,     0,   774,   420,
     103,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,     0,   443,   417,   418,   419,     0,     0,
       0,     0,     0,     0,     0,     0,   444,     0,     0,     0,
       0,     0,     0,     0,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   775,   443,
    1002,  1003,  1004,     0,     0,     0,     0,     0,     0,     0,
       0,   444,     0,     0,     0,     0,     0,     0,     0,  1005,
    1317,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1002,  1003,  1004,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1028,     0,     0,     0,
       0,     0,  1005,     0,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,  1027,  1003,  1004,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1028,
       0,     0,     0,     0,  1005,     0,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,   419,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1028,     0,     0,     0,   420,     0,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,  1004,
     443,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   444,     0,     0,     0,  1005,     0,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1005,  1028,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,  1027,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1028,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,     0,   443,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   444,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1028,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,     0,   443,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   444
};

static const yytype_int16 yycheck[] =
{
       5,     6,   121,     8,     9,    10,    11,    12,   146,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    54,  1075,    28,    29,   652,   172,   100,    91,     4,
       4,     4,    95,    96,     4,    55,    32,    42,   373,   704,
     485,   373,   479,   480,   623,    50,   504,    52,    44,   877,
      55,   622,    57,    49,   173,    54,   100,   120,  1062,   151,
     443,   219,    42,   866,   741,    30,   100,   214,    30,    30,
     516,   146,   602,   510,    79,   773,   475,   100,   897,     9,
     780,   475,  1071,   962,     9,     9,   230,     9,    30,     9,
      14,     9,    14,     9,   913,   100,    14,    30,     9,    79,
       9,     9,     9,     9,   550,     4,     9,    67,    33,     9,
      46,     9,     9,   512,     9,     9,     9,     9,   512,     9,
       9,     9,    35,     9,     9,     9,   170,     9,    67,     9,
     949,     9,     9,     9,   102,   103,   170,    80,    86,   231,
       9,    86,    46,   751,   102,   103,    80,   170,     9,    80,
      67,    80,    80,   359,    35,    30,  1610,   363,   202,     4,
     111,   130,   131,   130,   131,   170,     8,    80,   202,   151,
     130,   131,   177,    29,   218,    46,    46,     0,   624,   202,
      46,   118,   172,   389,   218,   391,   392,   393,   394,   126,
     172,    47,   186,   157,    50,   998,   186,   202,   151,    80,
     148,    35,   186,   148,    35,   148,   149,   150,    67,   186,
      67,  1665,   151,   218,   358,    67,    67,    67,   169,   186,
      67,   190,   190,   151,    67,   189,    67,   232,    67,   189,
     235,   165,   190,   186,   165,    67,   165,   242,   243,   160,
      67,    67,    67,    67,    80,    58,    80,  1251,   172,    80,
     189,   187,   165,   173,  1258,   188,  1260,   173,   188,   189,
    1249,   236,   187,   409,   188,   240,   188,  1146,    81,   186,
     188,    84,   972,    80,   974,  1279,   121,   188,    67,   188,
     188,   188,   188,   187,   165,   188,     4,  1106,   188,    67,
     188,   188,   325,   188,   188,   188,   188,    80,   188,   188,
     188,   187,   187,   187,   173,   187,   350,   187,   916,   187,
     187,   187,   173,   184,   189,   894,   350,   187,   184,   765,
      80,   187,  1125,    67,   770,   855,   325,   350,   173,   186,
     189,   165,   189,    51,   165,   186,    54,   189,   457,   186,
     403,   487,   189,   186,   349,   350,   189,   494,   189,    98,
     189,   356,    98,    71,   186,   191,   361,   189,   151,   186,
     187,   189,   151,   189,   189,   189,  1370,   400,    54,   349,
    1359,    89,  1361,    91,    51,   380,   186,    95,    96,   172,
      66,   119,   156,   388,   191,   458,    51,    80,   126,   452,
     453,   454,   455,   186,   399,  1060,   189,   842,   186,    80,
     189,   400,   120,    35,   186,   165,   155,   186,   191,   155,
     186,   189,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   458,   444,
      30,   446,   447,   448,  1142,   189,   186,    98,    80,  1126,
     449,    98,   373,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   613,   185,   149,   150,
    1459,   476,   477,   910,   479,   480,   481,   482,   443,   156,
     379,   443,   443,   488,   883,   629,   491,   631,     4,   883,
     183,   156,    35,    98,   171,   500,   189,   502,   518,   498,
      98,   443,   640,  1074,   155,   510,  1309,   449,   155,    67,
    1325,  1090,   151,   518,  1093,   520,   186,   151,   236,   332,
     333,   334,   240,   969,   186,   155,   244,   172,   449,   186,
      80,   866,   186,   475,   866,   918,    86,    80,   172,   697,
      67,   186,   387,    80,   130,   131,   693,   186,   523,    86,
     155,   189,   365,    78,   475,   189,   498,   155,   188,   189,
     706,   566,   172,     4,    14,   640,   610,   509,   172,   186,
     512,   729,   130,   131,    99,  1378,   186,   498,    80,    54,
      30,   155,   186,    29,    86,    26,    27,    35,   509,   188,
    1267,   512,  1269,   151,  1259,   658,  1411,   188,    48,   149,
     150,   745,   746,   130,   131,   610,   107,   325,   752,   753,
      56,   456,   149,   150,   115,   116,   117,   118,   119,   120,
    1435,   188,  1437,    80,  1613,   101,   102,   103,   186,    86,
     146,   156,    78,   158,   159,    80,   161,   162,   163,   157,
     787,    86,   800,   447,   194,   650,   148,   149,   150,   807,
    1108,    67,   170,    99,    50,    51,    52,   662,   195,   100,
     752,   107,  1099,   998,   188,    29,   998,  1238,   188,   387,
      66,   189,   476,  1110,  1253,  1340,   188,   481,   396,    67,
     172,   182,   400,    47,   189,   403,    50,   151,   134,   135,
     695,   148,   149,   150,   186,    80,  1749,    29,   186,  1376,
    1145,    86,    80,   186,   149,   150,   152,    67,    86,   155,
     156,  1764,   158,   159,   151,   161,   162,   163,   186,   724,
     101,   102,   103,   155,   623,    47,    48,    78,   188,   170,
     176,   449,   450,   451,   452,   453,   454,   455,    72,    73,
     186,   554,   115,   116,   117,   558,    78,  1412,    99,    45,
     563,   172,   757,    78,  1743,    87,    66,   475,   104,   105,
     106,   202,   151,  1209,   149,   150,   186,    99,   773,  1758,
     211,   149,   150,   769,    99,  1220,   186,   218,    72,    73,
     498,    78,   193,    80,     9,   260,   151,   262,   151,  1466,
    1125,   186,  1371,  1125,   512,   236,   943,    98,    99,   240,
       8,   152,    99,   778,   155,   523,   188,   158,   159,   186,
     161,   162,   163,   128,   129,   151,   188,   189,  1255,    14,
     152,   188,   189,   155,   542,   151,   158,   159,   126,   161,
     162,   163,   188,   158,   159,  1281,   161,   162,   163,   190,
     126,   988,   317,  1639,  1640,    14,   851,   565,   995,  1635,
    1636,  1297,   187,   172,   917,    14,    98,   789,   790,   156,
     865,   158,   159,   187,   161,   162,   163,   187,   107,   187,
     869,   387,   186,   591,   592,   187,   192,   186,   186,   854,
     854,   854,     9,   148,   854,    78,   891,    80,    81,  1717,
      90,     9,   189,   188,   191,   187,   901,   187,   339,   904,
     187,   906,   187,    78,    14,   910,    99,   348,  1736,   350,
     172,   186,     9,   186,   355,    80,  1744,   187,   763,   187,
    1357,   362,   397,  1081,    99,   400,   187,   869,   188,   115,
     116,   117,   118,   119,   120,  1381,   128,   186,  1615,   881,
     658,   883,    29,   948,  1390,   866,   387,   187,   869,    30,
      67,   129,   171,   918,  1400,   854,   918,   918,   132,   955,
     881,   151,   883,     9,   809,   158,   159,   187,   161,   162,
     163,    78,   151,  1120,  1309,    14,   918,  1309,   877,   184,
     155,   956,  1055,   158,   159,     9,   161,   162,   163,     9,
     173,    78,    99,    29,     9,   894,   182,    14,   997,   187,
     813,  1000,   188,   128,   817,   190,   193,     9,   950,   854,
      14,   193,    99,   193,   189,   187,   187,    50,    51,    52,
      56,    54,   186,   741,  1171,   743,   193,  1473,   151,   950,
     875,  1178,  1659,    66,   121,  1055,   115,   116,   117,   118,
     119,   120,    78,  1378,   485,   763,  1378,   134,   135,   156,
    1055,   158,   159,   160,   161,   162,   163,   187,    98,   777,
     778,    87,   537,    99,   188,   152,   188,   132,   155,   156,
       9,   158,   159,   151,   161,   162,   163,   998,   187,   186,
     186,  1086,   523,   151,  1059,  1059,  1059,   186,   151,  1059,
      14,   809,    14,   189,  1099,   189,   188,   815,   134,   135,
     189,   819,   820,   182,    14,  1110,  1111,  1734,   193,   189,
     187,    26,    27,   958,   188,   960,   152,   184,    30,   155,
     156,   839,   158,   159,   640,   161,   162,   163,   186,  1276,
      47,    48,    49,    50,    51,    52,   854,  1142,  1134,   186,
     176,    14,    30,   186,   619,   620,    14,  1152,   186,    66,
     186,   869,    49,   628,   186,   186,     9,   875,   187,  1605,
    1059,  1607,  1137,   881,   132,   883,   188,   188,   186,    14,
    1616,    78,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,   132,     9,   187,    66,    78,
     193,  1090,    99,     9,  1093,    80,     9,   186,   132,   917,
     107,   108,   186,  1341,  1125,   188,    14,   187,   649,    80,
      99,   929,   930,   931,  1059,   189,  1662,  1336,   187,   186,
     186,   132,    64,    65,   189,  1230,   189,   188,     9,  1234,
      87,  1236,   950,   193,   148,   124,   189,    30,   956,  1244,
     958,    74,   960,   188,   187,   173,   188,   763,   155,  1254,
    1255,   158,   159,   132,   161,   162,   163,    30,   187,     9,
       4,   187,   980,    78,   187,   132,   707,     9,   189,   158,
     159,   190,   161,   162,   163,    14,    80,    78,   187,   997,
     186,   189,  1000,   187,    99,   190,   187,  1433,   130,   131,
     188,   187,   186,   809,   187,   187,   211,   186,    99,     9,
     132,   187,    46,    30,  1149,   188,   747,   187,   749,   108,
      78,  1029,   187,   189,   789,   790,  1762,  1035,   188,  1218,
    1038,   160,   763,  1769,   188,   188,   156,    14,  1227,    80,
     113,    99,   187,   187,   775,   132,   187,   778,   854,   189,
     155,  1059,  1405,   158,   159,   187,   161,   162,   163,  1354,
     132,    14,  1357,   172,  1253,   189,    80,   158,   159,   875,
     161,   162,   163,   188,   108,    14,    14,    80,   809,   113,
     187,   115,   116,   117,   118,   119,   120,   121,   189,    78,
     186,   856,   187,   132,    14,   186,   827,    14,  1309,    14,
     158,   159,   188,   161,   162,   163,   188,   872,   188,     9,
      99,   842,   843,   189,  1122,   190,    56,    80,  1126,  1408,
     885,   172,  1387,   854,   158,   159,   186,   161,   186,  1137,
      26,    27,    80,     9,   339,   188,   111,    98,   151,    98,
     163,  1149,    33,   348,   875,   350,    14,   169,   182,   914,
     355,   186,   958,   186,   960,  1450,   190,   362,   188,  1595,
      78,   187,    80,   173,    80,   166,   155,  1378,   187,   158,
     159,  1360,   161,   162,   163,     4,     9,  1366,    80,  1368,
     188,    99,  1371,   187,  1319,    47,    48,    49,    50,    51,
      52,  1380,    54,  1630,   187,    26,    27,   189,    14,    80,
    1335,    14,    80,    14,    66,  1213,  1471,  1472,    80,    14,
      80,   976,   563,  1725,   979,   455,   813,    46,   452,   450,
     911,   817,   857,  1740,  1143,   956,  1472,   958,  1736,   960,
     568,   962,   963,  1294,  1462,  1579,  1413,  1768,   156,  1495,
     158,   159,  1331,   161,   162,   163,  1756,  1382,  1591,  1458,
    1327,  1063,   996,  1059,   930,  1121,  1391,  1122,   945,  1267,
     881,  1269,   400,   356,   789,  1320,  1693,  1590,   981,  1046,
    1029,   189,    -1,   191,    -1,  1464,    -1,    -1,    -1,   108,
     485,    -1,    78,  1711,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1653,  1590,    -1,    99,    -1,  1600,  1441,    -1,  1073,    -1,
    1075,  1319,    -1,    -1,   541,   211,    -1,  1325,    -1,    -1,
    1729,    -1,    -1,  1331,    -1,    -1,    -1,    -1,  1059,   158,
     159,    -1,   161,    -1,    -1,  1470,    -1,  1102,    -1,    -1,
    1105,  1476,    -1,  1149,    -1,   572,  1481,    -1,    -1,    -1,
      -1,    -1,    -1,   182,  1085,    -1,    26,    27,    -1,   155,
      30,   190,   158,   159,    -1,   161,   162,   163,  1376,    75,
      76,    77,    -1,    -1,  1382,    -1,    -1,    -1,    -1,  1387,
     211,    87,    -1,  1391,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1157,    -1,    -1,    -1,  1161,  1405,    -1,  1130,
    1408,    -1,    -1,  1411,    -1,    -1,  1137,    -1,    -1,    -1,
      -1,    -1,  1218,  1421,  1145,  1146,    -1,    -1,  1149,    -1,
    1428,  1227,    -1,    -1,    -1,    -1,    -1,  1435,    -1,  1437,
     136,   137,   138,   139,   140,  1443,    -1,    -1,    -1,    -1,
      -1,   147,    -1,   339,   649,    -1,    -1,   153,   154,  1214,
    1215,    -1,   348,    -1,    -1,    -1,    -1,    -1,  1466,   355,
      -1,   167,  1470,  1471,  1472,    -1,   362,    -1,  1476,    -1,
      -1,    -1,    -1,  1481,    -1,   181,    -1,   373,    -1,  1774,
      -1,    -1,    -1,    -1,    -1,  1620,    -1,  1782,    -1,  1220,
      -1,    -1,    -1,  1788,    -1,    -1,  1791,    -1,    -1,    -1,
      -1,    -1,   707,    -1,    -1,    -1,    -1,    -1,   339,    -1,
      -1,    -1,    -1,  1319,    -1,    -1,    -1,   348,    -1,    -1,
      -1,    -1,    -1,    -1,   355,  1660,  1661,    -1,  1717,    -1,
      -1,   362,  1667,    -1,    -1,  1341,  1301,    -1,  1303,    -1,
      -1,   211,   747,    -1,   749,    -1,    -1,  1736,    -1,   776,
      -1,    -1,    -1,    -1,  1360,  1744,    -1,    -1,    -1,    -1,
    1366,    -1,  1368,    -1,   791,   792,   793,   794,    -1,  1704,
     775,  1336,   799,    -1,  1380,    -1,  1382,    -1,    -1,    -1,
      -1,    -1,  1590,    -1,    -1,  1391,    -1,    -1,  1319,   485,
     260,    -1,   262,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,  1611,    -1,    -1,    -1,  1615,    -1,    -1,
      -1,    -1,  1620,    -1,    -1,    -1,    -1,    99,    -1,    -1,
    1628,    -1,   827,    -1,    -1,    -1,    -1,  1635,  1636,    -1,
      -1,  1639,  1640,    -1,    -1,  1770,    -1,   842,   843,    -1,
      -1,    -1,  1777,    -1,    -1,  1653,    -1,   317,    -1,    -1,
      -1,  1382,  1660,  1661,   485,    -1,  1387,    -1,  1464,  1667,
    1391,    -1,    -1,    -1,  1470,    -1,   893,    -1,    -1,   339,
    1476,    -1,    -1,    -1,    -1,  1481,   158,   159,   348,   161,
     162,   163,    -1,    -1,    -1,   355,    -1,    -1,    -1,    -1,
      -1,    -1,   362,    -1,    -1,    -1,  1704,    -1,    -1,    -1,
      -1,    -1,  1710,   373,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
    1728,    -1,    -1,    -1,    -1,    -1,    -1,   397,  1493,     4,
     400,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1470,
    1471,  1472,    -1,    -1,    -1,  1476,    -1,    -1,    -1,    -1,
    1481,    -1,    -1,   649,    -1,    64,    65,   962,   963,    -1,
      -1,    -1,  1770,    -1,    -1,    -1,    -1,    -1,    -1,  1777,
      -1,    46,    -1,   443,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   707,    -1,    -1,  1620,   485,  1043,    -1,   649,    -1,
      -1,   130,   131,   115,   116,   117,   118,   119,   120,  1594,
      -1,    -1,    -1,   108,   126,   127,    -1,    -1,   113,    -1,
     115,   116,   117,   118,   119,   120,   121,    -1,    -1,    -1,
      -1,   747,    -1,   749,  1660,  1661,    -1,    -1,    -1,    -1,
      -1,  1667,    -1,    -1,    -1,    -1,    -1,   537,   538,    -1,
     162,   541,   164,    -1,    -1,    -1,   707,     4,   187,   775,
    1085,    -1,    78,   158,   159,   177,   161,   179,    -1,  1620,
     182,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1704,    -1,
      -1,    -1,   572,    99,    -1,  1711,    -1,   182,    -1,    -1,
      -1,  1676,    -1,    -1,    -1,   190,   747,    -1,   749,    46,
      -1,  1148,    -1,    -1,    -1,  1130,    -1,    -1,    -1,  1660,
    1661,   827,    -1,    -1,    -1,    -1,  1667,  1164,    -1,  1166,
    1145,  1146,    -1,    -1,   775,    -1,   842,   843,    -1,   619,
     620,    -1,    -1,    -1,    -1,  1182,   152,    -1,   628,   155,
     156,    -1,   158,   159,  1770,   161,   162,   163,    -1,    -1,
     866,  1777,    -1,  1704,    -1,    -1,    -1,    -1,    -1,   649,
      -1,   108,    -1,    -1,  1749,    -1,   113,    -1,   115,   116,
     117,   118,   119,   120,   121,    -1,   827,    -1,    -1,  1764,
      -1,    -1,    -1,    -1,    -1,   115,   116,   117,   118,   119,
     120,   842,   843,    -1,    78,  1220,   126,   127,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   158,   159,    -1,   161,    99,    -1,   707,     4,  1770,
      -1,    -1,    -1,    -1,    -1,    -1,  1777,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   164,   182,    -1,   121,    -1,    -1,
      -1,    -1,    -1,   190,    -1,     4,   962,   963,    64,    65,
     134,   135,   182,    -1,    -1,    -1,    -1,   747,    -1,   749,
      46,  1308,    -1,    -1,    -1,  1312,    -1,  1314,   152,    -1,
    1317,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,   998,    -1,    -1,   775,   776,    46,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   789,
     790,   791,   792,   793,   794,   795,    -1,    -1,    -1,   799,
      -1,   962,   963,    -1,   130,   131,    -1,    26,    27,    -1,
     810,    30,   108,    -1,    -1,    -1,    -1,   113,    -1,   115,
     116,   117,   118,   119,   120,   121,    -1,   827,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1394,    -1,   108,
     840,    -1,   842,   843,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,    -1,    -1,    -1,   856,   857,    -1,  1085,
      -1,   187,   158,   159,    -1,   161,   866,    -1,    -1,    -1,
      -1,    -1,   872,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   885,   182,    -1,    -1,   158,
     159,    -1,   161,   893,   190,    -1,   896,    -1,    -1,  1125,
      -1,    -1,    -1,    -1,  1130,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,   182,   914,    -1,    -1,    -1,   918,  1145,
    1146,   190,  1479,  1480,  1085,     4,    -1,    29,  1485,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   962,   963,    66,    -1,    -1,    46,    -1,  1130,
      -1,    -1,    -1,    -1,    -1,    -1,   976,    -1,    -1,   979,
      -1,   981,    -1,    -1,  1145,  1146,    -1,    -1,    -1,    -1,
      -1,    -1,   211,    -1,  1220,    -1,   996,    -1,   998,    -1,
      -1,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,   108,
      -1,    -1,    -1,    -1,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,  1043,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,  1220,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,  1073,    54,  1075,    -1,  1634,    -1,   158,
     159,    -1,   161,  1309,    -1,  1085,    66,    -1,  1645,    -1,
      -1,   193,    -1,    -1,  1651,    -1,    64,    65,  1655,    -1,
      -1,    -1,  1102,   182,    -1,  1105,    -1,    -1,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     339,    -1,    -1,    -1,    -1,  1125,    -1,    -1,    -1,   348,
    1130,    -1,    -1,    -1,    -1,    -1,   355,    -1,    -1,    -1,
      -1,    -1,    -1,   362,    -1,  1145,  1146,    -1,  1148,    -1,
      -1,    -1,  1378,    -1,   373,    -1,    -1,  1157,  1715,    -1,
      -1,  1161,   130,   131,  1164,    -1,  1166,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1182,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,  1214,  1215,    -1,  1217,    -1,    -1,
    1220,    10,    11,    12,   443,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,   485,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1301,    -1,  1303,    -1,    -1,    -1,    -1,  1308,  1309,
      -1,    -1,  1312,    -1,  1314,    -1,    -1,  1317,    -1,    -1,
      -1,    -1,   541,    -1,    -1,  1325,  1326,    -1,    -1,  1329,
      64,    65,    -1,    10,    11,    12,  1336,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,   190,
      -1,    -1,    29,   572,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,  1378,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,  1394,    -1,   130,   131,    -1,    -1,
      -1,   190,    -1,  1403,  1404,    -1,    -1,    -1,    -1,    -1,
      -1,  1411,    -1,  1413,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
     649,    -1,    -1,    -1,    -1,  1435,    -1,  1437,    -1,    -1,
      -1,    -1,    -1,  1443,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1479,
    1480,    66,    -1,    -1,    -1,  1485,    -1,  1487,   707,    -1,
      -1,    -1,    -1,  1493,    -1,  1495,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      10,    11,    12,   190,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,   747,    29,
     749,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,   260,    54,   262,   775,   776,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    47,    48,    -1,
      -1,    -1,   791,   792,   793,   794,   795,    -1,    -1,    -1,
     799,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,   810,    -1,    -1,  1594,    75,    76,    77,    78,    29,
      -1,    -1,    -1,    -1,    -1,   190,    -1,    87,   827,    -1,
     317,  1611,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,   840,    -1,   842,   843,    -1,    56,    -1,  1628,    -1,
      -1,    -1,    -1,    -1,  1634,    -1,    -1,    -1,   857,    -1,
      -1,    -1,    -1,    -1,    -1,  1645,    -1,   866,    78,    -1,
      -1,  1651,    -1,    -1,   134,  1655,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    99,
      -1,    -1,    -1,    -1,   893,    -1,  1676,   896,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,    -1,   188,    -1,
     397,    -1,    -1,   400,    -1,    -1,   176,    -1,    -1,   918,
      -1,    -1,    -1,    -1,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1715,    -1,    -1,    -1,    -1,
      -1,    -1,   152,    -1,  1724,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,   165,    -1,    -1,    -1,    -1,
    1740,    -1,    -1,   962,   963,    -1,   176,    -1,    -1,  1749,
      -1,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1764,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   996,    -1,   998,
      -1,    -1,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1043,    -1,    -1,    -1,    -1,    -1,
     537,   538,    -1,    29,   541,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,   572,  1085,    26,    27,    -1,
      66,    30,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,  1125,    -1,    -1,    -1,
      -1,  1130,   619,   620,    -1,    66,    -1,    -1,    -1,    -1,
      -1,   628,    -1,    -1,    -1,    -1,  1145,  1146,    -1,  1148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1164,    -1,  1166,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1182,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,   190,    10,    11,    12,  1217,    -1,
      -1,  1220,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    56,   190,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,   211,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   776,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,   789,   790,   791,   792,   793,   794,   795,  1308,
    1309,    -1,   799,  1312,    -1,  1314,    -1,    -1,  1317,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1326,    -1,    -1,
    1329,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,   165,    -1,   856,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,  1378,
      -1,    -1,    -1,    -1,    -1,   872,    -1,    -1,   186,    -1,
      -1,    -1,   187,    -1,    -1,  1394,    -1,    -1,   885,    -1,
     339,    -1,    -1,    -1,  1403,  1404,   893,    -1,    -1,   348,
      -1,    -1,    -1,    -1,  1413,    -1,   355,    10,    11,    12,
      -1,    -1,    -1,   362,    -1,    -1,    -1,   914,    -1,    -1,
      -1,    -1,    -1,    -1,   373,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
    1479,  1480,    -1,    -1,    -1,    -1,  1485,    -1,  1487,   976,
      -1,    -1,   979,    -1,   981,    -1,  1495,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   443,    -1,    -1,    -1,    -1,   996,
      -1,    -1,    -1,    -1,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,    -1,    10,    11,    12,   485,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1043,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,  1073,    54,  1075,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,    66,
      -1,    -1,   541,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1102,    -1,    -1,  1105,    -1,
      75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    87,   572,    -1,  1634,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,  1645,    -1,    -1,    -1,
      -1,    -1,  1651,    -1,    -1,    -1,  1655,    -1,    -1,    -1,
      -1,  1148,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,
    1157,   126,    -1,    -1,  1161,    -1,    -1,  1164,    -1,  1166,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,  1182,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
     649,    -1,   167,    -1,    -1,    -1,  1715,    -1,    -1,    -1,
      -1,    -1,    -1,   190,    -1,  1724,   181,  1214,  1215,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,
      -1,  1740,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   707,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,   747,    -1,
     749,    -1,    -1,    -1,  1301,    -1,  1303,    -1,    -1,    -1,
      -1,  1308,    -1,    -1,    -1,  1312,    -1,  1314,    -1,    -1,
    1317,    -1,    -1,    -1,    -1,    -1,   775,   776,  1325,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,  1336,
      -1,    -1,   791,   792,   793,   794,   795,    -1,    -1,    -1,
     799,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    29,    54,    -1,   827,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,   842,   843,    -1,    -1,  1394,    -1,    -1,
      -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1411,    -1,    -1,   866,   190,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1435,    -1,
    1437,    -1,    -1,    -1,   893,    99,  1443,    -1,    -1,    -1,
      -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   115,   116,   117,   118,   119,   120,    -1,    -1,   918,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,   135,  1479,  1480,    -1,    -1,    -1,    -1,  1485,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1493,    -1,   152,    -1,
      -1,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
     188,    -1,    -1,   962,   963,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,   182,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   996,    -1,   998,
      -1,    -1,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1043,    -1,    -1,  1594,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1611,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1628,    -1,    -1,    -1,    -1,  1085,  1634,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1645,    -1,
      -1,    -1,    -1,    -1,  1651,    -1,    -1,    -1,  1655,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,  1125,    54,    -1,  1676,
      -1,  1130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,  1145,  1146,    -1,  1148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1164,    -1,  1166,  1715,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,  1182,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,  1749,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    48,    -1,  1764,    -1,    -1,
      53,  1220,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,    82,
      83,    -1,    -1,    -1,    87,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,   107,   108,   109,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,  1308,
    1309,   144,    -1,  1312,   147,  1314,    -1,    -1,  1317,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,   164,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,   177,    -1,   179,    -1,   181,   182,
     183,    -1,    -1,   186,    -1,   188,   189,   190,   191,   192,
      -1,   194,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,  1378,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,  1394,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
    1479,  1480,    -1,    -1,    -1,    -1,  1485,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1494,    46,    47,    48,    -1,
      -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
     100,    -1,    -1,   188,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,   115,   116,   117,   118,   119,
     120,    -1,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,
      -1,   181,   182,   183,    -1,  1634,   186,    -1,   188,   189,
     190,   191,   192,    -1,   194,   195,  1645,     3,     4,     5,
       6,     7,  1651,    -1,    -1,    -1,  1655,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,  1678,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    71,  1715,    -1,    -1,    75,
      76,    77,    78,    79,    80,    -1,    82,    83,    -1,    -1,
      -1,    87,    88,    89,    90,    -1,    92,    -1,    94,    -1,
      96,    -1,    -1,    99,   100,    -1,    -1,    -1,   104,   105,
     106,   107,   108,   109,   110,    -1,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    -1,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    -1,
      -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,
     176,   177,    -1,   179,    -1,   181,   182,   183,    -1,    -1,
     186,    -1,   188,   189,    -1,   191,   192,    -1,   194,   195,
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
      91,    92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,
      -1,    -1,    -1,   104,   105,   106,   107,    -1,   109,   110,
      -1,   112,    -1,   114,   115,   116,   117,   118,   119,   120,
      -1,   122,   123,   124,    -1,   126,   127,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,   170,
      -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,
     181,   182,   183,    -1,    -1,   186,    -1,   188,   189,    -1,
     191,   192,    -1,   194,   195,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    71,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    80,    -1,    82,    83,    -1,    -1,    -1,    87,
      88,    89,    90,    -1,    92,    -1,    94,    -1,    96,    97,
      -1,    99,   100,    -1,    -1,    -1,   104,   105,   106,   107,
      -1,   109,   110,    -1,   112,    -1,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,   123,   124,    -1,   126,   127,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,   164,    -1,    -1,   167,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,
     188,   189,    -1,   191,   192,    -1,   194,   195,     3,     4,
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
     182,   183,    -1,    -1,   186,    -1,   188,   189,   190,   191,
     192,    -1,   194,   195,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,
      -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    71,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,
      89,    90,    -1,    92,    -1,    94,    95,    96,    -1,    -1,
      99,   100,    -1,    -1,    -1,   104,   105,   106,   107,    -1,
     109,   110,    -1,   112,    -1,   114,   115,   116,   117,   118,
     119,   120,    -1,   122,   123,   124,    -1,   126,   127,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,
      -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,
     189,    -1,   191,   192,    -1,   194,   195,     3,     4,     5,
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
      -1,    94,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,   107,    -1,   109,   110,    -1,   112,
      -1,   114,   115,   116,   117,   118,   119,   120,    -1,   122,
     123,   124,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,   144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,   164,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
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
      90,    -1,    92,    93,    94,    -1,    96,    -1,    -1,    99,
     100,    -1,    -1,    -1,   104,   105,   106,   107,    -1,   109,
     110,    -1,   112,    -1,   114,   115,   116,   117,   118,   119,
     120,    -1,   122,   123,   124,    -1,   126,   127,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
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
     188,   189,    -1,   191,   192,    -1,   194,   195,     3,     4,
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
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,   186,    -1,   188,   189,    -1,   191,   192,    -1,   194,
     195,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
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
      -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,   192,    -1,   194,   195,     3,
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
      -1,    -1,   186,    -1,   188,    -1,    -1,   191,   192,    -1,
     194,   195,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    47,    48,    -1,    -1,
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
     161,   162,   163,    -1,   165,    -1,   167,    -1,    -1,   170,
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
      -1,   189,    -1,   191,   192,    -1,   194,   195,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
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
     182,   183,    -1,    -1,   186,    -1,    10,    11,    12,   191,
     192,    -1,   194,   195,     3,     4,     5,     6,     7,    -1,
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
      -1,   170,    -1,    -1,   188,    -1,    -1,   176,    -1,    -1,
      -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,    -1,   194,   195,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
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
      -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   115,   116,   117,   118,   119,   120,    -1,    -1,
     123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
     188,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
     183,    -1,    -1,   186,    -1,   188,    11,    12,   191,   192,
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
     170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,    -1,
      -1,   191,   192,    -1,   194,   195,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,
      -1,    10,    11,    12,   191,   192,    -1,   194,   195,     3,
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
      -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,   188,
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,
      -1,    -1,   186,   187,    -1,    -1,    -1,   191,   192,    -1,
     194,   195,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     181,   182,   183,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,    -1,   194,   195,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    47,
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
      -1,    -1,    -1,   191,   192,    -1,   194,   195,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
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
      -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
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
      -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,    10,
      11,    12,   191,   192,    -1,   194,   195,     3,     4,     5,
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
      -1,   167,    -1,    -1,   170,    -1,   187,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,
     186,    -1,    10,    11,    12,   191,   192,    -1,   194,   195,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    66,    -1,
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
     163,    -1,     3,     4,   167,     6,     7,   170,    -1,    10,
      11,    12,    13,   176,    -1,    -1,    -1,    -1,   181,   182,
     183,    -1,    -1,   186,    -1,    -1,    27,    -1,   191,   192,
      -1,   194,   195,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    54,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    68,    69,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,    -1,    -1,   126,   127,   128,   129,    -1,
      -1,    -1,   133,   134,   135,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,    -1,    -1,    -1,   158,   159,    27,
     161,   162,   163,   164,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,   189,    -1,
     191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    69,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,   126,   127,
     128,   129,    -1,    -1,    -1,   133,   134,   135,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,   152,    -1,    -1,    -1,    -1,    -1,
     158,   159,    -1,   161,   162,   163,   164,    27,   166,    29,
      -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   189,    -1,   191,    54,    -1,    56,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
     190,    -1,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,    -1,   127,   128,   129,    -1,    -1,
      -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
     152,    -1,    -1,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,   164,    27,   166,    29,    -1,   169,    -1,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,   190,    -1,
      54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,   126,   127,   128,   129,    -1,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    -1,    10,    11,    12,    13,   152,    -1,
      -1,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
     164,    27,   166,    29,    -1,   169,    -1,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,    -1,    54,    -1,
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
     176,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,    -1,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    69,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,   127,
     128,   129,    -1,    -1,    -1,   133,   134,   135,    75,    76,
      77,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,    -1,
     158,   159,    99,   161,   162,   163,   164,    -1,   166,    -1,
      -1,   169,     3,     4,     5,     6,     7,    -1,   176,    10,
      11,    12,    13,   120,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    54,   161,   162,   163,    -1,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,
      71,    72,    73,    74,   181,    -1,    -1,    78,    -1,   186,
      -1,    -1,    -1,    -1,   191,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   152,   153,   154,    -1,    -1,    -1,   158,   159,    -1,
     161,   162,   163,   164,    -1,   166,   167,    -1,   169,    10,
      11,    12,    -1,    -1,    -1,   176,   177,    -1,   179,    -1,
     181,   182,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,    66,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    27,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,   126,   127,   128,   129,   187,    -1,    -1,   133,
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
     121,   122,    -1,    -1,    -1,   126,   127,   128,   129,    -1,
      -1,    -1,   133,   134,   135,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,    -1,    -1,    -1,   158,   159,    27,
     161,   162,   163,   164,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    69,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,   127,
     128,   129,    30,    -1,    -1,   133,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    -1,    -1,    -1,   152,    53,    -1,    55,    -1,    -1,
     158,   159,    -1,   161,   162,   163,   164,    -1,   166,    67,
      -1,   169,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    99,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,   134,    66,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    67,   161,   162,   163,    -1,    -1,    -1,   167,
      75,    76,    77,    78,    -1,    80,    -1,    -1,   176,    -1,
      -1,    -1,    87,   181,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    47,
      48,    -1,   167,    -1,    -1,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    67,
      -1,   186,    -1,    -1,    -1,    -1,   191,    75,    76,    77,
      78,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    99,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,    66,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    67,   161,   162,   163,    -1,    -1,    -1,   167,
      75,    76,    77,    78,    -1,    80,    -1,    -1,   176,    -1,
      -1,    -1,    87,   181,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    67,   161,   162,   163,    -1,
      -1,    -1,   167,    75,    76,    77,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,   181,    -1,    -1,    -1,
      -1,   186,    -1,    -1,   189,    -1,   191,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    67,    -1,    69,   167,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,   181,
      -1,    -1,    87,    -1,   186,    -1,    -1,    -1,    -1,   191,
      -1,    -1,    -1,    -1,    99,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    67,   161,   162,   163,    -1,
      -1,    -1,   167,    75,    76,    77,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,   181,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    67,   161,
     162,   163,    -1,    -1,    -1,   167,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,   181,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    29,   191,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   132,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   132,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   132,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   132,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   132,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    75,    76,    77,    78,    -1,    80,
      -1,    -1,    -1,    -1,    66,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66
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
     213,   214,   215,   216,   219,   235,   236,   240,   243,   250,
     256,   316,   317,   325,   329,   330,   331,   332,   333,   334,
     335,   336,   338,   341,   353,   354,   355,   357,   358,   360,
     370,   371,   372,   374,   379,   382,   401,   409,   411,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     424,   437,   439,   441,   118,   119,   120,   133,   152,   162,
     186,   203,   235,   316,   335,   413,   335,   186,   335,   335,
     335,   104,   335,   335,   399,   400,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,    80,    87,
     120,   147,   186,   213,   354,   371,   374,   379,   413,   416,
     413,    35,   335,   428,   429,   335,   120,   126,   186,   213,
     248,   371,   372,   373,   375,   379,   410,   411,   412,   420,
     425,   426,   186,   326,   376,   186,   326,   345,   327,   335,
     221,   326,   186,   186,   186,   326,   188,   335,   203,   188,
     335,     3,     4,     6,     7,    10,    11,    12,    13,    27,
      29,    54,    56,    68,    69,    70,    71,    72,    73,    74,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   126,   127,   128,   129,   133,   134,
     135,   152,   156,   164,   166,   169,   176,   186,   203,   204,
     205,   215,   442,   457,   458,   460,   188,   332,   335,   189,
     228,   335,   107,   108,   155,   206,   209,   212,    80,   191,
     282,   283,   119,   126,   118,   126,    80,   284,   186,   186,
     186,   186,   203,   254,   445,   186,   186,   327,    80,    86,
     148,   149,   150,   434,   435,   155,   189,   212,   212,   203,
     255,   445,   156,   186,   445,   445,    80,   183,   189,   346,
      27,   325,   329,   335,   336,   413,   417,   217,   189,    86,
     377,   434,    86,   434,   434,    30,   155,   172,   446,   186,
       9,   188,    35,   234,   156,   253,   445,   120,   182,   235,
     317,   188,   188,   188,   188,   188,   188,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    54,    66,   188,    67,    67,   189,   151,
     127,   162,   164,   177,   179,   256,   315,   316,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    64,    65,   130,   131,   403,    67,   189,   408,   186,
     186,    67,   189,   191,   421,   186,   234,   235,    14,   335,
     188,   132,    45,   203,   398,    86,   325,   336,   151,   413,
     132,   193,     9,   384,   249,   325,   336,   413,   446,   151,
     186,   378,   403,   408,   187,   335,    30,   219,     8,   347,
       9,   188,   219,   220,   327,   328,   335,   203,   268,   223,
     188,   188,   188,   134,   135,   460,   460,   172,   186,   107,
     460,    14,   151,   134,   135,   152,   203,   205,   188,   188,
     229,   111,   169,   188,   155,   207,   210,   212,   155,   208,
     211,   212,   212,     9,   188,    98,   189,   413,     9,   188,
     126,   126,    14,     9,   188,   413,   438,   327,   325,   336,
     413,   416,   417,   187,   172,   246,   133,   413,   427,   428,
     188,    67,   403,   148,   435,    79,   335,   413,    86,   148,
     435,   212,   202,   188,   189,   241,   251,   361,   363,    87,
     186,   191,   348,   349,   351,   374,   419,   421,   439,    14,
      98,   440,   342,   343,   344,   278,   279,   401,   402,   187,
     187,   187,   187,   187,   190,   218,   219,   236,   243,   250,
     401,   335,   192,   194,   195,   203,   447,   448,   460,    35,
     165,   280,   281,   335,   442,   186,   445,   244,   234,   335,
     335,   335,    30,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   375,   335,   335,   423,
     423,   335,   430,   431,   126,   189,   204,   205,   420,   421,
     254,   203,   255,   445,   445,   253,   235,    35,   329,   332,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   156,   189,   203,   404,   405,   406,   407,
     420,   423,   335,   280,   280,   423,   335,   427,   234,   187,
     335,   186,   397,     9,   384,   187,   187,    35,   335,    35,
     335,   378,   187,   187,   187,   420,   280,   189,   203,   404,
     405,   420,   187,   217,   272,   189,   332,   335,   335,    90,
      30,   219,   266,   188,    28,    98,    14,     9,   187,    30,
     189,   269,   460,    29,    87,   215,   454,   455,   456,   186,
       9,    47,    48,    53,    55,    67,   134,   156,   176,   186,
     213,   215,   356,   371,   379,   380,   381,   203,   459,   217,
     186,   227,   212,     9,   188,    98,   212,     9,   188,    98,
      98,   209,   203,   335,   283,   380,    80,     9,   187,   187,
     187,   187,   187,   187,   187,   188,    47,    48,   452,   453,
     128,   259,   186,     9,   187,   187,    80,    81,   203,   436,
     203,    67,   190,   190,   199,   201,    30,   129,   258,   171,
      51,   156,   171,   365,   336,   132,     9,   384,   187,   151,
     460,   460,    14,   347,   278,   217,   184,     9,   385,   460,
     461,   403,   408,   403,   190,     9,   384,   173,   413,   335,
     187,     9,   385,    14,   339,   237,   128,   257,   186,   445,
     335,    30,   193,   193,   132,   190,     9,   384,   335,   446,
     186,   247,   242,   252,    14,   440,   245,   234,    69,   413,
     335,   446,   193,   190,   187,   187,   193,   190,   187,    47,
      48,    67,    75,    76,    77,    87,   134,   147,   176,   203,
     387,   389,   390,   393,   396,   203,   413,   413,   132,   257,
     403,   408,   187,   335,   273,    72,    73,   274,   217,   326,
     217,   328,    98,    35,   133,   263,   413,   380,   203,    30,
     219,   267,   188,   270,   188,   270,     9,   173,    87,   132,
     151,     9,   384,   187,   165,   447,   448,   449,   447,   380,
     380,   380,   380,   380,   383,   386,   186,   151,   186,   380,
     151,   189,    10,    11,    12,    29,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    66,   151,
     446,   190,   371,   189,   231,    98,   210,   203,    98,   211,
     203,   203,   190,    14,   413,   188,     9,   173,   203,   260,
     371,   189,   427,   133,   413,    14,   193,   335,   190,   199,
     460,   260,   189,   364,    14,   187,   335,   348,   420,   188,
     460,   184,   190,    30,   450,   402,    35,    80,   165,   404,
     405,   407,   404,   405,   460,    35,   165,   335,   380,   278,
     186,   371,   258,   340,   238,   335,   335,   335,   190,   186,
     280,   259,    30,   258,   460,    14,   257,   445,   375,   190,
     186,    14,    75,    76,    77,   203,   388,   388,   390,   391,
     392,    49,   186,    86,   148,   186,     9,   384,   187,   397,
      35,   335,   258,   190,    72,    73,   275,   326,   219,   190,
     188,    91,   188,   263,   413,   186,   132,   262,    14,   217,
     270,   101,   102,   103,   270,   190,   460,   132,   460,   203,
     454,     9,   187,   384,   132,   193,     9,   384,   383,   204,
     348,   350,   352,   187,   126,   204,   380,   432,   433,   380,
     380,   380,    30,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   459,    80,   232,
     203,   203,   380,   453,    98,    99,   451,     9,   288,   187,
     186,   329,   332,   335,   193,   190,   440,   288,   157,   170,
     189,   360,   367,   157,   189,   366,   132,   188,   450,   460,
     347,   461,    80,   165,    14,    80,   446,   413,   335,   187,
     278,   189,   278,   186,   132,   186,   280,   187,   189,   460,
     189,   188,   460,   258,   239,   378,   280,   132,   193,     9,
     384,   389,   391,   148,   348,   394,   395,   390,   413,   189,
     326,    30,    74,   219,   188,   328,   262,   427,   263,   187,
     380,    97,   101,   188,   335,    30,   188,   271,   190,   173,
     460,   132,   165,    30,   187,   380,   380,   187,   132,     9,
     384,   187,   132,   190,     9,   384,   380,    30,   187,   217,
     203,   460,   460,   371,     4,   108,   113,   119,   121,   158,
     159,   161,   190,   289,   314,   315,   316,   321,   322,   323,
     324,   401,   427,   190,   189,   190,    51,   335,   335,   335,
     347,    35,    80,   165,    14,    80,   335,   186,   450,   187,
     288,   187,   278,   335,   280,   187,   288,   440,   288,   188,
     189,   186,   187,   390,   390,   187,   132,   187,     9,   384,
     288,    30,   217,   188,   187,   187,   187,   224,   188,   188,
     271,   217,   460,   460,   132,   380,   348,   380,   380,   380,
     189,   190,   451,   128,   129,   177,   204,   443,   460,   261,
     371,   108,   324,    29,   121,   134,   135,   156,   162,   298,
     299,   300,   301,   371,   160,   306,   307,   124,   186,   203,
     308,   309,   290,   235,   460,     9,   188,     9,   188,   188,
     440,   315,   187,   285,   156,   362,   190,   190,    80,   165,
      14,    80,   335,   280,   113,   337,   450,   190,   450,   187,
     187,   190,   189,   190,   288,   278,   132,   390,   348,   190,
     217,   222,   225,    30,   219,   265,   217,   187,   380,   132,
     132,   217,   371,   371,   445,    14,   204,     9,   188,   189,
     443,   440,   301,   172,   189,     9,   188,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    54,    68,
      69,    70,    71,    72,    73,    74,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   133,   134,   136,
     137,   138,   139,   140,   152,   153,   154,   164,   166,   167,
     169,   176,   177,   179,   181,   182,   203,   368,   369,     9,
     188,   156,   160,   203,   309,   310,   311,   188,    80,   320,
     234,   291,   443,   443,    14,   235,   190,   286,   287,   443,
      14,    80,   335,   187,   186,   189,   188,   189,   312,   337,
     450,   285,   190,   187,   390,   132,    30,   219,   264,   265,
     217,   380,   380,   190,   188,   188,   380,   371,   294,   460,
     302,   303,   379,   299,    14,    30,    48,   304,   307,     9,
      33,   187,    29,    47,    50,    14,     9,   188,   205,   444,
     320,    14,   460,   234,   188,    14,   335,    35,    80,   359,
     217,   217,   189,   312,   190,   450,   390,   217,    95,   230,
     190,   203,   215,   295,   296,   297,     9,   173,     9,   384,
     190,   380,   369,   369,    56,   305,   310,   310,    29,    47,
      50,   380,    80,   172,   186,   188,   380,   445,   380,    80,
       9,   385,   190,   190,   217,   312,    93,   188,   111,   226,
     151,    98,   460,   379,   163,    14,   452,   292,   186,    35,
      80,   187,   190,   188,   186,   169,   233,   203,   315,   316,
     173,   380,   173,   276,   277,   402,   293,    80,   371,   231,
     166,   203,   188,   187,     9,   385,   115,   116,   117,   318,
     319,   276,    80,   261,   188,   450,   402,   461,   187,   187,
     188,   188,   189,   313,   318,    35,    80,   165,   450,   189,
     217,   461,    80,   165,    14,    80,   313,   217,   190,    35,
      80,   165,    14,    80,   335,   190,    80,   165,    14,    80,
     335,    14,    80,   335,   335
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
#line 902 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 989 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1022 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1034 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { (yyval).reset();;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { (yyval).reset();;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1062 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1063 "hphp.y"
    { (yyval).reset();;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1073 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1086 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1092 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1105 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1113 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1121 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1125 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1131 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
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
#line 1149 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1152 "hphp.y"
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
#line 1166 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1169 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1186 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1193 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1201 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1212 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1220 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { (yyval).reset();;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { (yyval).reset();;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { (yyval).reset();;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval).reset();;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1304 "hphp.y"
    { (yyval).reset();;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { (yyval).reset();;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { (yyval).reset();;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { (yyval).reset();;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { (yyval).reset();;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1366 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1375 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1394 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1399 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { (yyval).reset();;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1481 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { (yyval).reset();;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1491 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { (yyval).reset();;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { (yyval).reset();;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { (yyval).reset();;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { (yyval).reset();;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { (yyval).reset();;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { (yyval).reset();;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval).reset();;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval).reset();;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
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
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { (yyval).reset();;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
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

  case 515:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
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

  case 517:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
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

  case 519:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2069 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2072 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
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

  case 554:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
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

  case 555:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval).reset();;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { (yyval).reset();;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
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
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { (yyval).reset();;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval).reset();;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval).reset();;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval).reset();;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { (yyval).reset();;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { (yyval).reset();;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { (yyval).reset();;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval).reset();;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { (yyval).reset();;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { (yyval).reset();;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { (yyval).reset();;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
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
#line 2689 "hphp.y"
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
#line 2704 "hphp.y"
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
#line 2732 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
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

  case 819:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
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
#line 2763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
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

  case 828:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { (yyval).reset();;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { (yyval)++;;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

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

  case 863:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
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
#line 2915 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { (yyval).reset();;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2937 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
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
#line 3002 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3036 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3071 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3092 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3096 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3124 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3142 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    {;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    {;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
    {;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3161 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
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
#line 3177 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3183 "hphp.y"
    { ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3188 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3189 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3195 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3205 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3223 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3226 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3236 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3242 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3248 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3254 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3263 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13652 "hphp.5.tab.cpp"
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
#line 3266 "hphp.y"

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

