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
#line 886 "hphp.5.tab.cpp"

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
#define YYLAST   19173

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  210
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  282
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1023
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1860

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
    2811,  2813,  2815,  2817,  2819,  2821,  2825,  2829,  2834,  2839,
    2843,  2845,  2847,  2855,  2865,  2873,  2880,  2889,  2891,  2894,
    2899,  2904,  2906,  2908,  2913,  2915,  2916,  2918,  2921,  2923,
    2925,  2927,  2931,  2935,  2939,  2940,  2942,  2944,  2948,  2952,
    2955,  2959,  2966,  2967,  2969,  2974,  2977,  2978,  2984,  2988,
    2992,  2994,  3001,  3006,  3011,  3014,  3017,  3018,  3024,  3028,
    3032,  3034,  3037,  3038,  3044,  3048,  3052,  3054,  3057,  3060,
    3062,  3065,  3067,  3072,  3076,  3080,  3087,  3091,  3093,  3095,
    3097,  3102,  3107,  3112,  3117,  3122,  3127,  3130,  3133,  3138,
    3141,  3144,  3146,  3150,  3154,  3158,  3159,  3162,  3168,  3175,
    3182,  3190,  3192,  3195,  3197,  3200,  3202,  3207,  3209,  3214,
    3218,  3219,  3221,  3225,  3228,  3232,  3234,  3236,  3237,  3238,
    3241,  3244,  3247,  3252,  3255,  3261,  3265,  3267,  3269,  3270,
    3274,  3279,  3285,  3289,  3291,  3294,  3295,  3300,  3302,  3306,
    3309,  3312,  3315,  3317,  3319,  3321,  3323,  3327,  3332,  3339,
    3341,  3350,  3357,  3359
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     211,     0,    -1,    -1,   212,   213,    -1,   213,   214,    -1,
      -1,   234,    -1,   251,    -1,   258,    -1,   255,    -1,   263,
      -1,   471,    -1,   125,   200,   201,   202,    -1,   152,   226,
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
     226,    -1,   155,   226,    -1,   227,    -1,   227,   476,    -1,
     227,   476,    -1,   231,     9,   472,    14,   410,    -1,   108,
     472,    14,   410,    -1,   232,   233,    -1,    -1,   234,    -1,
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
     468,   201,   202,    -1,   202,    -1,    82,    -1,    83,    -1,
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
      -1,    35,    -1,    -1,   107,    -1,    -1,   250,   249,   475,
     252,   200,   291,   201,   480,   325,    -1,    -1,   329,   250,
     249,   475,   253,   200,   291,   201,   480,   325,    -1,    -1,
     431,   328,   250,   249,   475,   254,   200,   291,   201,   480,
     325,    -1,    -1,   162,   217,   256,    30,   490,   470,   203,
     298,   204,    -1,    -1,   431,   162,   217,   257,    30,   490,
     470,   203,   298,   204,    -1,    -1,   269,   266,   259,   270,
     271,   203,   301,   204,    -1,    -1,   431,   269,   266,   260,
     270,   271,   203,   301,   204,    -1,    -1,   127,   267,   261,
     272,   203,   301,   204,    -1,    -1,   431,   127,   267,   262,
     272,   203,   301,   204,    -1,    -1,   164,   268,   264,   271,
     203,   301,   204,    -1,    -1,   431,   164,   268,   265,   271,
     203,   301,   204,    -1,   475,    -1,   156,    -1,   475,    -1,
     475,    -1,   126,    -1,   119,   126,    -1,   119,   118,   126,
      -1,   118,   119,   126,    -1,   118,   126,    -1,   128,   403,
      -1,    -1,   129,   273,    -1,    -1,   128,   273,    -1,    -1,
     403,    -1,   273,     9,   403,    -1,   403,    -1,   274,     9,
     403,    -1,   132,   276,    -1,    -1,   443,    -1,    35,   443,
      -1,   133,   200,   457,   201,    -1,   234,    -1,    30,   232,
      93,   202,    -1,   234,    -1,    30,   232,    95,   202,    -1,
     234,    -1,    30,   232,    91,   202,    -1,   234,    -1,    30,
     232,    97,   202,    -1,   217,    14,   410,    -1,   281,     9,
     217,    14,   410,    -1,   203,   283,   204,    -1,   203,   202,
     283,   204,    -1,    30,   283,   101,   202,    -1,    30,   202,
     283,   101,   202,    -1,   283,   102,   348,   284,   232,    -1,
     283,   103,   284,   232,    -1,    -1,    30,    -1,   202,    -1,
     285,    72,   339,   234,    -1,    -1,   286,    72,   339,    30,
     232,    -1,    -1,    73,   234,    -1,    -1,    73,    30,   232,
      -1,    -1,   290,     9,   432,   331,   491,   165,    80,    -1,
     290,     9,   432,   331,   491,    35,   165,    80,    -1,   290,
       9,   432,   331,   491,   165,    -1,   290,   415,    -1,   432,
     331,   491,   165,    80,    -1,   432,   331,   491,    35,   165,
      80,    -1,   432,   331,   491,   165,    -1,    -1,   432,   331,
     491,    80,    -1,   432,   331,   491,    35,    80,    -1,   432,
     331,   491,    35,    80,    14,   348,    -1,   432,   331,   491,
      80,    14,   348,    -1,   290,     9,   432,   331,   491,    80,
      -1,   290,     9,   432,   331,   491,    35,    80,    -1,   290,
       9,   432,   331,   491,    35,    80,    14,   348,    -1,   290,
       9,   432,   331,   491,    80,    14,   348,    -1,   292,     9,
     432,   491,   165,    80,    -1,   292,     9,   432,   491,    35,
     165,    80,    -1,   292,     9,   432,   491,   165,    -1,   292,
     415,    -1,   432,   491,   165,    80,    -1,   432,   491,    35,
     165,    80,    -1,   432,   491,   165,    -1,    -1,   432,   491,
      80,    -1,   432,   491,    35,    80,    -1,   432,   491,    35,
      80,    14,   348,    -1,   432,   491,    80,    14,   348,    -1,
     292,     9,   432,   491,    80,    -1,   292,     9,   432,   491,
      35,    80,    -1,   292,     9,   432,   491,    35,    80,    14,
     348,    -1,   292,     9,   432,   491,    80,    14,   348,    -1,
     294,   415,    -1,    -1,   348,    -1,    35,   443,    -1,   165,
     348,    -1,   294,     9,   348,    -1,   294,     9,   165,   348,
      -1,   294,     9,    35,   443,    -1,   295,     9,   296,    -1,
     296,    -1,    80,    -1,   205,   443,    -1,   205,   203,   348,
     204,    -1,   297,     9,    80,    -1,   297,     9,    80,    14,
     410,    -1,    80,    -1,    80,    14,   410,    -1,   298,   299,
      -1,    -1,   300,   202,    -1,   473,    14,   410,    -1,   301,
     302,    -1,    -1,    -1,   327,   303,   333,   202,    -1,    -1,
     329,   490,   304,   333,   202,    -1,   334,   202,    -1,   335,
     202,    -1,   336,   202,    -1,    -1,   328,   250,   249,   474,
     200,   305,   289,   201,   480,   326,    -1,    -1,   431,   328,
     250,   249,   475,   200,   306,   289,   201,   480,   326,    -1,
     158,   311,   202,    -1,   159,   319,   202,    -1,   161,   321,
     202,    -1,     4,   128,   403,   202,    -1,     4,   129,   403,
     202,    -1,   113,   274,   202,    -1,   113,   274,   203,   307,
     204,    -1,   307,   308,    -1,   307,   309,    -1,    -1,   230,
     151,   217,   166,   274,   202,    -1,   310,    98,   328,   217,
     202,    -1,   310,    98,   329,   202,    -1,   230,   151,   217,
      -1,   217,    -1,   312,    -1,   311,     9,   312,    -1,   313,
     400,   317,   318,    -1,   156,    -1,    29,   314,    -1,   314,
      -1,   134,    -1,   134,   172,   490,   173,    -1,   134,   172,
     490,     9,   490,   173,    -1,   403,    -1,   121,    -1,   162,
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
      80,    -1,    80,    14,   410,    -1,   334,     9,   473,    14,
     410,    -1,   108,   473,    14,   410,    -1,   335,     9,   473,
      -1,   119,   108,   473,    -1,   119,   337,   470,    -1,   337,
     470,    14,   490,    -1,   108,   177,   475,    -1,   200,   338,
     201,    -1,    69,   405,   408,    -1,    68,   348,    -1,   392,
      -1,   368,    -1,   200,   348,   201,    -1,   340,     9,   348,
      -1,   348,    -1,   340,    -1,    -1,    27,    -1,    27,   348,
      -1,    27,   348,   132,   348,    -1,   443,    14,   342,    -1,
     133,   200,   457,   201,    14,   342,    -1,    28,   348,    -1,
     443,    14,   345,    -1,   133,   200,   457,   201,    14,   345,
      -1,   349,    -1,   443,    -1,   338,    -1,   447,    -1,   446,
      -1,   133,   200,   457,   201,    14,   348,    -1,   443,    14,
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
     348,    30,   348,    -1,   348,    29,    30,   348,    -1,   467,
      -1,    63,   348,    -1,    62,   348,    -1,    61,   348,    -1,
      60,   348,    -1,    59,   348,    -1,    58,   348,    -1,    57,
     348,    -1,    70,   406,    -1,    56,   348,    -1,   412,    -1,
     367,    -1,   366,    -1,   206,   407,   206,    -1,    13,   348,
      -1,   370,    -1,   113,   200,   391,   415,   201,    -1,    -1,
      -1,   250,   249,   200,   352,   291,   201,   480,   350,   203,
     232,   204,    -1,    -1,   329,   250,   249,   200,   353,   291,
     201,   480,   350,   203,   232,   204,    -1,    -1,    80,   355,
     360,    -1,    -1,   182,    80,   356,   360,    -1,    -1,   197,
     357,   291,   198,   480,   360,    -1,    -1,   182,   197,   358,
     291,   198,   480,   360,    -1,    -1,   182,   203,   359,   232,
     204,    -1,     8,   348,    -1,     8,   345,    -1,     8,   203,
     232,   204,    -1,    87,    -1,   469,    -1,   362,     9,   361,
     132,   348,    -1,   361,   132,   348,    -1,   363,     9,   361,
     132,   410,    -1,   361,   132,   410,    -1,   362,   414,    -1,
      -1,   363,   414,    -1,    -1,   176,   200,   364,   201,    -1,
     134,   200,   458,   201,    -1,    67,   458,   207,    -1,   403,
     203,   460,   204,    -1,   403,   203,   462,   204,    -1,   370,
      67,   453,   207,    -1,   371,    67,   453,   207,    -1,   367,
      -1,   469,    -1,   446,    -1,    87,    -1,   200,   349,   201,
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
     450,    -1,   403,    -1,   120,    -1,   455,    -1,   200,   201,
      -1,   339,    -1,    -1,    -1,    86,    -1,   464,    -1,   200,
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
     126,    -1,   228,    -1,    79,    -1,   469,    -1,   409,    -1,
     208,   464,   208,    -1,   209,   464,   209,    -1,   147,   464,
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
     204,    -1,   434,    -1,   450,    -1,   217,    -1,   203,   348,
     204,    -1,   436,    -1,   450,    -1,    67,   453,   207,    -1,
     203,   348,   204,    -1,   444,   438,    -1,   200,   338,   201,
     438,    -1,   456,   438,    -1,   200,   338,   201,   438,    -1,
     200,   338,   201,   433,   435,    -1,   200,   349,   201,   433,
     435,    -1,   200,   338,   201,   433,   434,    -1,   200,   349,
     201,   433,   434,    -1,   450,    -1,   402,    -1,   448,    -1,
     449,    -1,   439,    -1,   441,    -1,   443,   433,   435,    -1,
     404,   151,   450,    -1,   445,   200,   293,   201,    -1,   446,
     200,   293,   201,    -1,   200,   443,   201,    -1,   402,    -1,
     448,    -1,   449,    -1,   439,    -1,   443,   433,   434,    -1,
     442,    -1,   445,   200,   293,   201,    -1,   200,   443,   201,
      -1,   450,    -1,   439,    -1,   402,    -1,   367,    -1,   409,
      -1,   200,   443,   201,    -1,   200,   349,   201,    -1,   446,
     200,   293,   201,    -1,   445,   200,   293,   201,    -1,   200,
     447,   201,    -1,   351,    -1,   354,    -1,   443,   433,   437,
     476,   200,   293,   201,    -1,   200,   338,   201,   433,   437,
     476,   200,   293,   201,    -1,   404,   151,   219,   476,   200,
     293,   201,    -1,   404,   151,   450,   200,   293,   201,    -1,
     404,   151,   203,   348,   204,   200,   293,   201,    -1,   451,
      -1,   454,   451,    -1,   451,    67,   453,   207,    -1,   451,
     203,   348,   204,    -1,   452,    -1,    80,    -1,   205,   203,
     348,   204,    -1,   348,    -1,    -1,   205,    -1,   454,   205,
      -1,   450,    -1,   440,    -1,   441,    -1,   455,   433,   435,
      -1,   404,   151,   450,    -1,   200,   443,   201,    -1,    -1,
     440,    -1,   442,    -1,   455,   433,   434,    -1,   200,   443,
     201,    -1,   457,     9,    -1,   457,     9,   443,    -1,   457,
       9,   133,   200,   457,   201,    -1,    -1,   443,    -1,   133,
     200,   457,   201,    -1,   459,   414,    -1,    -1,   459,     9,
     348,   132,   348,    -1,   459,     9,   348,    -1,   348,   132,
     348,    -1,   348,    -1,   459,     9,   348,   132,    35,   443,
      -1,   459,     9,    35,   443,    -1,   348,   132,    35,   443,
      -1,    35,   443,    -1,   461,   414,    -1,    -1,   461,     9,
     348,   132,   348,    -1,   461,     9,   348,    -1,   348,   132,
     348,    -1,   348,    -1,   463,   414,    -1,    -1,   463,     9,
     410,   132,   410,    -1,   463,     9,   410,    -1,   410,   132,
     410,    -1,   410,    -1,   464,   465,    -1,   464,    86,    -1,
     465,    -1,    86,   465,    -1,    80,    -1,    80,    67,   466,
     207,    -1,    80,   433,   217,    -1,   149,   348,   204,    -1,
     149,    79,    67,   348,   207,   204,    -1,   150,   443,   204,
      -1,   217,    -1,    81,    -1,    80,    -1,   123,   200,   340,
     201,    -1,   124,   200,   443,   201,    -1,   124,   200,   349,
     201,    -1,   124,   200,   447,   201,    -1,   124,   200,   446,
     201,    -1,   124,   200,   338,   201,    -1,     7,   348,    -1,
       6,   348,    -1,     5,   200,   348,   201,    -1,     4,   348,
      -1,     3,   348,    -1,   443,    -1,   468,     9,   443,    -1,
     404,   151,   218,    -1,   404,   151,   126,    -1,    -1,    98,
     490,    -1,   177,   475,    14,   490,   202,    -1,   431,   177,
     475,    14,   490,   202,    -1,   179,   475,   470,    14,   490,
     202,    -1,   431,   179,   475,   470,    14,   490,   202,    -1,
     219,    -1,   490,   219,    -1,   218,    -1,   490,   218,    -1,
     219,    -1,   219,   172,   482,   173,    -1,   217,    -1,   217,
     172,   482,   173,    -1,   172,   478,   173,    -1,    -1,   490,
      -1,   477,     9,   490,    -1,   477,   414,    -1,   477,     9,
     165,    -1,   478,    -1,   165,    -1,    -1,    -1,    30,   490,
      -1,    98,   490,    -1,    99,   490,    -1,   482,     9,   483,
     217,    -1,   483,   217,    -1,   482,     9,   483,   217,   481,
      -1,   483,   217,   481,    -1,    47,    -1,    48,    -1,    -1,
      87,   132,   490,    -1,    29,    87,   132,   490,    -1,   230,
     151,   217,   132,   490,    -1,   485,     9,   484,    -1,   484,
      -1,   485,   414,    -1,    -1,   176,   200,   486,   201,    -1,
     230,    -1,   217,   151,   489,    -1,   217,   476,    -1,    29,
     490,    -1,    56,   490,    -1,   230,    -1,   134,    -1,   135,
      -1,   487,    -1,   488,   151,   489,    -1,   134,   172,   490,
     173,    -1,   134,   172,   490,     9,   490,   173,    -1,   156,
      -1,   200,   107,   200,   479,   201,    30,   490,   201,    -1,
     200,   490,     9,   477,   414,   201,    -1,   490,    -1,    -1
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
    2725,  2726,  2730,  2731,  2732,  2736,  2738,  2746,  2747,  2751,
    2753,  2761,  2762,  2766,  2767,  2772,  2774,  2779,  2790,  2804,
    2816,  2831,  2832,  2833,  2834,  2835,  2836,  2837,  2847,  2856,
    2858,  2860,  2864,  2865,  2866,  2867,  2868,  2884,  2885,  2887,
    2896,  2897,  2898,  2899,  2900,  2901,  2902,  2903,  2905,  2910,
    2914,  2915,  2919,  2922,  2929,  2933,  2942,  2949,  2951,  2957,
    2959,  2960,  2964,  2965,  2972,  2973,  2978,  2979,  2984,  2985,
    2986,  2987,  2998,  3001,  3004,  3005,  3006,  3007,  3018,  3022,
    3023,  3024,  3026,  3027,  3028,  3032,  3034,  3037,  3039,  3040,
    3041,  3042,  3045,  3047,  3048,  3052,  3054,  3057,  3059,  3060,
    3061,  3065,  3067,  3070,  3073,  3075,  3077,  3081,  3082,  3084,
    3085,  3091,  3092,  3094,  3104,  3106,  3108,  3111,  3112,  3113,
    3117,  3118,  3119,  3120,  3121,  3122,  3123,  3124,  3125,  3126,
    3127,  3131,  3132,  3136,  3138,  3146,  3148,  3152,  3156,  3161,
    3165,  3173,  3174,  3178,  3179,  3185,  3186,  3195,  3196,  3204,
    3207,  3211,  3214,  3219,  3224,  3226,  3227,  3228,  3232,  3233,
    3237,  3238,  3241,  3244,  3246,  3250,  3256,  3257,  3258,  3262,
    3266,  3276,  3284,  3286,  3290,  3292,  3297,  3303,  3306,  3311,
    3319,  3322,  3325,  3326,  3329,  3332,  3333,  3338,  3341,  3345,
    3349,  3355,  3365,  3366
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
     445,   445,   445,   445,   445,   445,   445,   445,   445,   446,
     447,   447,   448,   448,   449,   449,   449,   450,   450,   451,
     451,   451,   452,   452,   453,   453,   454,   454,   455,   455,
     455,   455,   455,   455,   456,   456,   456,   456,   456,   457,
     457,   457,   457,   457,   457,   458,   458,   459,   459,   459,
     459,   459,   459,   459,   459,   460,   460,   461,   461,   461,
     461,   462,   462,   463,   463,   463,   463,   464,   464,   464,
     464,   465,   465,   465,   465,   465,   465,   466,   466,   466,
     467,   467,   467,   467,   467,   467,   467,   467,   467,   467,
     467,   468,   468,   469,   469,   470,   470,   471,   471,   471,
     471,   472,   472,   473,   473,   474,   474,   475,   475,   476,
     476,   477,   477,   478,   479,   479,   479,   479,   480,   480,
     481,   481,   482,   482,   482,   482,   483,   483,   483,   484,
     484,   484,   485,   485,   486,   486,   487,   488,   489,   489,
     490,   490,   490,   490,   490,   490,   490,   490,   490,   490,
     490,   490,   491,   491
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
       0,   435,     0,   825,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   916,     0,
     904,   707,     0,   713,   714,   715,    22,   772,   892,   163,
     164,   716,     0,   144,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   197,     0,     0,     0,     0,     0,     0,
     402,   403,   404,   407,   406,   405,     0,     0,     0,     0,
     224,     0,     0,     0,   720,   722,   723,   717,   718,     0,
       0,     0,   724,   719,     0,   695,    24,    25,    26,    28,
      27,     0,   721,     0,     0,     0,     0,   725,   408,    29,
      30,    32,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,   528,     0,   162,   134,   896,   708,     0,     0,
       4,   123,   125,   128,   771,     0,   694,     0,     6,   196,
       7,     9,     8,    10,     0,     0,   400,   445,     0,     0,
       0,     0,     0,     0,     0,   443,   880,   881,   514,   513,
     429,   517,     0,     0,   428,   852,   696,     0,   774,   512,
     399,   855,   856,   867,   444,     0,     0,   447,   446,   853,
     854,   851,   887,   891,     0,   502,   773,    11,   407,   406,
     405,     0,     0,    28,   123,   196,     0,   960,   444,   959,
       0,   957,   956,   516,     0,   436,   440,     0,     0,   485,
     486,   487,   488,   511,   509,   508,   507,   506,   505,   504,
     503,   892,   716,   698,     0,     0,   980,   873,   696,     0,
     697,   467,     0,   465,     0,   920,     0,   781,   427,   703,
       0,   980,   702,     0,   712,   697,   899,   900,   906,   898,
     704,     0,     0,   706,   510,     0,     0,     0,     0,   432,
       0,   142,   434,     0,     0,   148,   150,     0,     0,   152,
       0,    82,    81,    76,    75,    67,    68,    59,    79,    90,
       0,    62,     0,    74,    66,    72,    92,    85,    84,    57,
      80,    99,   100,    58,    95,    55,    96,    56,    97,    54,
     101,    89,    93,    98,    86,    87,    61,    88,    91,    53,
      83,    69,   102,    77,    70,    60,    52,    51,    50,    49,
      48,    47,    71,   104,    64,    45,    46,    73,  1013,  1014,
      65,  1019,    44,    63,    94,     0,     0,   123,   103,   971,
    1012,     0,  1015,     0,     0,     0,   154,     0,     0,     0,
       0,   187,     0,     0,     0,     0,     0,     0,   106,   111,
     313,     0,     0,   312,     0,   228,     0,   225,   318,     0,
       0,     0,     0,     0,   977,   212,   222,   912,   916,     0,
     941,     0,   727,     0,     0,     0,   939,     0,    16,     0,
     127,   204,   216,   223,   601,   544,     0,   965,   526,   530,
     532,   829,   445,     0,   443,   444,   446,     0,     0,   709,
       0,   710,     0,     0,     0,   186,     0,     0,   130,   304,
       0,    21,   195,     0,   221,   208,   220,   405,   408,   196,
     401,   177,   178,   179,   180,   181,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   904,     0,   176,   895,   895,   184,   926,
       0,     0,     0,     0,     0,     0,     0,     0,   398,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   466,   464,   830,   831,     0,   895,     0,   843,
     304,   304,   895,     0,   897,   888,   912,     0,   196,     0,
       0,   156,     0,   827,   822,   781,     0,   445,   443,     0,
     924,     0,   549,   780,   915,   445,   443,   444,   130,     0,
     304,   426,     0,   845,   705,     0,   134,   264,     0,   525,
       0,   159,     0,     0,   433,     0,     0,     0,     0,     0,
     151,   175,   153,  1013,  1014,  1010,  1011,     0,  1005,     0,
       0,     0,     0,    78,    43,    65,    42,   972,     0,   182,
     155,   185,     0,     0,     0,     0,     0,     0,     0,   559,
       0,   566,   568,   569,   570,   571,   572,   573,   564,   586,
     587,   134,     0,   172,   174,     0,     0,   108,   115,     0,
       0,   110,   119,   112,     0,    18,     0,     0,   314,     0,
     157,   227,   226,     0,     0,   158,   961,     0,     0,   445,
     443,   444,   447,   446,     0,   998,   234,     0,   913,     0,
       0,   160,     0,     0,   726,   940,   772,     0,     0,   938,
     777,   937,   126,     5,    13,    14,     0,   232,     0,     0,
     537,     0,     0,     0,   781,     0,     0,   699,   538,     0,
       0,     0,     0,   829,   134,     0,   783,   828,  1023,   425,
     499,   861,   879,   139,   133,   135,   136,   137,   138,   399,
       0,   515,   775,   776,   124,   781,     0,   981,     0,     0,
       0,   783,   305,     0,   520,   198,   230,     0,   470,   472,
     471,     0,     0,   468,   469,   473,   475,   474,   490,   489,
     492,   491,   493,   495,   497,   496,   494,   484,   483,   477,
     478,   476,   479,   480,   482,   498,   481,   894,     0,     0,
     930,     0,   781,   964,     0,   963,   980,   858,   887,   214,
     206,   218,     0,   965,   210,   196,   435,     0,   438,   441,
     449,   560,   463,   462,   461,   460,   459,   458,   457,   456,
     455,   454,   453,   452,   833,     0,   832,   835,   857,   839,
     980,   836,     0,     0,     0,     0,     0,     0,     0,     0,
     958,   437,   820,   824,   780,   826,     0,   700,     0,   919,
       0,   918,     0,   700,   903,   902,     0,     0,   832,   835,
     901,   836,   430,   266,   268,   134,   535,   534,   431,     0,
     134,   248,   143,   434,     0,     0,     0,     0,     0,   260,
     260,   149,     0,     0,     0,     0,  1003,   781,     0,   987,
       0,     0,     0,     0,     0,   779,     0,   695,     0,     0,
     128,   729,   694,   734,     0,   728,   132,   733,   980,  1016,
       0,     0,   576,     0,     0,   582,   579,   580,   588,     0,
     567,   562,     0,   565,     0,     0,     0,   116,    19,     0,
       0,   120,    20,     0,     0,     0,   105,   113,     0,   311,
     319,   316,     0,     0,   950,   955,   952,   951,   954,   953,
      12,   996,   997,     0,     0,     0,     0,   912,   909,     0,
     548,   949,   948,   947,     0,   943,     0,   944,   946,     0,
       5,     0,     0,     0,   595,   596,   604,   603,     0,   443,
       0,   780,   543,   547,     0,     0,   966,     0,   527,     0,
       0,   988,   829,   290,  1022,     0,     0,   844,     0,   893,
     780,   983,   979,   306,   307,   693,   782,   303,     0,   829,
       0,     0,   232,   522,   200,   501,     0,   552,   553,     0,
     550,   780,   925,     0,     0,   304,   234,     0,   232,     0,
       0,   230,     0,   904,   450,     0,     0,   841,   842,   859,
     860,   889,   890,     0,     0,     0,   808,   788,   789,   790,
     797,     0,     0,     0,   801,   799,   800,   814,   781,     0,
     822,   923,   922,     0,     0,   846,   711,     0,   270,     0,
       0,   140,     0,     0,     0,     0,     0,     0,     0,   240,
     241,   252,     0,   134,   250,   169,   260,     0,   260,     0,
       0,  1017,     0,     0,     0,   780,  1004,  1006,   986,   781,
     985,     0,   781,   755,   756,   753,   754,   787,     0,   781,
     779,     0,   546,     0,     0,   932,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1009,   561,     0,     0,     0,   584,
     585,   583,     0,     0,   563,     0,   188,     0,   191,   173,
       0,   107,   117,     0,   109,   121,   114,   315,     0,   962,
     161,   998,   978,   993,   233,   235,   325,     0,     0,   910,
       0,   942,     0,    17,     0,   965,   231,   325,     0,     0,
     700,   540,     0,   964,   963,   701,   967,     0,   988,   533,
       0,     0,  1023,     0,   295,   293,   835,   847,   980,   835,
     848,   982,     0,     0,   308,   131,     0,   829,   229,     0,
     829,     0,   500,   929,   928,     0,   304,     0,     0,     0,
       0,     0,     0,   232,   202,   712,   834,   304,     0,   793,
     794,   795,   796,   802,   803,   812,     0,   781,     0,   808,
       0,   792,   816,   780,   819,   821,   823,     0,   917,   834,
       0,     0,     0,     0,   267,   536,   145,     0,   434,   240,
     242,   912,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   254,     0,     0,   999,     0,  1002,   780,     0,     0,
       0,   731,   780,   778,     0,   769,     0,   781,     0,   735,
     770,   768,   936,     0,   781,   738,   740,   739,     0,     0,
     736,   737,   741,   743,   742,   758,   757,   760,   759,   761,
     763,   765,   764,   762,   751,   750,   745,   746,   744,   747,
     748,   749,   752,  1008,   574,     0,   575,   581,   589,   590,
       0,   134,   118,   122,   317,     0,     0,     0,   995,     0,
     399,   914,   912,   439,   442,   448,     0,    15,     0,   399,
     607,     0,     0,   609,   602,   605,     0,   600,     0,   969,
       0,   989,   529,     0,   296,     0,     0,   291,     0,   310,
     309,   988,     0,   325,     0,   829,     0,   304,     0,   885,
     325,   965,   325,   968,     0,     0,     0,   451,     0,     0,
     805,   780,   807,   798,     0,   791,     0,     0,   781,   813,
     921,     0,   134,     0,   263,   249,     0,     0,     0,   239,
     165,   253,     0,     0,   256,     0,   261,   262,   134,   255,
    1018,  1000,     0,   984,     0,  1021,   786,   785,   730,     0,
     780,   545,   732,     0,   551,   780,   931,   767,     0,     0,
       0,     0,   992,   990,   991,   236,     0,     0,     0,   406,
     397,     0,     0,     0,   213,   324,   326,     0,   396,     0,
       0,     0,   965,   399,     0,   945,   321,   217,   598,     0,
       0,   539,   531,     0,   299,   289,     0,   292,   298,   304,
     519,   988,   399,   988,     0,   927,     0,   884,   399,     0,
     399,   970,   325,   829,   882,   811,   810,   804,     0,   806,
     780,   815,   134,   269,   141,   146,   167,   243,     0,   251,
     257,   134,   259,  1001,     0,     0,   542,     0,   935,   934,
     766,     0,   134,   192,   994,     0,     0,     0,   973,     0,
       0,     0,   237,     0,   965,     0,   362,   358,   364,   695,
      28,     0,   352,     0,   357,   361,   374,     0,   372,   377,
       0,   376,     0,   375,     0,   196,   328,     0,   330,     0,
     331,   332,     0,     0,   911,     0,   599,   597,   608,   606,
     300,     0,     0,   287,   297,     0,     0,     0,     0,   209,
     519,   988,   886,   215,   321,   219,   399,     0,     0,   818,
       0,   265,     0,     0,   134,   246,   166,   258,  1020,   784,
       0,     0,     0,     0,     0,     0,   424,     0,   974,     0,
     342,   346,   421,   422,   356,     0,     0,     0,   337,   659,
     658,   655,   657,   656,   676,   678,   677,   647,   618,   619,
     637,   653,   652,   614,   624,   625,   627,   626,   646,   630,
     628,   629,   631,   632,   633,   634,   635,   636,   638,   639,
     640,   641,   642,   643,   645,   644,   615,   616,   617,   620,
     621,   623,   661,   662,   671,   670,   669,   668,   667,   666,
     654,   673,   663,   664,   665,   648,   649,   650,   651,   674,
     675,   679,   681,   680,   682,   683,   660,   685,   684,   687,
     689,   688,   622,   692,   690,   691,   686,   672,   613,   369,
     610,     0,   338,   390,   391,   389,   382,     0,   383,   339,
     416,     0,     0,     0,     0,   420,     0,   196,   205,   320,
       0,     0,     0,   288,   302,   883,     0,   134,   392,   134,
     199,     0,     0,     0,   211,   988,   809,     0,   134,   244,
     147,   168,     0,   541,   933,   577,   190,   340,   341,   419,
     238,     0,     0,   781,     0,   365,   353,     0,     0,     0,
     371,   373,     0,     0,   378,   385,   386,   384,     0,     0,
     327,   975,     0,     0,     0,   423,     0,   322,     0,   301,
       0,   593,   783,     0,     0,   134,   201,   207,     0,   817,
       0,     0,     0,   170,   343,   123,     0,   344,   345,     0,
       0,   359,   780,   367,   363,   368,   611,   612,     0,   354,
     387,   388,   380,   381,   379,   417,   414,   998,   333,   329,
     418,     0,   323,   594,   782,     0,   521,   393,     0,   203,
       0,   247,   578,     0,   194,     0,   399,     0,   366,   370,
       0,     0,   829,   335,     0,   591,   518,   523,   245,     0,
       0,   171,   350,     0,   398,   360,   415,   976,     0,   783,
     410,   829,   592,     0,   193,     0,     0,   349,   988,   829,
     274,   411,   412,   413,  1023,   409,     0,     0,     0,   348,
       0,   410,     0,   988,     0,   347,   394,   134,   334,  1023,
       0,   279,   277,     0,   134,     0,     0,   280,     0,     0,
     275,   336,     0,   395,     0,   283,   273,     0,   276,   282,
     189,   284,     0,     0,   271,   281,     0,   272,   286,   285
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   900,   633,   184,  1468,   329,
     347,   586,   590,   348,   587,   591,   122,   123,   124,   125,
     126,   127,   397,   664,   665,   533,   250,  1532,   539,  1448,
    1533,  1774,   856,   342,   581,  1733,  1089,  1271,  1791,   413,
     185,   666,   940,  1151,  1326,   131,   636,   957,   667,   686,
     961,   616,   956,   668,   637,   958,   415,   365,   382,   134,
     942,   903,   886,  1104,  1471,  1203,  1009,  1680,  1536,   802,
    1015,   538,   811,  1017,  1358,   794,   998,  1001,  1192,  1798,
    1799,   655,   656,   680,   681,   352,   353,   359,  1505,  1659,
    1660,  1280,  1395,  1494,  1653,  1782,  1801,  1691,  1737,  1738,
    1739,  1481,  1482,  1483,  1484,  1693,  1694,  1700,  1749,  1487,
    1488,  1492,  1646,  1647,  1648,  1670,  1828,  1396,  1397,   186,
     136,  1814,  1815,  1651,  1399,  1400,  1401,  1402,   137,   243,
     534,   535,   138,   139,   140,   141,   142,   143,   144,   145,
    1517,   146,   939,  1150,   147,   247,   652,   391,   653,   654,
     529,   643,   644,  1227,   645,  1228,   148,   149,   150,   833,
     151,   152,   339,   153,   340,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   846,   847,  1081,   578,   579,   580,
     853,  1722,   154,   638,  1507,   639,  1118,   908,  1297,  1294,
    1639,  1640,   155,   156,   157,   234,   244,   400,   521,   158,
    1037,   837,   159,  1038,   931,   923,  1039,   985,  1173,   986,
    1175,  1176,  1177,   988,  1337,  1338,   989,   773,   505,   197,
     198,   669,   658,   486,  1136,  1137,   759,   760,   927,   161,
     236,   162,   163,   188,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   718,   174,   240,   241,   619,   226,   227,
     721,   722,  1233,  1234,   375,   376,   894,   175,   607,   176,
     651,   177,   331,  1661,  1712,   366,   408,   675,   676,  1031,
    1131,  1278,   883,   884,   816,   817,   818,   332,   333,   839,
    1470,   925
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1525
static const yytype_int16 yypact[] =
{
   -1525,   170, -1525, -1525,  5441, 13514, 13514,    -6, 13514, 13514,
   13514, 11444, 13514, -1525, 13514, 13514, 13514, 13514, 13514, 13514,
   13514, 13514, 13514, 13514, 13514, 13514, 17673, 17673, 11651, 13514,
   17799,     3,    20, -1525, -1525, -1525, -1525, -1525,   219, -1525,
   -1525,   196, 13514, -1525,    20,   184,   207,   220, -1525,    20,
   11858, 14656, 12065, -1525, 14604, 10409,   134, 13514, 18453,   161,
   -1525, -1525, -1525,    70,   476,    60,   260,   278,   293,   298,
   -1525, 14656,   324,   340, -1525, -1525, -1525, -1525, -1525, 13514,
     551,  2322, -1525, -1525, 14656, -1525, -1525, -1525, -1525, 14656,
   -1525, 14656, -1525,   328,   344, 14656, 14656, -1525,   209, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, 13514, -1525, -1525,   350,   465,   634,   634,
   -1525,   526,   405,   435, -1525,   401, -1525,    80, -1525,   573,
   -1525, -1525, -1525, -1525, 18514,   560, -1525, -1525,   415,   420,
     422,   424,   437,   446, 15869, -1525, -1525, -1525, -1525,    57,
   -1525,   563,   589,   464, -1525,    55,   458,   512,   191, -1525,
    2154,   138, -1525, -1525,  1602,   133,   468,    79, -1525,   141,
     146,   471,   152, -1525,   162, -1525,   618, -1525, -1525, -1525,
     522,   487,   570, -1525, -1525,   573,   560, 18877,  2533, 18877,
   13514, 18877, 18877, 14993,   504, 17087, 14993,   687, 14656,   679,
     679,   347,   679,   679,   679,   679,   679,   679,   679,   679,
     679, -1525, -1525, -1525,    61, 13514,   575, -1525, -1525,   610,
     568,   390,   577,   390, 17673, 18112,   556,   761, -1525,   522,
   13514,   575,   628,   629,   581, -1525,   148, -1525, -1525, -1525,
     390,   133, 12272, -1525, -1525, 13514,  8960,   777,    82, 18877,
    9995, -1525, 13514, 13514, 14656, -1525, -1525, 15914,   586, -1525,
   15959, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   16440, -1525, 16440, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,    78,    76,
     570, -1525, -1525, -1525, -1525,   591,  3224,    96, -1525, -1525,
     644,   782, -1525,   654, 15415,   717, -1525,   605, 16004,   614,
     646, -1525,    26, 16049, 18557, 18572, 14656,    84, -1525,   255,
   -1525, 17148,    86, -1525,   688, -1525,   694, -1525,   805,    98,
   17673, 13514, 13514,   622,   652, -1525, -1525, 17279, 11651,   100,
      68,   341, -1525, 13721, 17673,   578, -1525, 14656, -1525,   364,
     405, -1525, -1525, -1525, -1525, 17925,   811,   729, -1525, -1525,
   -1525,    66,   633, 18877,   638,  1224,   641,  5648, 13514,   431,
     627,   637,   431,   461,   493, -1525, 14656, 16440,   647, 10616,
   14604, -1525, -1525, 14266, -1525, -1525, -1525, -1525, -1525,   573,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, 13514, 13514, 13514,
   12479, 13514, 13514, 13514, 13514, 13514, 13514, 13514, 13514, 13514,
   13514, 13514, 13514, 13514, 13514, 13514, 13514, 13514, 13514, 13514,
   13514, 13514, 13514, 17799, 13514, -1525, 13514, 13514, -1525, 13514,
    4600, 14656, 14656, 14656, 14656, 14656, 18514,   737,   693, 10202,
   13514, 13514, 13514, 13514, 13514, 13514, 13514, 13514, 13514, 13514,
   13514, 13514, -1525, -1525, -1525, -1525,  4120, 13514, 13514, -1525,
   10616, 10616, 13514, 13514,   350,   149, 17279,   651,   573, 12686,
   16094, -1525, 13514, -1525,   655,   837,   706,   656,   659, 13928,
     390, 12893, -1525, 13100, -1525,   660,   661,  1750, -1525,   173,
   10616, -1525, 16513, -1525, -1525, 16139, -1525, -1525, 10823, -1525,
   13514, -1525,   773,  9167,   855,   664, 18787,   853,    72,    89,
   -1525, -1525, -1525,   696, -1525, -1525, -1525, 16440,  2342,   671,
     864, 16888, 14656, -1525, -1525, -1525, -1525, -1525,   690, -1525,
   -1525, -1525,   803, 13514,   804,   806, 13514, 13514, 13514, -1525,
     646, -1525, -1525, -1525, -1525, -1525, -1525, -1525,   699, -1525,
   -1525, -1525,   685, -1525, -1525, 14656,   691,   880,   394, 14656,
     692,   881,   404,   428, 18618, -1525, 14656, 13514,   390,   161,
   -1525, -1525, -1525, 16888,   812, -1525,   390,   109,   118,   698,
     700,  2339,    65,   702,   695,   598,   767,   704,   390,   119,
     707, -1525,  4471, 14656, -1525, -1525,   833,  3624,    41, -1525,
   -1525, -1525,   405, -1525, -1525, -1525,   877,   788,   747,   234,
     775, 13514,   350,   800,   924,   733,   787,   149, -1525, 16440,
   16440,   926,   777,    66, -1525,   744,   938, -1525, 16440,    87,
     440,   140, -1525, -1525, -1525, -1525, -1525, -1525, -1525,   640,
    3807, -1525, -1525, -1525, -1525,   939,   776, -1525, 17673, 13514,
     753,   949, 18877,   945, -1525, -1525,   832, 14461, 12050, 13925,
   14993, 13514, 18832, 19072, 17478,  2690, 10595,  3731, 11629, 11629,
   11629, 11629,  2603,  2603,  2603,  2603,  2603,   127,   127,   749,
     749,   749,   347,   347,   347, -1525,   679, 18877,   758,   759,
   18157,   763,   962,   -20, 13514,   -16,   575,   195,   149, -1525,
   -1525, -1525,   958,   729, -1525,   573, 13514, 17411, -1525, -1525,
   14993, -1525, 14993, 14993, 14993, 14993, 14993, 14993, 14993, 14993,
   14993, 14993, 14993, 14993, -1525, 13514,   -12,   155, -1525, -1525,
     575,    -9,   766,  3948,   774,   778,   770,  4145,   120,   780,
   -1525, 18877, 17018, -1525, 14656, -1525,    87,    14, 17673, 18877,
   17673, 18214,    87,   390,   168,   823,   784, 13514, -1525,   169,
   -1525, -1525, -1525,  8753,   635, -1525, -1525, 18877, 18877,    20,
   -1525, -1525, -1525, 13514,   895, 16720, 16888, 14656,  9374,   792,
     793, -1525,    94,   891,   865,   845, -1525,   989,   801, 16299,
   16440, 16888, 16888, 16888, 16888, 16888,   807,   850,   808, 16888,
      51, -1525,   852, -1525,   809, -1525, 18965, -1525,   230, -1525,
   13514,   820, 18877,   829,   995, 11429,  1008, -1525, 18877, 16227,
   -1525,   699,   940, -1525,  5855, 18499,   815,   434, -1525, 18557,
   14656,   441, -1525, 18572, 14656, 14656, -1525, -1525,  4475, -1525,
   18965,  1007, 17673,   822, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525,   142, 14656, 18499,   819, 17279, 17542,  1012,
   -1525, -1525, -1525, -1525,   821, -1525, 13514, -1525, -1525,  5027,
   -1525, 16440, 18499,   824, -1525, -1525, -1525, -1525,  1015,   831,
   13514, 17925, -1525, -1525, 14071,   836, -1525, 16440, -1525,   841,
    6062,  1004,    58, -1525, -1525,   189,  4120, -1525, 16513, -1525,
   16440, -1525, -1525,   390, 18877, -1525, 11030, -1525, 16888,    42,
     842, 18499,   788, -1525, -1525, 19001, 13514, -1525, -1525, 13514,
   -1525, 13514, -1525,  4787,   843, 10616,   767,  1011,   788, 16440,
    1030,   832, 14656, 17799,   390,  4832,   848, -1525, -1525,   145,
     849, -1525, -1525,  1031,  3006,  3006, 17018, -1525, -1525, -1525,
    1002,   854,   237,   858, -1525, -1525, -1525, -1525,  1043,   859,
     655,   390,   390, 13307, 16513, -1525, -1525,  4921,   649,    20,
    9995, -1525,  6269,   851,  6476,   857, 16720, 17673,   861,   931,
     390, 18965,  1050, -1525, -1525, -1525, -1525,    64, -1525,    40,
   16440, -1525,   933, 16440, 14656,  2342, -1525, -1525, -1525,  1057,
   -1525,   866,   939,   632,   632,  1003,  1003, 18316,   867,  1059,
   16888, 15701, 17925, 16184, 15558, 16888, 16888, 16888, 16888, 16590,
   16888, 16888, 16888, 16888, 16888, 16888, 16888, 16888, 16888, 16888,
   16888, 16888, 16888, 16888, 16888, 16888, 16888, 16888, 16888, 16888,
   16888, 16888, 16888, 14656, -1525, 18877, 13514, 13514, 13514, -1525,
   -1525, -1525, 13514, 13514, -1525,   646, -1525,   990, -1525, -1525,
   14656, -1525, -1525, 14656, -1525, -1525, -1525, -1525, 16888,   390,
   -1525,   598, -1525,   631,  1066, -1525, -1525,   121,   879,   390,
   11237, -1525,  3473, -1525,  5234,   729,  1066, -1525,   417,   208,
   -1525, 18877,   948, -1525, -1525, -1525, -1525,   883,  1004, -1525,
   16440,   777, 16440,    81,  1069,  1009,   171, -1525,   575,   172,
   -1525, -1525, 17673, 13514, 18877, 18965,   887,    42, -1525,   893,
      42,   897, 19001, 18877, 18259,   900, 10616,   901,   898, 16440,
     903,   902, 16440,   788, -1525,   581,    -8, 10616, 13514, -1525,
   -1525, -1525, -1525, -1525, -1525,   975,   904,  1100,  1021, 17018,
     964, -1525, 17925, 17018, -1525, -1525, -1525, 17673, 18877, -1525,
      20,  1080,  1040,  9995, -1525, -1525, -1525,   913, 13514,   931,
     390, 17279, 16720,   928, 16888,  6683,   286,   934, 13514,    85,
      47, -1525,   957, 16440, -1525,  1001, -1525, 16372,  1104,   936,
   16888, -1525, 16888, -1525,   942, -1525,  1016,  1131,   946, -1525,
   -1525, -1525, 18361,   950,  1140, 12257, 15412, 15222, 16888, 18922,
   19107, 17740,  2843, 11009,  3199, 12664, 12664, 12664, 12664,  1320,
    1320,  1320,  1320,  1320,   863,   863,   632,   632,   632,  1003,
    1003,  1003,  1003, -1525, 18877, 13706, 18877, -1525, 18877, -1525,
     954, -1525, -1525, -1525, 18965, 14656, 16440, 16440, -1525, 18499,
     113, -1525, 17279, -1525, -1525, 14993,   952, -1525,   955,  1099,
   -1525,   106, 13514, -1525, -1525, -1525, 13514, -1525, 13514, -1525,
     777, -1525, -1525,   334,  1143,  1081, 13514, -1525,   960,   390,
   18877,  1004,   965, -1525,   968,    42, 13514, 10616,   970, -1525,
   -1525,   729, -1525, -1525,   963,   969,   973, -1525,   976, 17018,
   -1525, 17018, -1525, -1525,   977, -1525,  1049,   981,  1174, -1525,
     390,  1154, -1525,   983, -1525, -1525,   985,   986,   124, -1525,
   -1525, 18965,   987,   988, -1525,  4548, -1525, -1525, -1525, -1525,
   -1525, -1525, 16440, -1525, 16440, -1525, 18965, 18418, -1525, 16888,
   17925, -1525, -1525, 16888, -1525, 16888, -1525, 19037, 16888, 13514,
     991,  6890,   631, -1525, -1525, -1525,   606, 14799, 18499,  1083,
   -1525, 16818,  1028,   932, -1525, -1525, -1525,   737, 16253,   102,
     104,   994,   729,   693,   130, -1525, -1525, -1525,  1036,  4969,
   15164, 18877, -1525,   296,  1184,  1119, 13514, -1525, 18877, 10616,
    1087,  1004,  1639,  1004,  1000, 18877,  1005, -1525,  1961,   999,
    2079, -1525, -1525,    42, -1525, -1525,  1072, -1525, 17018, -1525,
   17925, -1525, -1525,  8753, -1525, -1525, -1525, -1525,  9581, -1525,
   -1525, -1525,  8753, -1525,  1010, 16888, 18965,  1073, 18965, 18463,
   19037, 13499, -1525, -1525, -1525, 18499, 18499, 14656, -1525,  1194,
   15701,    74, -1525, 14799,   729, 18438, -1525,  1037, -1525,   105,
    1018,   107, -1525, 15221, -1525, -1525, -1525,   111, -1525, -1525,
   17071, -1525,  1020, -1525,  1130,   573, -1525, 14994, -1525, 14994,
   -1525, -1525,  1199,   737, -1525, 14214, -1525, -1525, -1525, -1525,
    1209,  1144, 13514, -1525, 18877,  1024,  1027,  1025,   537, -1525,
    1087,  1004, -1525, -1525, -1525, -1525,  2147,  1029, 17018, -1525,
    1097,  8753,  9788,  9581, -1525, -1525, -1525,  8753, -1525, 18965,
   16888, 16888, 13514,  7097,  1033,  1034, -1525, 16888, -1525, 18499,
   -1525, -1525, -1525, -1525, -1525, 16440,   557, 16818, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,   139,
   -1525,  1028, -1525, -1525, -1525, -1525, -1525,    93,   535, -1525,
    1218,   112, 15415,  1130,  1223, -1525, 16440,   573, -1525, -1525,
    1051,  1240, 13514, -1525, 18877, -1525,   466, -1525, -1525, -1525,
   -1525,  1056,   537, 14409, -1525,  1004, -1525, 17018, -1525, -1525,
   -1525, -1525,  7304, 18965, 18965, 11843, -1525, -1525, -1525, 18965,
   -1525, 17738,   153,  1252,  1058, -1525, -1525, 16888, 15221, 15221,
    1207, -1525, 17071, 17071,   612, -1525, -1525, -1525, 16888,  1188,
   -1525,  1092,  1070,   114, 16888, -1525, 14656, -1525, 16888, 18877,
    1189, -1525,  1264,  7511,  7718, -1525, -1525, -1525,   537, -1525,
    7925,  1074,  1195,  1163, -1525,  1179,  1127, -1525, -1525,  1181,
   16440, -1525,   557, -1525, -1525, 18965, -1525, -1525,  1120, -1525,
    1249, -1525, -1525, -1525, -1525, 18965,  1270,   598, -1525, -1525,
   18965,  1086, 18965, -1525,   492,  1089, -1525, -1525,  8132, -1525,
    1090, -1525, -1525,  1094,  1126, 14656,   693,  1123, -1525, -1525,
   16888,   159,    90, -1525,  1219, -1525, -1525, -1525, -1525, 18499,
     815, -1525,  1132, 14656,   534, -1525, 18965, -1525,  1101,  1291,
     301,    90, -1525,  1225, -1525, 18499,  1106, -1525,  1004,    92,
   -1525, -1525, -1525, -1525, 16440, -1525,  1108,  1109,   116, -1525,
     541,   301,   335,  1004,  1112, -1525, -1525, -1525, -1525, 16440,
     314,  1302,  1237,   541, -1525,  8339,   368,  1304,  1241, 13514,
   -1525, -1525,  8546, -1525,   316,  1306,  1242, 13514, -1525, 18877,
   -1525,  1311,  1247, 13514, -1525, 18877, 13514, -1525, 18877, 18877
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1525, -1525, -1525,  -566, -1525, -1525, -1525,   459,    43,  -326,
   -1525, -1525, -1525,   735,   472,   467,    15,  1561,  3404, -1525,
    2668, -1525,  -499, -1525,    62, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525, -1525,  -457, -1525, -1525,  -159,
     193,    28, -1525, -1525, -1525, -1525, -1525, -1525,    36, -1525,
   -1525, -1525, -1525,    38, -1525, -1525,   868,   876,   875,  -100,
     379,  -864,   388,   443,  -459,   150,  -927, -1525,  -186, -1525,
   -1525, -1525, -1525,  -735,    -7, -1525, -1525, -1525, -1525,  -451,
   -1525,  -612, -1525,  -419, -1525, -1525,   752, -1525,  -172, -1525,
   -1525, -1052, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525, -1525,  -204, -1525,  -119, -1525, -1525, -1525, -1525, -1525,
    -284, -1525,   -34,  -951, -1525, -1524,  -456, -1525,  -132,   137,
    -133,  -445, -1525,  -272, -1525, -1525, -1525,    -1,   -29,   -10,
      21,  -742,  -418, -1525, -1525,   -11, -1525, -1525,    -5,   -57,
    -137, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
    -609,  -852, -1525, -1525, -1525, -1525, -1525,  1786, -1525, -1525,
   -1525, -1525,   915, -1525, -1525,   304, -1525,   826, -1525, -1525,
   -1525, -1525, -1525, -1525, -1525,   308, -1525,   827, -1525, -1525,
     540, -1525,   274, -1525, -1525, -1525, -1525, -1525, -1525, -1525,
   -1525,  -945, -1525,  2091,  1674,  -395, -1525, -1525,   228,  3198,
    2382, -1525, -1525,   354,  -194,  -646, -1525, -1525,   427,   221,
    -726,   224, -1525, -1525, -1525, -1525, -1525,   414, -1525, -1525,
   -1525,    17,  -873,  -164,  -424,  -416, -1525,   479,  -110, -1525,
   -1525,    24,    27,   602, -1525, -1525,   663,   -33, -1525, -1525,
      56,  -136, -1525,   284, -1525, -1525, -1525,  -432,  1038, -1525,
   -1525, -1525, -1525, -1525,   480,   478, -1525, -1525, -1525,  -354,
    -656, -1525,   998,  -968, -1525,   -66,  -183,   -46,   590, -1525,
   -1059,    30,  -344,   315,   393, -1525, -1525, -1525, -1525,   342,
     355, -1096
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1008
static const yytype_int16 yytable[] =
{
     187,   189,   420,   191,   192,   193,   195,   196,   557,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   160,   246,   225,   228,   383,   497,   793,   467,   386,
     387,   648,   130,   514,   251,   937,  1303,   249,   495,   255,
     132,   919,   133,   918,   337,   257,   987,   260,   518,  1132,
     338,   738,   343,   420,   237,   489,   394,   238,   715,  1122,
     466,  1005,   757,   248,   768,  1289,   128,   899,   416,  1300,
     758,   764,   765,   349,   249,  1019,   522,   960,  1149,  1199,
     396,   807,   854,  1549,   392,   -43,   239,   -78,    13,   410,
     -43,   530,   -78,   594,  1160,   599,   379,   328,   789,   380,
     369,   786,  1702,  1020,    13,   -42,   790,   604,   393,   530,
     -42,  1497,    13,  1499,  -355,  1356,  1557,  1386,   872,   809,
    1641,  1709,  -862,  1709,  -554,  1549,  1703,   530,   888,   888,
     888,   523,  -556,   888,   726,   622,    13,   582,    13,   888,
     358,   135,  1208,  1209,   484,   485,  -556,   506,  1726,  1208,
    1209,  1101,  -104,  1697,   487,   920,  -103,  1408,   508,    13,
    -837,  1304,  1740,  -840,  -838,  1207,  1208,  1209,  1101,  1698,
       3,   484,   485,   516,   447,   448,   449,   450,   451,   452,
    -104,   453,   396,   726,  -103,   500,   507,  1699,  -837,   354,
    1226,  -840,  -838,   454,   190,   583,   355,   129,   484,   485,
     487,   515,  -980,   242,  1769,  -865,   623,  -869,  -863,   372,
     393,  -698,  -868,  -864,  -876,  -905,   492,   484,   485,   492,
     245,  1387,  -866,   407,  1133,   393,  1388,  -524,    60,    61,
      62,   178,  1389,   417,  1390,  -908,  -907,   525,  -849,  -850,
     525,   350,   211,  -294,  1211,   898,  1305,   249,   536,   647,
     547,  1359,  1420,   211,  -980,  -872,  -782,  -873,  -862,  -782,
     687,  1422,   384,  -557,  -294,   491,   878,  1021,  1428,  1134,
    1430,  1391,  1392,   808,  1393,  1349,  1550,  1551,   -43,   491,
     -78,  1206,   411,  1210,   531,   905,   595,  1357,   600,   388,
     488,  -278,   810,  -782,  1704,   418,  1002,   468,   -42,  1325,
     605,  1004,   621,  -699,  1498,   610,  1500,  -355,   527,  1558,
     873,   775,   532,  1642,  1710,  1102,  1759,  1394,  1825,   874,
     889,   973,  1281,  1180,   728,  1447,  1741,  1146,   419,   613,
    1336,  1504,  1797,   609,  1114,   420,   488,   341,  -871,   769,
    -875,  -865,  -697,  -869,  -863,  -878,  -701,   685,  -868,  -864,
     728,  -905,   493,   596,  1135,   493,   249,   393,  -866,   588,
     592,   593,  1518,   225,  1520,  1295,   351,   494,   627,  1413,
    1830,  -908,  -907,   728,  -849,  -850,  1510,   328,   116,   498,
    1526,  1073,   608,   728,   252,  1181,   728,  1353,  1208,  1209,
     906,  -874,   632,   670,  1837,   955,  1851,   383,   732,   733,
     416,   453,   407,  1844,   682,   907,   389,   253,   657,   334,
     406,  1296,   390,   454,  1414,  1831,  1811,  1812,  1813,  1469,
     254,   370,   688,   689,   690,   692,   693,   694,   695,   696,
     697,   698,   699,   700,   701,   702,   703,   704,   705,   706,
     707,   708,   709,   710,   711,   712,   713,   714,  1845,   716,
     912,   717,   717,   328,   720,  1107,  1346,  1339,   739,  1288,
     360,  1511,  1672,   121,   740,   742,   743,   744,   745,   746,
     747,   748,   749,   750,   751,   752,   753,   237,   361,  1838,
     238,  1852,   717,   763,   384,   682,   682,   717,   767,   624,
     373,   374,   860,   362,   740,   926,   928,   771,   363,  1415,
    1832,  1720,   864,   725,  1139,  1552,   779,  -558,   781,   239,
     258,   370,  1140,   327,  1205,   682,   727,   796,  1457,   406,
     484,   485,  1302,   797,   367,   798,   865,  1784,   952,  1654,
     364,  1655,  1090,  1846,   135,  1312,  1157,   467,  1314,  1093,
     368,   370,   761,   954,   385,   370,  1721,   629,   381,   406,
     364,   399,   328,   398,   364,   364,   405,   648,   842,   406,
     406,   845,   848,   849,  1705,   727,   634,   635,  1165,   466,
     484,   485,  1785,   370,  1290,   785,   962,   966,   791,   629,
     373,   374,  1706,   406,   909,  1707,  -980,  1291,  1530,   406,
     129,  -700,   868,   364,   356,   801,   406,   401,   403,   404,
     857,   409,   357,  1435,   861,  1436,   164,   407,   412,   349,
     373,   374,   926,   928,   373,   374,  1728,   421,   994,   928,
    1292,   944,   422,  1026,   423,   545,   424,   546,   221,   223,
     456,   370,    33,    34,    35,  -980,   393,   371,  -980,   425,
    -876,  1752,   373,   374,   212,   881,   882,   518,   426,    60,
      61,    62,   178,   179,   417,  1074,   457,   504,   370,  1753,
     735,   459,  1754,   460,   629,  1429,   458,    53,   490,   672,
     657,  -870,   995,  -698,   934,    60,    61,    62,   178,   179,
     417,   550,  1069,  1070,  1071,  -555,   945,   496,   648,   222,
     222,  1412,  1283,    74,    75,    76,    77,    78,  1072,   372,
     373,   374,   673,  1424,   214,   121,   501,   999,  1000,   121,
      82,    83,  1529,   537,   370,   395,   418,   370,  1822,   953,
     402,  1190,  1191,   629,    92,   377,   630,   373,   374,  1276,
    1277,   195,   503,  1836,  1465,  1466,  1807,  1318,    97,  1668,
    1669,   719,   418,  1826,  1827,   454,  1502,   407,  1328,  1820,
     965,  1750,  1751,  1746,  1747,    60,    61,    62,    63,    64,
     417,   509,   677,   512,  1833,   334,    70,   461,  -874,  1348,
     513,   762,  1381,  1029,  1032,   647,   766,   491,   728,  -696,
     519,   520,   997,   373,   374,   528,   373,   374,   541,  1003,
     728,   548,   728,   556,  1184, -1007,   551,   558,   249,   450,
     451,   452,  1676,   453,   463,   552,   468,   559,    60,    61,
      62,   178,   179,   417,   601,   454,   561,   395,  1553,   603,
     602,  1527,   418,   614,   615,   649,   510,   650,   648,   562,
     563,   564,   517,   671,   659,  1075,   565,   566,  1219,   660,
     567,   568,   662,  1443,    53,  1223,   774,  -129,   164,   625,
    1404,   684,   164,   631,   624,   772,   121,   776,   728,  1452,
     777,   782,   783,   799,   530,   674,   803,   806,   547,   327,
    1014,   819,   364,   820,   588,   418,   840,   625,   592,   631,
     625,   631,   631,   841,   843,   855,   844,   222,   852,   859,
     863,  1112,   871,   858,   862,   885,  1164,   880,  1426,   875,
     896,   876,   812,   879,   887,  1121,   647,   901,   890,  1800,
    1066,  1067,  1068,  1069,  1070,  1071,   160,   902,   904,   556,
     364,   730,   364,   364,   364,   364,  -716,   130,  1800,  1072,
     135,  1144,   910,   911,   913,   132,  1821,   133,   914,   657,
     917,  1152,   921,  1531,  1153,   756,  1154,   922,   930,   932,
     682,  1729,  1537,   598,   935,  1308,   657,  1124,   936,   938,
     941,   128,   606,  1543,   611,   947,   948,   950,   556,   618,
    1125,   951,   959,   967,  -701,   969,   628,   971,  1022,   970,
     943,   788,   761,  1332,   791,   996,   129,   237,  1188,  1193,
     238,   135,   121,  1006,  1016,  1018,  1024,  1023,  1025,   164,
    1515,  1041,  1027,  1044,   915,   916,  1076,  1040,  1042,  1078,
      36,   838,  1045,   924,   222,  1077,   648,  1082,  1088,   239,
    1085,  1098,  1106,   222,  1100,   612,  1110,  1117,  1111,  1119,
     222,    48,  1120,  1371,  1130,  1682,   135,   222,  1126,  1128,
    1376,  1159,  1147,  1156,  1162,  1168,   647,   129,  1167,  -877,
     791,  1178,  1183,  1196,  1179,   867,  1489,   135,  1182,  1198,
    1185,  1201,  1194,  1202,  1204,  1213,  1217,  1218,  1222,  1072,
    1270,  1264,  1265,  1266,  1221,  1279,  1765,   845,  1268,  1282,
    1298,   893,   895,  1306,  1225,  1299,   648,  1231,  1311,  1307,
      86,    87,   129,    88,   183,    90,  1313,  1315,   618,  1284,
    1317,  1320,  1319,  1386,  1323,  1285,  1322,  1329,   980,  1331,
    1342,  1330,  1335,   129,  1343,  1345,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,  1350,
    1360,   160,  1490,  1362,  1364,   164,  1354,  1365,  1310,   135,
    1370,   135,   130,  1368,  1441,    13,   364,  1372,  1369,  1375,
     132,   682,   133,  1810,  1374,  1380,  1405,  1416,  1406,   222,
    1419,  1417,   682,  1285,   657,  1431,  1421,   657,  1723,  1423,
    1724,  1427,  1432,  1433,   677,   677,   128,  1434,  1437,  1730,
    1341,  1438,  1439,  1440,  1442,  1444,  1445,  1446,  1486,  1449,
    1450,  1473,  1506,   249,  1462,   129,  1501,   129,  1512,  1513,
    1516,  1521,  1524,  1355,  1528,  1540,  1522,  1387,  1547,  1555,
    1650,  1538,  1388,  1656,    60,    61,    62,   178,  1389,   417,
    1390,  1556,  1649,  1662,  1663,  1665,  1768,  1666,  1667,  1677,
    1675,   984,  1708,   990,   647,  1687,  1688,  1714,   499,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
     481,   135,   121,  1717,  1718,  1344,  1115,  1391,  1392,  1725,
    1393,  1742,  1744,  1748,  1757,   420,  1012,   121,  1756,  1763,
    1758,  1503,  1127,  1764,  1773,  1772,  1771,  -351,  1775,  1776,
     933,   418,  1703,  1779,  1780,  1141,  1783,  1409,   482,   483,
    1786,  1410,  1788,  1411,  1789,  1790,  1795,  1403,  1805,  1802,
    1809,  1418,  1808,  1407,   647,  1817,  1403,   129,  1819,  1823,
    1824,  1425,   682,   121,  1161,  1834,  1839,  1840,  1847,  1092,
    1853,  1848,  1854,  1095,  1096,  1856,  1711,  1857,  1835,   866,
    1094,  1091,   657,  1804,   734,  1842,  1652,   729,   731,   964,
    1163,   222,   135,  1103,  1158,  1116,  1818,  1681,  1451,  1347,
    1816,   869,  1673,  1696,   484,   485,  1554,  1701,   121,  1493,
   -1008, -1008, -1008, -1008, -1008,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,   556,  1461,  1212,  1829,  1841,  1214,   121,
     991,  1713,   992,  1671,   741,   756,  1072,   788,  1474,  1269,
    1267,  1084,  1293,  1327,  1224,   164,   850,   851,   129,  1333,
     222,  1546,  1174,  1334,  1186,  1138,   620,  1010,   683,  1030,
     164,  1514,  1464,  1781,   682,  1263,  1275,  1398,  1216,     0,
       0,   364,     0,     0,     0,   661,  1398,     0,     0,     0,
       0,     0,     0,  1172,  1172,   984,     0,     0,     0,  1403,
       0,   222,     0,   222,     0,  1403,     0,  1403,     0,     0,
     657,     0,     0,   788,     0,     0,   164,     0,     0,   121,
       0,   121,     0,   121,     0,     0,     0,     0,   222,     0,
       0,     0,     0,     0,  1099,     0,     0,     0,     0,     0,
       0,     0,     0,  1215,     0,  1301,     0,   924,     0,   618,
    1109,     0,     0,     0,     0,     0,     0,     0,  1716,  1743,
     556,   164,     0,   556,     0,     0,     0,  1664,     0,     0,
    1535,     0,     0,  1548,  1321,     0,     0,  1324,   135,     0,
       0,     0,   164,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   838,     0,     0,   222,     0,  1685,     0,     0,
     468,     0,     0,  1403,     0,     0,     0,     0,     0,  1272,
     222,   222,  1273,     0,     0,     0,     0,     0,     0,  1398,
       0,     0,     0,     0,     0,  1398,     0,  1398,  1361,     0,
       0,     0,  1141,   121,   129,     0,     0,     0,     0,     0,
     135,     0,     0,     0,     0,     0,     0,   216,   216,   135,
    1495,   231,     0,     0,  1679,  1535,     0,     0,     0,     0,
       0,     0,   164,     0,   164,     0,   164,     0,  1010,  1200,
       0,     0,     0,     0,     0,   231,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,   481,     0,
       0,  1383,  1384,     0,     0,     0,   129,     0,   984,     0,
       0,     0,   984,  1386,  1793,   129,     0,     0,     0,     0,
    1761,     0,   121,     0,     0,     0,     0,  1719,     0,     0,
       0,   420,     0,  1398,   121,     0,   482,   483,   135,   222,
     222,     0,     0,     0,   135,     0,     0,     0,     0,     0,
     135,     0,     0,     0,     0,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   328,  1657,     0,     0,     0,
     219,   219,     0,     0,   233,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   164,  1453,     0,  1454,
       0,     0,     0,     0,   129,     0,     0,     0,     0,     0,
     129,     0,   484,   485,  1382,     0,   129,     0,     0,     0,
       0,     0,     0,     0,  1309,     0,     0,  1387,     0,     0,
       0,     0,  1388,  1496,    60,    61,    62,   178,  1389,   417,
    1390,     0,     0,     0,   499,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,     0,     0,     0,
       0,     0,     0,     0,     0,   216,     0,     0,   984,  1340,
     984,     0,     0,     0,     0,   164,     0,  1391,  1392,   657,
    1393,     0,     0,   618,  1010,   222,     0,   164,     0,     0,
       0,     0,   217,   217,   482,   483,     0,     0,   657,   135,
       0,   418,     0,     0,     0,     0,   657,     0,     0,     0,
       0,   231,     0,   231,  1849,     0,     0,     0,     0,     0,
     121,     0,  1855,  1519,     0,     0,   327,     0,  1858,     0,
     222,  1859,  1491,     0,     0,     0,     0,     0,     0,     0,
     135,   135,     0,     0,   222,   222,     0,   135,     0,     0,
       0,     0,     0,     0,     0,   129,     0,     0,     0,     0,
     484,   485,     0,     0,   618,     0,     0,   231,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   984,   219,     0,
       0,     0,   121,     0,     0,   135,     0,   121,     0,     0,
    1692,   121,   216,  1794,     0,     0,   129,   129,     0,     0,
       0,   216,     0,   129,     0,     0,   364,     0,   216,   556,
       0,     0,   327,     0,     0,   216,     0,     0,     0,     0,
       0,     0,  1638,     0,     0,   222,   231,     0,     0,  1645,
       0,   784,     0,     0,     0,     0,   327,     0,   327,     0,
       0,   129,     0,     0,   327,  1386,     0,     0,   231,     0,
       0,   231,   135,     0,     0,     0,     0,     0,     0,   135,
       0,     0,     0,   164,     0,     0,     0,   984,     0,     0,
     121,   121,   121,     0,     0,     0,   121,     0,     0,     0,
       0,     0,   121,     0,     0,     0,     0,    13,     0,     0,
     217,  1715,     0,     0,   231,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   219,     0,     0,   129,     0,
       0,     0,     0,     0,   219,   129,     0,     0,     0,     0,
       0,   219,     0,     0,     0,   164,     0,     0,   219,     0,
     164,     0,     0,     0,   164,     0,     0,   216,     0,   646,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1387,
       0,     0,     0,     0,  1388,     0,    60,    61,    62,   178,
    1389,   417,  1390,  1386,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1777,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   231,   231,
       0,   556,   830,     0,     0,     0,     0,   218,   218,  1391,
    1392,   232,  1393,     0,     0,    13,     0,   233,     0,     0,
       0,     0,   327,   164,   164,   164,   984,   217,     0,   164,
       0,   121,     0,   418,     0,   164,   217,     0,     0,     0,
    1735,  1386,     0,   217,     0,     0,     0,  1638,  1638,     0,
     217,  1645,  1645,     0,   830,  1523,     0,     0,     0,   924,
     219,     0,     0,     0,     0,   364,     0,     0,     0,     0,
       0,     0,   121,   121,   924,     0,     0,  1387,     0,   121,
       0,     0,  1388,    13,    60,    61,    62,   178,  1389,   417,
    1390,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     231,   231,     0,     0,     0,     0,     0,     0,     0,   231,
       0,     0,     0,     0,     0,     0,     0,   121,     0,     0,
       0,     0,     0,     0,  1792,     0,     0,  1391,  1392,   216,
    1393,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1806,     0,     0,  1387,     0,     0,     0,     0,
    1388,   418,    60,    61,    62,   178,  1389,   417,  1390,    60,
      61,    62,    63,    64,   417,     0,     0,     0,     0,     0,
      70,   461,   217,  1525,   164,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   121,     0,     0,     0,   216,     0,
       0,   121,     0,     0,     0,  1391,  1392,     0,  1393,     0,
       0,     0,     0,     0,     0,   218,   462,     0,   463,     0,
       0,     0,     0,     0,     0,   164,   164,     0,     0,   418,
       0,   464,   164,   465,     0,     0,   418,     0,     0,   216,
       0,   216,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1674,   219,   499,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,   481,   216,   830,     0,     0,
     164,   813,     0,     0,     0,     0,     0,     0,     0,     0,
     231,   231,   830,   830,   830,   830,   830,     0,     0,     0,
     830,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,   482,   483,     0,     0,     0,     0,     0,
       0,   219,     0,     0,     0,     0,   231,     0,     0,     0,
      36,    48,     0,     0,     0,     0,     0,     0,     0,   814,
       0,     0,     0,   216,     0,     0,     0,   164,     0,     0,
       0,    48,   218,     0,   164,     0,   231,     0,   216,   216,
       0,   218,   219,     0,   219,     0,     0,     0,   218,     0,
       0,     0,   231,   231,   217,   218,     0,     0,     0,   484,
     485,     0,   231,     0,     0,     0,   218,   377,   231,   219,
      86,    87,     0,    88,   183,    90,     0,     0,     0,     0,
       0,   231,     0,     0,   182,     0,     0,    84,     0,   830,
      86,    87,   231,    88,   183,    90,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
     231,     0,     0,   217,   231,   378,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
     877,     0,     0,     0,   232,     0,   219,   499,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,   481,
       0,   219,   219,     0,   217,     0,   217,   216,   216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   231,     0,     0,   231,   646,   231,   218,     0,     0,
       0,   217,     0,     0,     0,     0,     0,   482,   483,     0,
       0,   830,     0,   231,     0,     0,   830,   830,   830,   830,
     830,   830,   830,   830,   830,   830,   830,   830,   830,   830,
     830,   830,   830,   830,   830,   830,   830,   830,   830,   830,
     830,   830,   830,   830,     0,     0,     0,   233,     0,     0,
       0,     0,   834, -1008, -1008, -1008, -1008, -1008,   445,   446,
     447,   448,   449,   450,   451,   452,     0,   453,   217,   830,
       0,     0,     0,   484,   485,     0,     0,     0,     0,   454,
       0,     0,     0,   217,   217,     0,     0,     0,     0,     0,
     219,   219,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   231,     0,   231,   834,     0,     0,     0,     0,     0,
       0,     0,     0,   216,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   646,     0,     0,     0,
     231,     0,   330,   231,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   231,   453,     0,     0,     0,   216,     0,
       0,     0,     0,     0,     0,     0,   454,     0,     0,     0,
       0,     0,   216,   216,     0,   830,     0,     0,     0,   218,
       0,     0,     0,     0,   231,     0,     0,     0,   231,     0,
       0,   830,     0,   830,     0,     0,     0,     0,     0,     0,
       0,     0,   217,   217,     0,     0,     0,     0,     0,   830,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   219,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   218,     0,
       0,     0,     0,     0,     0,     0,     0,   231,   231,     0,
     231,     0,     0,   216,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   646,     0,     0,     0,
       0,   219,     0,     0,     0,     0,     0,     0,     0,   218,
       0,   218,     0,     0,     0,   219,   219,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,   218,   834,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1072,
       0,     0,   834,   834,   834,   834,   834,     0,     0,     0,
     834,     0,     0,   231,     0,   231,     0,     0,   217,     0,
     830,   231,     0,   836,   830,     0,   830,     0,   330,   830,
     330,     0,     0,     0,     0,     0,  1087,     0,   231,   231,
       0,     0,   231,     0,     0,     0,   219,     0,     0,   231,
       0,     0,     0,   218,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   217,     0,     0,  1105,     0,   218,   218,
       0,     0,     0,     0,     0,   870,     0,   217,   217,     0,
       0,     0,     0,  1105,   330,     0,     0,     0,     0,     0,
       0,   231,   218,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   830,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   231,   231,     0,   834,
       0,     0,  1148,     0,   231,     0,   231,     0,     0,     0,
       0,     0,     0,     0,   646,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   232,     0,     0,     0,   231,     0,
     231,     0,     0,     0,     0,     0,   231,     0,   217,     0,
       0,     0,     0,     0,     0,   330,     0,     0,   330,     0,
       0,  1169,  1170,  1171,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   218,   218,     0,
       0,   830,   830,     0,     0,    48,     0,     0,   830,     0,
     231,     0,     0,     0,   646,     0,   231,     0,   231,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   834,     0,   218,     0,     0,   834,   834,   834,   834,
     834,   834,   834,   834,   834,   834,   834,   834,   834,   834,
     834,   834,   834,   834,   834,   834,   834,   834,   834,   834,
     834,   834,   834,   834,    86,    87,     0,    88,   183,    90,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1011,   834,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,  1033,  1034,  1035,  1036,     0,     0,     0,
       0,  1043,     0,     0,     0,   330,   815,   231,     0,   832,
       0,     0,     0,     0,   220,   220,     0,     0,   235,     0,
       0,     0,     0,   218,   231,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,   231,   270,     0,     0,     0,     0,   830,     0,
       0,     0,     0,     0,     0,  1072,     0,     0,     0,   830,
       0,   832,     0,   218,     0,   830,     0,     0,   218,   830,
     272,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   218,   218,     0,   834,     0,     0,     0,     0,
       0,   231,    36,     0,     0,     0,     0,     0,     0,     0,
       0,   834,     0,   834,     0,     0,     0,   330,   330,     0,
    1145,     0,     0,    48,     0,     0,   330,     0,     0,   834,
       0,   549,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   830,     0,     0,     0,     0,     0,     0,     0,     0,
     231,     0,     0,     0,     0,     0,     0,     0,   543,   544,
       0,     0,     0,     0,     0,     0,   231,     0,     0,     0,
    1385,     0,     0,   218,     0,   231,   182,     0,     0,    84,
     321,     0,    86,    87,     0,    88,   183,    90,     0,     0,
     231,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     325,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   220,     0,   326,     0,     0,  1232,  1235,  1236,
    1237,  1239,  1240,  1241,  1242,  1243,  1244,  1245,  1246,  1247,
    1248,  1249,  1250,  1251,  1252,  1253,  1254,  1255,  1256,  1257,
    1258,  1259,  1260,  1261,  1262,     0,     0,     0,     0,     0,
     834,   218,     0,     0,   834,     0,   834,     0,     0,   834,
       0,     0,     0,     0,   832,     0,     0,     0,     0,  1472,
    1274,     0,  1485,   427,   428,   429,     0,   330,   330,   832,
     832,   832,   832,   832,     0,     0,     0,   832,     0,     0,
       0,     0,   430,     0,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,     0,   453,     0,     0,
       0,   218,     0,     0,     0,     0,     0,     0,     0,   454,
       0,     0,     0,     0,     0,     0,   834,     0,     0,   220,
       0,     0,     0,     0,     0,     0,  1544,  1545,   220,     0,
       0,     0,     0,     0,     0,   220,  1485,     0,     0,   330,
       0,     0,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   235,     0,   330,  1351,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   330,     0,
       0,     0,  1366,     0,  1367,     0,   832,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1377,     0,     0,     0,     0,     0,     0,   330,     0,     0,
       0,   834,   834,     0,   427,   428,   429,     0,   834,     0,
    1690,     0,     0,     0,     0,     0,     0,     0,  1485,     0,
       0,   235,     0,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,     0,   453,     0,
    1286,     0,     0,     0,     0,     0,     0,     0,   330,     0,
     454,   330,     0,   815,   220,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   832,     0,
       0,     0,     0,   832,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,     0,     0,     0,     0,     0,     0,     0,     0,   835,
       0,  1456,     0,     0,     0,  1458,     0,  1459,     0,     0,
    1460,     0,     0,     0,     0,     0,   832,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,     0,   453,     0,     0,   834,     0,
       0,     0,     0,     0,     0,     0,     0,   454,   330,   834,
     330,   835,     0,     0,     0,   834,     0,     0,     0,   834,
       0,     0,     0,     0,     0,     0,     0,   427,   428,   429,
       0,     0,     0,     0,     0,     0,     0,   330,   897,     0,
     330,     0,     0,     0,     0,     0,   430,  1539,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   834,   832,   454,     0,     0,   220,     0,     0,     0,
    1803,   330,     0,     0,     0,   330,     0,     0,   832,     0,
     832,     0,     0,     0,     0,     0,  1472,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   832,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1683,  1684,     0,     0,     0,     0,     0,  1689,
       0,     0,     0,     0,     0,   220,     0,     0,     0,     0,
       0,     0,     0,     0,   330,   330,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   831,     0,     0,   427,   428,
     429,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   220,   430,   220,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,     0,   453,   220,   835,     0,     0,   831,     0,     0,
       0,   929,     0,     0,   454,     0,     0,     0,     0,   835,
     835,   835,   835,   835,     0,     0,     0,   835,     0,     0,
     330,     0,   330,     0,     0,     0,     0,   832,     0,     0,
       0,   832,     0,   832,     0,     0,   832,     0,     0,     0,
       0,     0,     0,     0,     0,   330,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   330,     0,     0,     0,
     220,     0,     0,     0,     0,     0,     0,     0,     0,  1745,
       0,     0,     0,     0,     0,   220,   220,     0,     0,     0,
    1755,     0,     0,     0,     0,     0,  1760,     0,     0,     0,
    1762,     0,     0,     0,     0,     0,     0,     0,     0,   235,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   832,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   835,     0,     0,     0,
       0,   330,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   968,     0,     0,   427,   428,   429,     0,     0,
       0,   235,  1796,     0,     0,   330,     0,   330,     0,     0,
       0,     0,     0,   330,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,    36,   453,
     211,     0,     0,     0,   220,   220,     0,     0,   832,   832,
     831,   454,     0,     0,     0,   832,     0,     0,     0,    48,
       0,     0,     0,   330,     0,   831,   831,   831,   831,   831,
       0,     0,     0,   831,     0,     0,     0,     0,   835,     0,
     235,     0,     0,   835,   835,   835,   835,   835,   835,   835,
     835,   835,   835,   835,   835,   835,   835,   835,   835,   835,
     835,   835,   835,   835,   835,   835,   835,   835,   835,   835,
     835,     0,     0,     0,     0,     0,   754,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   835,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
       0,     0,     0,   755,   330,   116,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     220,   330,   831,     0,     0,     0,     0,     0,     0,   972,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1736,
       0,     0,     0,     0,     0,   832,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   832,     0,     0,     0,
     235,     0,   832,     0,     0,   220,   832,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   220,
     220,     0,   835,     0,     0,     0,     0,     0,   330,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   835,     0,
     835,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   835,     0,     0,     0,
       0,     0,     0,     0,   831,     0,     0,     0,   832,   831,
     831,   831,   831,   831,   831,   831,   831,   831,   831,   831,
     831,   831,   831,   831,   831,   831,   831,   831,   831,   831,
     831,   831,   831,   831,   831,   831,   831,     0,     0,     0,
     220,     0,   330,     0,     0,   427,   428,   429,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   330,     0,     0,
       0,     0,   831,     0,   430,     0,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,     0,   453,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   454,     0,     0,     0,     0,     0,     0,     0,    36,
       0,   891,   892,     0,     0,     0,     0,     0,   427,   428,
     429,     0,     0,     0,     0,     0,     0,   835,   235,     0,
      48,   835,     0,   835,     0,     0,   835,   430,  1356,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,     0,   453,   261,   262,     0,   263,   264,   831,     0,
     265,   266,   267,   268,   454,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   831,     0,   831,   269,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,   235,     0,
       0,     0,   831,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   835,   271,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   273,   274,
     275,   276,   277,   278,   279,     0,     0,     0,    36,  1097,
     211,     0,     0,     0,     0,     0,     0,     0,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,    48,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,     0,     0,     0,     0,   723,   314,   315,   316,
       0,     0,     0,   317,   553,   554,     0,     0,   835,   835,
       0,     0,     0,     0,     0,   835,     0,     0,     0,     0,
    1357,     0,   555,     0,  1695,     0,     0,     0,    86,    87,
       0,    88,   183,    90,   322,     0,   323,     0,     0,   324,
       0,     0,     0,   831,     0,     0,     0,   831,     0,   831,
       0,     0,   831,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   427,   428,   429,
       0,     0,     0,   724,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,   427,   428,   429,     0,     0,     0,     0,     0,
       0,     0,     0,   454,     0,     0,     0,     0,     0,   831,
       0,   430,     0,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,     0,   453,     0,     0,     0,
       0,     0,     0,     0,     0,   835,     0,     0,   454,     0,
       0,     0,     0,     0,     0,     0,   835,     0,     0,     0,
       0,     0,   835,     0,     0,     0,   835,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   427,   428,   429,     0,     0,     0,     0,     0,     0,
    1778,     0,     0,     0,   831,   831,     0,     0,     0,     0,
     430,   831,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,     0,   453,     0,     0,   835,   427,
     428,   429,     0,     0,     0,     0,     0,   454,     0,     0,
       0,  1155,     0,     0,     0,     0,     0,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,     0,   453,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,   454,  1166,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,   831,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,   831,     0,    41,    42,    43,    44,   831,    45,
       0,    46,   831,    47,     0,  1189,    48,    49,     0,     0,
       0,    50,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,  1508,    80,     0,     0,     0,     0,    81,
      82,    83,    84,    85,   831,    86,    87,     0,    88,    89,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,    95,     0,    96,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1113,   116,   117,     0,   118,   119,     5,     6,     7,
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
      52,    53,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,    69,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,    81,    82,    83,    84,
      85,     0,    86,    87,     0,    88,    89,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,    95,     0,    96,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1287,   116,
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
      48,    49,     0,     0,     0,    50,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,    81,    82,    83,    84,    85,     0,    86,
      87,     0,    88,    89,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,    95,     0,
      96,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,     0,   116,   117,     0,   118,
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
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,   663,   116,   117,     0,   118,   119,     5,     6,
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
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1086,
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
       0,     0,   113,     0,   114,   115,  1129,   116,   117,     0,
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
       0,   114,   115,  1195,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,  1197,    45,     0,
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
    1352,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
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
     113,     0,   114,   115,  1463,   116,   117,     0,   118,   119,
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
     115,  1686,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,  1731,
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
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1766,   116,   117,     0,   118,
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
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,  1767,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,  1770,    46,
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
       0,     0,   113,     0,   114,   115,  1787,   116,   117,     0,
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
       0,   114,   115,  1843,   116,   117,     0,   118,   119,     5,
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
    1850,   116,   117,     0,   118,   119,     5,     6,     7,     8,
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
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     526,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,   178,   179,
      65,     0,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   800,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,   178,   179,    65,     0,    66,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,  1013,     0,     0,     0,     0,     0,
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
       0,  1534,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,  1678,     0,
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
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,   736,
      12,     0,     0,     0,     0,     0,     0,   737,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
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
       0,     0,     0,    97,    98,   335,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,     0,     0,     0,   116,   117,     0,
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
      97,    98,   335,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   336,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,     0,   453,
       0,   678,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   454,     0,    14,    15,     0,     0,     0,     0,    16,
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
     112,     0,     0,   113,     0,     0,   795,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,     0,     0,     0,  1142,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1072,     0,    14,    15,     0,
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
       0,    88,   183,    90,     0,  1143,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   736,    12,     0,     0,     0,     0,
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
     108,   109,   110,   111,   112,     0,     0,   113,     0,   427,
     428,   429,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,     0,   453,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,   454,     0,    16,     0,    17,
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
      94,  1079,  1080,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,     0,     0,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10, -1008, -1008, -1008, -1008,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,     0,   453,     0,     0,   224,     0,     0,     0,
       0,     0,     0,     0,     0,   454,     0,     0,    14,    15,
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
       0,   113,     0,   427,   428,   429,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   430,     0,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,     0,   453,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,   454,
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
       0,     0,  1732,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     256,   428,   429,   116,   117,     0,   118,   119,     5,     6,
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
      60,    61,    62,   178,   179,   180,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   259,  1047,  1048,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1049,     0,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,  1072,     0,    16,     0,    17,    18,    19,
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
   -1008, -1008, -1008, -1008,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,     0,     0,     0,
       0,   737,     0,     0,     0,     0,     0,     0,     0,     0,
    1072,     0,     0,    14,    15,     0,     0,     0,     0,    16,
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
       0,     0,     0,     0,     0,     0,     0,     0,   778,     0,
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
       0,     0,     0,     0,     0,   780,     0,     0,     0,     0,
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
       0,     0,  1187,     0,     0,     0,     0,     0,     0,     0,
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
     108,   109,   110,   111,   112,     0,     0,   113,     0,   427,
     428,   429,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,     0,   453,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,   454,     0,    16,     0,    17,
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
       0,    92,     0,     0,    93,     0,     0,  1542,     0,     0,
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
     626,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   178,
     179,   180,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,    93,     0,  1379,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,     0,     0,     0,   116,   117,     0,   118,
     119,   261,   262,     0,   263,   264,     0,   429,   265,   266,
     267,   268,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   430,   269,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,     0,   453,
       0,     0,   271,     0,     0,     0,     0,     0,     0,     0,
       0,   454,     0,     0,     0,     0,   273,   274,   275,   276,
     277,   278,   279,     0,     0,     0,    36,     0,   211,     0,
       0,     0,     0,     0,     0,     0,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,    48,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
       0,     0,     0,     0,   313,   314,   315,   316,     0,     0,
       0,   317,   553,   554,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   261,   262,     0,   263,   264,     0,
     555,   265,   266,   267,   268,     0,    86,    87,     0,    88,
     183,    90,   322,     0,   323,     0,     0,   324,   269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   271,     0,     0,     0,     0,
       0,   724,     0,   116,     0,     0,     0,     0,     0,   273,
     274,   275,   276,   277,   278,   279,     0,     0,     0,    36,
       0,   211,     0,     0,     0,     0,     0,     0,     0,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
      48,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,     0,     0,     0,     0,  1123,   314,   315,
     316,     0,     0,     0,   317,   553,   554,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   261,   262,     0,
     263,   264,     0,   555,   265,   266,   267,   268,     0,    86,
      87,     0,    88,   183,    90,   322,     0,   323,     0,     0,
     324,   269,     0,   270,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   271,     0,
     272,     0,     0,     0,     0,     0,   116,     0,     0,     0,
       0,     0,   273,   274,   275,   276,   277,   278,   279,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,    48,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,     0,     0,     0,     0,
       0,   314,   315,   316,    36,     0,     0,   317,   318,   319,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,   320,     0,     0,    84,
     321,     0,    86,    87,     0,    88,   183,    90,   322,     0,
     323,     0,     0,   324,     0,     0,     0,     0,     0,     0,
     325,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   261,   262,   326,   263,   264,     0,  1658,   265,
     266,   267,   268,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,     0,     0,     0,   269,     0,   270,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   271,     0,   272,   684,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   273,   274,   275,
     276,   277,   278,   279,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,    48,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,     0,     0,     0,     0,     0,   314,   315,   316,    36,
       0,     0,   317,   318,   319,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,   320,     0,     0,    84,   321,     0,    86,    87,     0,
      88,   183,    90,   322,     0,   323,     0,     0,   324,     0,
       0,     0,     0,     0,     0,   325,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   261,   262,   326,
     263,   264,     0,  1727,   265,   266,   267,   268,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,     0,     0,
       0,   269,     0,   270,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   271,     0,
     272,   943,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   273,   274,   275,   276,   277,   278,   279,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,    48,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,     0,     0,     0,     0,
     313,   314,   315,   316,    36,     0,     0,   317,   318,   319,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,   320,     0,     0,    84,
     321,     0,    86,    87,     0,    88,   183,    90,   322,     0,
     323,     0,     0,   324,     0,     0,     0,     0,     0,     0,
     325,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   261,   262,   326,   263,   264,     0,     0,   265,
     266,   267,   268,     0,    86,    87,     0,    88,   183,    90,
       0,     0,     0,     0,     0,     0,   269,     0,   270,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   271,     0,   272,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   273,   274,   275,
     276,   277,   278,   279,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,    48,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,     0,     0,     0,     0,     0,   314,   315,   316,     0,
       0,     0,   317,   318,   319,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   320,     0,     0,    84,   321,     0,    86,    87,     0,
      88,   183,    90,   322,     0,   323,     0,     0,   324,     0,
       0,     0,     0,     0,     0,   325,  1467,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   261,   262,   326,
     263,   264,     0,     0,   265,   266,   267,   268,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,   430,   270,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,     0,   453,   271,     0,
     272,     0,     0,     0,     0,     0,     0,     0,     0,   454,
       0,     0,   273,   274,   275,   276,   277,   278,   279,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,    48,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,     0,     0,     0,     0,
       0,   314,   315,   316,     0,     0,     0,   317,   318,   319,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   320,     0,     0,    84,
     321,     0,    86,    87,     0,    88,   183,    90,   322,     0,
     323,     0,     0,   324,     0,     0,     0,     0,     0,     0,
     325,     0,     0,     0,   427,   428,   429,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   430,   326,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,     0,   453,     0,
       0,     0,     0,     0,  1559,  1560,  1561,  1562,  1563,     0,
     454,  1564,  1565,  1566,  1567,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1568,  1569,
       0,  1049,     0,  1050,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1570,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1072,  1571,
    1572,  1573,  1574,  1575,  1576,  1577,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1578,
    1579,  1580,  1581,  1582,  1583,  1584,  1585,  1586,  1587,  1588,
      48,  1589,  1590,  1591,  1592,  1593,  1594,  1595,  1596,  1597,
    1598,  1599,  1600,  1601,  1602,  1603,  1604,  1605,  1606,  1607,
    1608,  1609,  1610,  1611,  1612,  1613,  1614,  1615,  1616,  1617,
    1618,     0,     0,     0,  1619,  1620,     0,  1621,  1622,  1623,
    1624,  1625,     0,     0,     0,     0,     0,     0,  1509,     0,
       0,     0,     0,  1626,  1627,  1628,     0,     0,     0,    86,
      87,     0,    88,   183,    90,  1629,     0,  1630,  1631,     0,
    1632,     0,     0,     0,     0,     0,     0,  1633,  1634,     0,
    1635,     0,  1636,  1637,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   261,   262,
       0,   263,   264,     0,  1048,   265,   266,   267,   268,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1049,   269,  1050,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,     0,     0,     0,     0,   271,
       0,     0,     0,     0,     0,     0,     0,     0,  1072,     0,
       0,     0,     0,   273,   274,   275,   276,   277,   278,   279,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,    48,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,     0,     0,     0,
       0,   313,   314,   315,   316,     0,     0,     0,   317,   553,
     554,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   261,   262,     0,   263,   264,     0,   555,   265,   266,
     267,   268,     0,    86,    87,     0,    88,   183,    90,   322,
       0,   323,     0,     0,   324,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   271,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   273,   274,   275,   276,
     277,   278,   279,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,    48,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
       0,     0,     0,     0,  1230,   314,   315,   316,     0,     0,
       0,   317,   553,   554,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   261,   262,     0,   263,   264,     0,
     555,   265,   266,   267,   268,     0,    86,    87,     0,    88,
     183,    90,   322,     0,   323,     0,     0,   324,   269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   271,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   273,
     274,   275,   276,   277,   278,   279,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
      48,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,     0,     0,     0,     0,     0,   314,   315,
     316,     0,     0,     0,   317,   553,   554,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   555,     0,     0,     0,     0,     0,    86,
      87,     0,    88,   183,    90,   322,     0,   323,     0,     0,
     324,     0,     0,     0,     0,     0,     0,     0,     0,   427,
     428,   429,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,     0,   453,   427,   428,   429,     0,     0,     0,
       0,     0,     0,     0,     0,   454,     0,     0,     0,     0,
       0,     0,     0,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,     0,   453,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
     454,     0,     0,     0,     0,     0,     0,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,     0,   453,   427,   428,   429,     0,     0,     0,
       0,     0,     0,     0,     0,   454,     0,     0,     0,     0,
       0,     0,     0,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,     0,   453,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
     454,   455,     0,     0,     0,     0,     0,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,     0,   453,   427,   428,   429,     0,     0,     0,
       0,     0,     0,     0,     0,   454,   540,     0,     0,     0,
       0,     0,     0,   430,     0,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,     0,   453,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
     454,   542,     0,     0,     0,     0,     0,     0,   430,     0,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,     0,   453,  1046,  1047,  1048,     0,     0,     0,
       0,     0,     0,     0,     0,   454,   560,     0,     0,     0,
       0,     0,     0,  1049,     0,  1050,  1051,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,   427,   428,   429,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1072,   584,     0,     0,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,   270,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   454,     0,   770,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   272,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
     792,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,   272,     0,     0,     0,     0,
    -398,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   178,   179,   417,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,  1229,     0,   543,   544,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,   270,     0,     0,     0,   182,     0,     0,    84,   321,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
       0,     0,     0,  1083,     0,     0,     0,     0,   272,   325,
       0,     0,     0,   543,   544,   418,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      36,   182,     0,   326,    84,   321,     0,    86,    87,     0,
      88,   183,    90,     0,  1028,     0,     0,     0,     0,   270,
       0,    48,     0,     0,     0,   325,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   272,     0,     0,   326,
       0,     0,     0,     0,     0,     0,   543,   544,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,   182,     0,     0,    84,   321,     0,
      86,    87,     0,    88,   183,    90,     0,  1363,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,   325,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   326,     0,   543,   544,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,   182,   211,     0,    84,   321,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,   325,     0,     0,     0,
    1238,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   821,   822,     0,
     326,     0,     0,   823,     0,   824,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   825,     0,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,   754,
       0,    86,    87,     0,    88,   183,    90,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,     0,     0,     0,   787,     0,   116,     0,
       0,     0,     0,     0,   826,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,   182,    82,    83,    84,   827,     0,    86,    87,
       0,    88,   183,    90,     0,  1007,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,   828,     0,     0,     0,
       0,    97,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    28,     0,     0,
     829,     0,     0,     0,     0,    33,    34,    35,    36,     0,
     211,     0,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,  1475,     0,     0,
       0,     0,     0,  1008,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,    97,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    48,     0,     0,
     215,     0,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   821,   822,     0,     0,  1476,
       0,   823,     0,   824,     0,     0,     0,     0,     0,     0,
       0,     0,  1477,  1478,     0,   825,     0,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,     0,     0,     0,
     182,     0,     0,    84,  1479,   212,    86,    87,     0,    88,
    1480,    90,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,     0,     0,
       0,     0,   826,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    82,    83,    84,   827,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,     0,     0,
       0,     0,     0,     0,   828,   974,   975,     0,     0,    97,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   976,     0,     0,   829,     0,
       0,     0,     0,   977,   978,   979,    36,   427,   428,   429,
       0,     0,     0,     0,     0,   980,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   430,    48,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,     0,     0,     0,     0,     0,     0,     0,    36,
       0,     0,   981,   454,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   982,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   983,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    28,     0,     0,     0,   502,
       0,     0,     0,    33,    34,    35,    36,  1643,   211,    86,
      87,  1644,    88,   183,    90,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   213,     0,
       0,  1490,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,    28,     0,   215,     0,
       0,   597,     0,   116,    33,    34,    35,    36,     0,   211,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   617,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,    28,   215,
     963,     0,     0,     0,   116,     0,    33,    34,    35,    36,
       0,   211,     0,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   213,   453,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   454,    73,     0,    74,    75,    76,
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
       0,     0,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1108,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    92,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
      28,     0,   215,     0,     0,     0,     0,   116,    33,    34,
      35,    36,     0,   211,     0,     0,     0,     0,     0,     0,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,   213,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1072,    73,     0,    74,
      75,    76,    77,    78,     0,     0,    36,     0,     0,     0,
     214,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,    48,     0,     0,
      92,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,   215,    33,    34,    35,    36,   116,   211,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
     182,     0,     0,    84,     0,     0,    86,    87,    48,    88,
     183,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   229,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    74,    75,    76,    77,    78,
       0,     0,  1734,     0,     0,     0,   214,     0,     0,     0,
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,     0,     0,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,   230,
      33,    34,    35,    36,   116,   211,     0,     0,     0,     0,
       0,     0,   640,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,     0,
       0,     0,    92,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   427,   428,   429,   641,     0,     0,     0,     0,
     642,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   430,     0,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,     0,   453,   427,   428,   429,
       0,     0,     0,     0,     0,     0,     0,     0,   454,     0,
       0,     0,     0,     0,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   454,   427,   428,   429,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   430,   511,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,     0,   453,   427,
     428,   429,     0,     0,     0,     0,     0,     0,     0,     0,
     454,     0,     0,     0,     0,     0,     0,     0,   430,   949,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,     0,   453,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   454,  1046,  1047,  1048,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1049,   993,  1050,  1051,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,     0,
       0,  1046,  1047,  1048,     0,     0,     0,     0,     0,     0,
       0,     0,  1072,     0,     0,     0,     0,     0,     0,     0,
    1049,  1316,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1072,  1046,  1047,
    1048,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1049,  1220,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,     0,     0,  1046,  1047,  1048,     0,     0,     0,     0,
       0,     0,     0,     0,  1072,     0,     0,     0,     0,     0,
       0,     0,  1049,  1373,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1072,
       0,    36,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1455,     0,    48,     0,     0,     0,     0,     0,     0,  1476,
     344,   345,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1477,  1478,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     182,     0,    36,    84,    85,  1541,    86,    87,    48,    88,
    1480,    90,     0,     0,     0,     0,     0,     0,   346,     0,
       0,    86,    87,    48,    88,   183,    90,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    36,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      36,   182,     0,     0,    84,    85,    48,    86,    87,     0,
      88,   183,    90,     0,     0,     0,     0,     0,     0,     0,
     414,    48,    86,    87,     0,    88,   183,    90,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    36,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   585,     0,     0,    86,    87,    48,    88,   183,
      90,     0,     0,     0,     0,     0,     0,   589,     0,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,     0,   346,     0,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   427,   428,   429,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   804,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,   427,   428,   429,     0,     0,     0,     0,     0,
       0,     0,     0,   454,     0,     0,     0,     0,     0,     0,
       0,   430,   946,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   805,   453,   427,   428,   429,
       0,     0,     0,     0,     0,     0,     0,     0,   454,     0,
       0,     0,     0,     0,     0,     0,   430,     0,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
       0,   453,  1046,  1047,  1048,     0,     0,     0,     0,     0,
       0,     0,     0,   454,     0,     0,     0,     0,     0,     0,
       0,  1049,  1378,  1050,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1046,  1047,  1048,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1072,     0,
       0,     0,     0,     0,  1049,     0,  1050,  1051,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1072,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,     0,   453,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   454,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1072,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,     0,   453,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   454,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1072
};

static const yytype_int16 yycheck[] =
{
       5,     6,   135,     8,     9,    10,    11,    12,   334,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     4,    32,    28,    29,    91,   185,   526,   160,    95,
      96,   385,     4,   227,    44,   681,  1132,    42,   174,    49,
       4,   653,     4,   652,    55,    50,   772,    52,   231,   922,
      55,   469,    57,   186,    30,   165,   113,    30,   453,   911,
     160,   803,   486,    42,   496,  1117,     4,   633,   134,  1128,
     486,   490,   491,    58,    79,   810,   240,   733,   942,  1006,
     113,     9,   581,     9,   113,     9,    30,     9,    46,     9,
      14,     9,    14,     9,   958,     9,    81,    54,   522,    84,
      79,   520,     9,     9,    46,     9,   522,     9,   113,     9,
      14,     9,    46,     9,     9,    30,     9,     4,     9,    30,
       9,     9,    67,     9,    67,     9,    33,     9,     9,     9,
       9,   241,    67,     9,   460,    67,    46,   111,    46,     9,
      80,     4,   102,   103,   130,   131,    67,    86,  1672,   102,
     103,     9,   172,    14,    67,   654,   172,    51,   215,    46,
     172,    80,     9,   172,   172,   101,   102,   103,     9,    30,
       0,   130,   131,   230,    47,    48,    49,    50,    51,    52,
     200,    54,   215,   509,   200,   190,   215,    48,   200,   119,
    1042,   200,   200,    66,   200,   169,   126,     4,   130,   131,
      67,   230,   151,   200,  1728,    67,   370,    67,    67,   148,
     215,   151,    67,    67,   200,    67,    67,   130,   131,    67,
     200,   108,    67,   172,    35,   230,   113,     8,   115,   116,
     117,   118,   119,   120,   121,    67,    67,   242,    67,    67,
     245,    80,    80,   201,   204,   204,   165,   252,   253,   385,
     172,   204,  1311,    80,   203,   200,   198,   200,   203,   201,
     419,  1313,   156,    67,   198,   200,   201,   173,  1320,    80,
    1322,   158,   159,   201,   161,  1202,   202,   203,   202,   200,
     202,  1016,   202,  1018,   202,    51,   202,   202,   202,    80,
     203,   201,   203,   201,   201,   182,   795,   160,   202,  1163,
     202,   800,   202,   151,   202,   362,   202,   202,   246,   202,
     201,   505,   250,   202,   202,   173,   202,   204,   202,   201,
     201,   201,   201,    86,   460,   201,   173,   939,   135,   362,
    1182,   201,   173,   362,   900,   468,   203,   203,   200,   498,
     200,   203,   151,   203,   203,   200,   151,   413,   203,   203,
     486,   203,   203,    98,   165,   203,   361,   362,   203,   344,
     345,   346,  1421,   368,  1423,   157,   205,   205,   373,    35,
      35,   203,   203,   509,   203,   203,    80,   334,   205,   186,
    1432,   151,   361,   519,   200,   148,   522,   101,   102,   103,
     156,   200,   377,   398,    80,   200,    80,   463,   464,   465,
     466,    54,   172,    35,   409,   171,   197,   200,   391,    54,
     155,   203,   203,    66,    80,    80,   115,   116,   117,  1387,
     200,    80,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,    80,   454,
     644,   456,   457,   410,   459,   887,  1198,  1183,   469,  1115,
     200,   165,  1521,     4,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   453,   200,   165,
     453,   165,   487,   488,   156,   490,   491,   492,   493,   148,
     149,   150,    98,   200,   499,   659,   660,   502,   200,   165,
     165,    35,    98,   460,   928,  1473,   511,    67,   513,   453,
      51,    80,   928,    54,  1013,   520,   460,   528,  1370,   155,
     130,   131,  1131,   528,   200,   530,    98,    35,   722,  1497,
      71,  1499,    98,   165,   397,  1147,   955,   669,  1150,    98,
     200,    80,   486,   726,   200,    80,    80,    86,    89,   155,
      91,    86,   509,   203,    95,    96,    30,   911,   563,   155,
     155,   566,   567,   568,    29,   509,   202,   203,   963,   669,
     130,   131,    80,    80,   157,   519,   735,   760,   522,    86,
     149,   150,    47,   155,   641,    50,   151,   170,  1440,   155,
     397,   151,   597,   134,   118,   533,   155,   117,   118,   119,
     585,   200,   126,  1329,   589,  1331,     4,   172,    35,   594,
     149,   150,   776,   777,   149,   150,  1675,   202,   782,   783,
     203,   687,   202,   817,   202,   270,   202,   272,    26,    27,
      67,    80,    75,    76,    77,   200,   641,    86,   203,   202,
     200,    29,   149,   150,    87,    47,    48,   830,   202,   115,
     116,   117,   118,   119,   120,   838,    67,   198,    80,    47,
     467,   203,    50,   151,    86,  1321,   202,   107,   200,   208,
     653,   200,   782,   151,   679,   115,   116,   117,   118,   119,
     120,   326,    50,    51,    52,    67,   691,   200,  1042,    26,
      27,  1300,  1110,   136,   137,   138,   139,   140,    66,   148,
     149,   150,   209,  1315,   147,   246,   202,    72,    73,   250,
     153,   154,  1438,   254,    80,   113,   182,    80,  1814,   724,
      86,    72,    73,    86,   167,   155,   148,   149,   150,    98,
      99,   736,    45,  1829,   128,   129,   202,  1156,   181,   202,
     203,   457,   182,   202,   203,    66,  1402,   172,  1167,  1808,
     755,  1702,  1703,  1698,  1699,   115,   116,   117,   118,   119,
     120,   151,   407,   207,  1823,   410,   126,   127,   200,  1201,
       9,   487,  1271,   819,   820,   911,   492,   200,   914,   151,
     151,   200,   787,   149,   150,     8,   149,   150,   202,   799,
     926,   200,   928,   334,   988,   151,    14,    80,   803,    50,
      51,    52,  1528,    54,   164,   151,   669,   202,   115,   116,
     117,   118,   119,   120,   126,    66,   202,   215,  1474,    14,
     126,  1433,   182,   201,   172,    14,   224,    98,  1182,   183,
     184,   185,   230,   206,   201,   840,   190,   191,  1032,   201,
     194,   195,   201,  1342,   107,  1039,     9,   200,   246,   371,
    1282,   200,   250,   375,   148,   200,   397,   201,   994,  1358,
     201,   201,   201,    90,     9,   406,   202,    14,   172,   410,
     808,   200,   413,     9,   859,   182,   186,   399,   863,   401,
     402,   403,   404,    80,    80,   200,    80,   224,   189,     9,
       9,   896,    80,   202,   202,   128,   962,   202,  1317,   201,
      67,   201,   547,   201,   200,   910,  1042,    30,   201,  1782,
      47,    48,    49,    50,    51,    52,   899,   129,   171,   460,
     461,   462,   463,   464,   465,   466,   151,   899,  1801,    66,
     793,   936,   132,     9,   201,   899,  1809,   899,   151,   922,
      14,   946,   198,  1442,   949,   486,   951,     9,     9,   173,
     955,  1677,  1451,   351,   201,  1138,   939,   914,     9,    14,
     128,   899,   360,  1462,   362,   207,   207,   204,   509,   367,
     914,     9,    14,   207,   151,   201,   374,   207,    87,   201,
     200,   522,   926,  1177,   928,   201,   793,   963,   993,   999,
     963,   854,   533,    98,   202,   202,   151,   132,     9,   397,
    1419,   151,   201,   151,   649,   650,   186,   200,   200,    14,
      78,   552,   203,   658,   351,   186,  1370,     9,   203,   963,
      80,    14,   203,   360,   202,   362,    14,   203,   207,    14,
     367,    99,   201,  1227,    30,  1534,   899,   374,   202,   198,
    1234,    30,   200,   200,    14,    14,  1182,   854,   200,   200,
     994,    49,     9,   202,   200,   596,   124,   920,   200,   202,
     201,   200,  1000,   132,    14,   132,     9,   201,     9,    66,
      80,  1076,  1077,  1078,   207,     9,  1722,  1082,  1083,   200,
     132,   622,   623,    14,  1041,   202,  1440,  1044,   201,    80,
     158,   159,   899,   161,   162,   163,   203,   200,   496,  1110,
     200,   203,   201,     4,   202,  1110,   203,   132,    87,     9,
      30,   207,   148,   920,    74,   202,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   201,
     173,  1114,   200,   132,    30,   533,   202,   201,  1143,  1002,
       9,  1004,  1114,   201,  1338,    46,   687,   201,   132,     9,
    1114,  1156,  1114,  1799,   204,   201,   204,    14,   203,   496,
     200,    80,  1167,  1168,  1147,   202,   201,  1150,  1667,   201,
    1669,   201,   203,   200,   819,   820,  1114,   201,   201,  1678,
    1190,   132,   201,     9,    30,   202,   201,   201,   160,   202,
     202,   108,   156,  1198,   203,  1002,   202,  1004,    14,    80,
     113,   201,   203,  1208,   132,   132,   201,   108,    14,   172,
      80,   201,   113,    14,   115,   116,   117,   118,   119,   120,
     121,   203,   202,    14,    80,   201,  1725,   200,   203,   132,
     201,   772,    14,   774,  1370,   202,   202,    14,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,  1114,   793,   202,    14,  1193,   901,   158,   159,   203,
     161,     9,   204,    56,   172,  1398,   807,   808,    80,    80,
     200,  1403,   917,     9,   111,    80,   202,    98,   151,    98,
     678,   182,    33,   163,    14,   930,   200,  1292,    64,    65,
     201,  1296,   202,  1298,   200,   169,   173,  1280,   166,    80,
       9,  1306,   201,   204,  1440,    80,  1289,  1114,   202,   201,
     201,  1316,  1317,   854,   959,   203,    14,    80,    14,   860,
      14,    80,    80,   864,   865,    14,  1652,    80,  1827,   594,
     863,   859,  1315,  1790,   466,  1834,  1495,   461,   463,   737,
     961,   678,  1205,   884,   956,   902,  1805,  1533,  1355,  1199,
    1801,   599,  1524,  1557,   130,   131,  1475,  1641,   899,  1393,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,   914,  1379,  1020,  1821,  1833,  1023,   920,
     778,  1653,   780,  1520,   469,   926,    66,   928,  1389,  1085,
    1082,   851,  1118,  1165,  1040,   793,   570,   570,  1205,  1178,
     737,  1467,   975,  1179,   990,   926,   368,   805,   410,   819,
     808,  1416,  1382,  1757,  1419,  1073,  1101,  1280,  1025,    -1,
      -1,   962,    -1,    -1,    -1,   201,  1289,    -1,    -1,    -1,
      -1,    -1,    -1,   974,   975,   976,    -1,    -1,    -1,  1422,
      -1,   778,    -1,   780,    -1,  1428,    -1,  1430,    -1,    -1,
    1433,    -1,    -1,   994,    -1,    -1,   854,    -1,    -1,  1000,
      -1,  1002,    -1,  1004,    -1,    -1,    -1,    -1,   805,    -1,
      -1,    -1,    -1,    -1,   872,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1024,    -1,  1130,    -1,  1132,    -1,   887,
     888,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1657,  1693,
    1041,   899,    -1,  1044,    -1,    -1,    -1,  1512,    -1,    -1,
    1448,    -1,    -1,  1470,  1159,    -1,    -1,  1162,  1381,    -1,
      -1,    -1,   920,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1073,    -1,    -1,   872,    -1,  1542,    -1,    -1,
    1403,    -1,    -1,  1526,    -1,    -1,    -1,    -1,    -1,  1090,
     887,   888,  1093,    -1,    -1,    -1,    -1,    -1,    -1,  1422,
      -1,    -1,    -1,    -1,    -1,  1428,    -1,  1430,  1213,    -1,
      -1,    -1,  1217,  1114,  1381,    -1,    -1,    -1,    -1,    -1,
    1443,    -1,    -1,    -1,    -1,    -1,    -1,    26,    27,  1452,
    1397,    30,    -1,    -1,  1532,  1533,    -1,    -1,    -1,    -1,
      -1,    -1,  1000,    -1,  1002,    -1,  1004,    -1,  1006,  1007,
      -1,    -1,    -1,    -1,    -1,    54,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,  1276,  1277,    -1,    -1,    -1,  1443,    -1,  1179,    -1,
      -1,    -1,  1183,     4,  1776,  1452,    -1,    -1,    -1,    -1,
    1716,    -1,  1193,    -1,    -1,    -1,    -1,  1662,    -1,    -1,
      -1,  1794,    -1,  1526,  1205,    -1,    64,    65,  1531,  1006,
    1007,    -1,    -1,    -1,  1537,    -1,    -1,    -1,    -1,    -1,
    1543,    -1,    -1,    -1,    -1,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1652,  1503,    -1,    -1,    -1,
      26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1114,  1362,    -1,  1364,
      -1,    -1,    -1,    -1,  1531,    -1,    -1,    -1,    -1,    -1,
    1537,    -1,   130,   131,  1275,    -1,  1543,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1142,    -1,    -1,   108,    -1,    -1,
      -1,    -1,   113,  1398,   115,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   224,    -1,    -1,  1329,  1187,
    1331,    -1,    -1,    -1,    -1,  1193,    -1,   158,   159,  1782,
     161,    -1,    -1,  1201,  1202,  1142,    -1,  1205,    -1,    -1,
      -1,    -1,    26,    27,    64,    65,    -1,    -1,  1801,  1682,
      -1,   182,    -1,    -1,    -1,    -1,  1809,    -1,    -1,    -1,
      -1,   270,    -1,   272,  1839,    -1,    -1,    -1,    -1,    -1,
    1381,    -1,  1847,   204,    -1,    -1,  1387,    -1,  1853,    -1,
    1187,  1856,  1393,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1723,  1724,    -1,    -1,  1201,  1202,    -1,  1730,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1682,    -1,    -1,    -1,    -1,
     130,   131,    -1,    -1,  1282,    -1,    -1,   326,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1438,   224,    -1,
      -1,    -1,  1443,    -1,    -1,  1768,    -1,  1448,    -1,    -1,
    1555,  1452,   351,  1776,    -1,    -1,  1723,  1724,    -1,    -1,
      -1,   360,    -1,  1730,    -1,    -1,  1467,    -1,   367,  1470,
      -1,    -1,  1473,    -1,    -1,   374,    -1,    -1,    -1,    -1,
      -1,    -1,  1483,    -1,    -1,  1282,   385,    -1,    -1,  1490,
      -1,   201,    -1,    -1,    -1,    -1,  1497,    -1,  1499,    -1,
      -1,  1768,    -1,    -1,  1505,     4,    -1,    -1,   407,    -1,
      -1,   410,  1835,    -1,    -1,    -1,    -1,    -1,    -1,  1842,
      -1,    -1,    -1,  1381,    -1,    -1,    -1,  1528,    -1,    -1,
    1531,  1532,  1533,    -1,    -1,    -1,  1537,    -1,    -1,    -1,
      -1,    -1,  1543,    -1,    -1,    -1,    -1,    46,    -1,    -1,
     224,  1656,    -1,    -1,   453,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   351,    -1,    -1,  1835,    -1,
      -1,    -1,    -1,    -1,   360,  1842,    -1,    -1,    -1,    -1,
      -1,   367,    -1,    -1,    -1,  1443,    -1,    -1,   374,    -1,
    1448,    -1,    -1,    -1,  1452,    -1,    -1,   496,    -1,   385,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
      -1,    -1,    -1,    -1,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,     4,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1740,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   547,   548,
      -1,  1652,   551,    -1,    -1,    -1,    -1,    26,    27,   158,
     159,    30,   161,    -1,    -1,    46,    -1,   453,    -1,    -1,
      -1,    -1,  1673,  1531,  1532,  1533,  1677,   351,    -1,  1537,
      -1,  1682,    -1,   182,    -1,  1543,   360,    -1,    -1,    -1,
    1691,     4,    -1,   367,    -1,    -1,    -1,  1698,  1699,    -1,
     374,  1702,  1703,    -1,   603,   204,    -1,    -1,    -1,  1814,
     496,    -1,    -1,    -1,    -1,  1716,    -1,    -1,    -1,    -1,
      -1,    -1,  1723,  1724,  1829,    -1,    -1,   108,    -1,  1730,
      -1,    -1,   113,    46,   115,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     649,   650,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   658,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1768,    -1,    -1,
      -1,    -1,    -1,    -1,  1775,    -1,    -1,   158,   159,   678,
     161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1793,    -1,    -1,   108,    -1,    -1,    -1,    -1,
     113,   182,   115,   116,   117,   118,   119,   120,   121,   115,
     116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,
     126,   127,   496,   204,  1682,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1835,    -1,    -1,    -1,   737,    -1,
      -1,  1842,    -1,    -1,    -1,   158,   159,    -1,   161,    -1,
      -1,    -1,    -1,    -1,    -1,   224,   162,    -1,   164,    -1,
      -1,    -1,    -1,    -1,    -1,  1723,  1724,    -1,    -1,   182,
      -1,   177,  1730,   179,    -1,    -1,   182,    -1,    -1,   778,
      -1,   780,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   204,   678,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,   805,   806,    -1,    -1,
    1768,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     819,   820,   821,   822,   823,   824,   825,    -1,    -1,    -1,
     829,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    -1,    -1,    64,    65,    -1,    -1,    -1,    -1,    -1,
      -1,   737,    -1,    -1,    -1,    -1,   855,    -1,    -1,    -1,
      78,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,   872,    -1,    -1,    -1,  1835,    -1,    -1,
      -1,    99,   351,    -1,  1842,    -1,   885,    -1,   887,   888,
      -1,   360,   778,    -1,   780,    -1,    -1,    -1,   367,    -1,
      -1,    -1,   901,   902,   678,   374,    -1,    -1,    -1,   130,
     131,    -1,   911,    -1,    -1,    -1,   385,   155,   917,   805,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,    -1,
      -1,   930,    -1,    -1,   152,    -1,    -1,   155,    -1,   938,
     158,   159,   941,   161,   162,   163,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
     959,    -1,    -1,   737,   963,   203,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
     201,    -1,    -1,    -1,   453,    -1,   872,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,   887,   888,    -1,   778,    -1,   780,  1006,  1007,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1020,    -1,    -1,  1023,   911,  1025,   496,    -1,    -1,
      -1,   805,    -1,    -1,    -1,    -1,    -1,    64,    65,    -1,
      -1,  1040,    -1,  1042,    -1,    -1,  1045,  1046,  1047,  1048,
    1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,    -1,    -1,    -1,   963,    -1,    -1,
      -1,    -1,   551,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,   872,  1098,
      -1,    -1,    -1,   130,   131,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,   887,   888,    -1,    -1,    -1,    -1,    -1,
    1006,  1007,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1130,    -1,  1132,   603,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1042,    -1,    -1,    -1,
    1159,    -1,    54,  1162,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,  1182,    54,    -1,    -1,    -1,  1187,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,  1201,  1202,    -1,  1204,    -1,    -1,    -1,   678,
      -1,    -1,    -1,    -1,  1213,    -1,    -1,    -1,  1217,    -1,
      -1,  1220,    -1,  1222,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1006,  1007,    -1,    -1,    -1,    -1,    -1,  1238,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   737,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1276,  1277,    -1,
    1279,    -1,    -1,  1282,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1182,    -1,    -1,    -1,
      -1,  1187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   778,
      -1,   780,    -1,    -1,    -1,  1201,  1202,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,   805,   806,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,   821,   822,   823,   824,   825,    -1,    -1,    -1,
     829,    -1,    -1,  1362,    -1,  1364,    -1,    -1,  1142,    -1,
    1369,  1370,    -1,   551,  1373,    -1,  1375,    -1,   270,  1378,
     272,    -1,    -1,    -1,    -1,    -1,   855,    -1,  1387,  1388,
      -1,    -1,  1391,    -1,    -1,    -1,  1282,    -1,    -1,  1398,
      -1,    -1,    -1,   872,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1187,    -1,    -1,   885,    -1,   887,   888,
      -1,    -1,    -1,    -1,    -1,   603,    -1,  1201,  1202,    -1,
      -1,    -1,    -1,   902,   326,    -1,    -1,    -1,    -1,    -1,
      -1,  1440,   911,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1455,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1465,  1466,    -1,   938,
      -1,    -1,   941,    -1,  1473,    -1,  1475,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1370,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   963,    -1,    -1,    -1,  1497,    -1,
    1499,    -1,    -1,    -1,    -1,    -1,  1505,    -1,  1282,    -1,
      -1,    -1,    -1,    -1,    -1,   407,    -1,    -1,   410,    -1,
      -1,    75,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1006,  1007,    -1,
      -1,  1540,  1541,    -1,    -1,    99,    -1,    -1,  1547,    -1,
    1549,    -1,    -1,    -1,  1440,    -1,  1555,    -1,  1557,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1040,    -1,  1042,    -1,    -1,  1045,  1046,  1047,  1048,
    1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   806,  1098,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   821,   822,   823,   824,    -1,    -1,    -1,
      -1,   829,    -1,    -1,    -1,   547,   548,  1656,    -1,   551,
      -1,    -1,    -1,    -1,    26,    27,    -1,    -1,    30,    -1,
      -1,    -1,    -1,  1142,  1673,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,  1691,    29,    -1,    -1,    -1,    -1,  1697,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,  1708,
      -1,   603,    -1,  1182,    -1,  1714,    -1,    -1,  1187,  1718,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1201,  1202,    -1,  1204,    -1,    -1,    -1,    -1,
      -1,  1740,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1220,    -1,  1222,    -1,    -1,    -1,   649,   650,    -1,
     938,    -1,    -1,    99,    -1,    -1,   658,    -1,    -1,  1238,
      -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1780,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1789,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
      -1,    -1,    -1,    -1,    -1,    -1,  1805,    -1,    -1,    -1,
    1279,    -1,    -1,  1282,    -1,  1814,   152,    -1,    -1,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
    1829,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,   224,    -1,   200,    -1,    -1,  1045,  1046,  1047,
    1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,  1072,    -1,    -1,    -1,    -1,    -1,
    1369,  1370,    -1,    -1,  1373,    -1,  1375,    -1,    -1,  1378,
      -1,    -1,    -1,    -1,   806,    -1,    -1,    -1,    -1,  1388,
    1098,    -1,  1391,    10,    11,    12,    -1,   819,   820,   821,
     822,   823,   824,   825,    -1,    -1,    -1,   829,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,  1440,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,  1455,    -1,    -1,   351,
      -1,    -1,    -1,    -1,    -1,    -1,  1465,  1466,   360,    -1,
      -1,    -1,    -1,    -1,    -1,   367,  1475,    -1,    -1,   901,
      -1,    -1,   374,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   385,    -1,   917,  1204,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   930,    -1,
      -1,    -1,  1220,    -1,  1222,    -1,   938,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1238,    -1,    -1,    -1,    -1,    -1,    -1,   959,    -1,    -1,
      -1,  1540,  1541,    -1,    10,    11,    12,    -1,  1547,    -1,
    1549,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1557,    -1,
      -1,   453,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
     207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1020,    -1,
      66,  1023,    -1,  1025,   496,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1040,    -1,
      -1,    -1,    -1,  1045,  1046,  1047,  1048,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   551,
      -1,  1369,    -1,    -1,    -1,  1373,    -1,  1375,    -1,    -1,
    1378,    -1,    -1,    -1,    -1,    -1,  1098,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,  1697,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,  1130,  1708,
    1132,   603,    -1,    -1,    -1,  1714,    -1,    -1,    -1,  1718,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1159,   204,    -1,
    1162,    -1,    -1,    -1,    -1,    -1,    29,  1455,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1780,  1204,    66,    -1,    -1,   678,    -1,    -1,    -1,
    1789,  1213,    -1,    -1,    -1,  1217,    -1,    -1,  1220,    -1,
    1222,    -1,    -1,    -1,    -1,    -1,  1805,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1238,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1540,  1541,    -1,    -1,    -1,    -1,    -1,  1547,
      -1,    -1,    -1,    -1,    -1,   737,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1276,  1277,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   551,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   778,    29,   780,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,   805,   806,    -1,    -1,   603,    -1,    -1,
      -1,   204,    -1,    -1,    66,    -1,    -1,    -1,    -1,   821,
     822,   823,   824,   825,    -1,    -1,    -1,   829,    -1,    -1,
    1362,    -1,  1364,    -1,    -1,    -1,    -1,  1369,    -1,    -1,
      -1,  1373,    -1,  1375,    -1,    -1,  1378,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1387,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1398,    -1,    -1,    -1,
     872,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1697,
      -1,    -1,    -1,    -1,    -1,   887,   888,    -1,    -1,    -1,
    1708,    -1,    -1,    -1,    -1,    -1,  1714,    -1,    -1,    -1,
    1718,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   911,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1455,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   938,    -1,    -1,    -1,
      -1,  1473,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   204,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,   963,  1780,    -1,    -1,  1497,    -1,  1499,    -1,    -1,
      -1,    -1,    -1,  1505,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    78,    54,
      80,    -1,    -1,    -1,  1006,  1007,    -1,    -1,  1540,  1541,
     806,    66,    -1,    -1,    -1,  1547,    -1,    -1,    -1,    99,
      -1,    -1,    -1,  1555,    -1,   821,   822,   823,   824,   825,
      -1,    -1,    -1,   829,    -1,    -1,    -1,    -1,  1040,    -1,
    1042,    -1,    -1,  1045,  1046,  1047,  1048,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,    -1,    -1,    -1,    -1,    -1,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1098,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,    -1,
      -1,    -1,    -1,   203,  1656,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1142,  1673,   938,    -1,    -1,    -1,    -1,    -1,    -1,   204,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1691,
      -1,    -1,    -1,    -1,    -1,  1697,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1708,    -1,    -1,    -1,
    1182,    -1,  1714,    -1,    -1,  1187,  1718,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1201,
    1202,    -1,  1204,    -1,    -1,    -1,    -1,    -1,  1740,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1220,    -1,
    1222,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1238,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1040,    -1,    -1,    -1,  1780,  1045,
    1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,    -1,    -1,    -1,
    1282,    -1,  1814,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1829,    -1,    -1,
      -1,    -1,  1098,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    80,    81,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,  1369,  1370,    -1,
      99,  1373,    -1,  1375,    -1,    -1,  1378,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,     3,     4,    -1,     6,     7,  1204,    -1,
      10,    11,    12,    13,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1220,    -1,  1222,    27,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,  1440,    -1,
      -1,    -1,  1238,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1455,    54,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    68,    69,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,   204,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,    -1,    -1,    -1,    -1,   126,   127,   128,   129,
      -1,    -1,    -1,   133,   134,   135,    -1,    -1,  1540,  1541,
      -1,    -1,    -1,    -1,    -1,  1547,    -1,    -1,    -1,    -1,
     202,    -1,   152,    -1,  1556,    -1,    -1,    -1,   158,   159,
      -1,   161,   162,   163,   164,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,  1369,    -1,    -1,    -1,  1373,    -1,  1375,
      -1,    -1,  1378,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    10,    11,    12,
      -1,    -1,    -1,   203,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,  1455,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1697,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1708,    -1,    -1,    -1,
      -1,    -1,  1714,    -1,    -1,    -1,  1718,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
    1742,    -1,    -1,    -1,  1540,  1541,    -1,    -1,    -1,    -1,
      29,  1547,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,  1780,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    66,   204,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,
      53,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    -1,
      -1,  1697,    75,    76,    77,    78,    79,    80,    -1,    82,
      83,    -1,  1708,    -1,    87,    88,    89,    90,  1714,    92,
      -1,    94,  1718,    96,    -1,   204,    99,   100,    -1,    -1,
      -1,   104,   105,   106,   107,   108,   109,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,   144,    -1,   204,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,  1780,   158,   159,    -1,   161,   162,
     163,   164,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,
      -1,    -1,    -1,   176,   177,    -1,   179,    -1,   181,   182,
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
     106,   107,   108,   109,   110,    -1,   112,   113,   114,   115,
     116,   117,   118,   119,   120,    -1,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    -1,
      -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,
     176,   177,    -1,   179,    -1,   181,   182,    -1,   184,   185,
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
      99,   100,    -1,    -1,    -1,   104,   105,   106,   107,   108,
     109,   110,    -1,   112,   113,   114,   115,   116,   117,   118,
     119,   120,    -1,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,
      -1,   170,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,
     179,    -1,   181,   182,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,   202,   203,    -1,   205,   206,    -1,   208,
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
      -1,    -1,    -1,    87,    88,    89,    90,    91,    92,    -1,
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
      97,    -1,    99,   100,    -1,    -1,    -1,   104,   105,   106,
     107,    -1,   109,   110,    -1,   112,    -1,   114,   115,   116,
     117,   118,   119,   120,    -1,   122,   123,   124,    -1,   126,
     127,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,   164,    -1,    -1,
     167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,   202,   203,    -1,   205,   206,
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
      -1,    87,    88,    89,    90,    -1,    92,    -1,    94,    95,
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
      -1,    -1,    87,    88,    89,    90,    -1,    92,    93,    94,
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
     197,    -1,    -1,   200,    -1,   202,   203,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,   202,   203,    -1,   205,   206,    -1,   208,   209,
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
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
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
      28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,
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
      -1,    -1,    -1,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    47,    48,    -1,    -1,    -1,    -1,    53,
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
      -1,    -1,    -1,    13,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    47,    48,    -1,
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
      -1,   127,   128,   129,    -1,    -1,    -1,   133,   134,   135,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    10,    11,    12,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    29,   200,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      66,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    68,
      69,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   204,    -1,
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
     189,   190,   191,   192,   193,   194,   195,   196,    29,    -1,
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
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,   202,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,   201,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    56,    -1,    -1,    -1,    -1,
     107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,   116,
     117,   118,   119,   120,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   201,    -1,   134,   135,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    29,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
      -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,    56,   176,
      -1,    -1,    -1,   134,   135,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      78,   152,    -1,   200,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,   165,    -1,    -1,    -1,    -1,    29,
      -1,    99,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    56,    -1,    -1,   200,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,   165,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,   200,    -1,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    78,   152,    80,    -1,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      30,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    47,    48,    -1,
     200,    -1,    -1,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,   156,
      -1,   158,   159,    -1,   161,   162,   163,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,   205,    -1,
      -1,    -1,    -1,    -1,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    35,    -1,   167,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,   181,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    67,    -1,    -1,
     200,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      -1,   181,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    99,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    -1,   121,
      -1,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,
     152,    -1,    -1,   155,   156,    87,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   176,    47,    48,    -1,    -1,   181,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    67,    -1,    -1,   200,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    99,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    -1,   134,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,    -1,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    67,    -1,    -1,    -1,   132,
      -1,    -1,    -1,    75,    76,    77,    78,   156,    80,   158,
     159,   160,   161,   162,   163,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   120,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    67,    -1,   200,    -1,
      -1,   203,    -1,   205,    75,    76,    77,    78,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    67,   200,
      69,    -1,    -1,    -1,   205,    -1,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,   120,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,   134,    -1,   136,   137,   138,
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
      -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      67,    -1,   200,    -1,    -1,    -1,    -1,   205,    75,    76,
      77,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    78,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    99,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,    -1,   200,    75,    76,    77,    78,   205,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
     152,    -1,    -1,   155,    -1,    -1,   158,   159,    99,   161,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   136,   137,   138,   139,   140,
      -1,    -1,   204,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    -1,   200,
      75,    76,    77,    78,   205,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    10,    11,    12,   200,    -1,    -1,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
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
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
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
      52,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   132,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,   121,
     107,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   134,   135,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     152,    -1,    78,   155,   156,   132,   158,   159,    99,   161,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,   155,    -1,
      -1,   158,   159,    99,   161,   162,   163,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    78,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      78,   152,    -1,    -1,   155,   156,    99,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    99,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    78,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,   155,    -1,    -1,   158,   159,    99,   161,   162,
     163,    -1,    -1,    -1,    -1,    -1,    -1,   155,    -1,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,    -1,    -1,   155,    -1,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    28,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    98,    54,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66
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
     449,   450,   451,   452,   454,   467,   469,   471,   118,   119,
     120,   133,   152,   162,   217,   250,   329,   348,   443,   348,
     200,   348,   348,   348,   104,   348,   348,   429,   430,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,    80,    87,   120,   147,   200,   227,   367,   403,   404,
     409,   443,   446,   443,    35,   348,   458,   459,   348,   120,
     200,   227,   403,   404,   405,   409,   440,   441,   442,   450,
     455,   456,   200,   339,   406,   200,   339,   355,   340,   348,
     236,   339,   200,   200,   200,   339,   202,   348,   217,   202,
     348,     3,     4,     6,     7,    10,    11,    12,    13,    27,
      29,    54,    56,    68,    69,    70,    71,    72,    73,    74,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   126,   127,   128,   129,   133,   134,   135,
     152,   156,   164,   166,   169,   176,   200,   217,   218,   219,
     230,   472,   487,   488,   490,   183,   202,   345,   348,   372,
     374,   203,   243,   348,   107,   108,   155,   220,   223,   226,
      80,   205,   295,   296,   119,   126,   118,   126,    80,   297,
     200,   200,   200,   200,   217,   267,   475,   200,   200,   340,
      80,    86,   148,   149,   150,   464,   465,   155,   203,   226,
     226,   217,   268,   475,   156,   200,   475,   475,    80,   197,
     203,   357,   338,   348,   349,   443,   447,   232,   203,    86,
     407,   464,    86,   464,   464,    30,   155,   172,   476,   200,
       9,   202,    35,   249,   156,   266,   475,   120,   182,   250,
     330,   202,   202,   202,   202,   202,   202,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    54,    66,   202,    67,    67,   202,   203,
     151,   127,   162,   164,   177,   179,   269,   328,   329,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    64,    65,   130,   131,   433,    67,   203,   438,
     200,   200,    67,   203,   205,   451,   200,   249,   250,    14,
     348,   202,   132,    45,   217,   428,    86,   338,   349,   151,
     443,   132,   207,     9,   414,   338,   349,   443,   476,   151,
     200,   408,   433,   438,   201,   348,    30,   234,     8,   360,
       9,   202,   234,   235,   340,   341,   348,   217,   281,   238,
     202,   202,   202,   134,   135,   490,   490,   172,   200,   107,
     490,    14,   151,   134,   135,   152,   217,   219,    80,   202,
     202,   202,   183,   184,   185,   190,   191,   194,   195,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   387,   388,
     389,   244,   111,   169,   202,   155,   221,   224,   226,   155,
     222,   225,   226,   226,     9,   202,    98,   203,   443,     9,
     202,   126,   126,    14,     9,   202,   443,   468,   340,   338,
     349,   443,   446,   447,   201,   172,   261,   133,   443,   457,
     458,   202,    67,   433,   148,   465,    79,   348,   443,    86,
     148,   465,   226,   216,   202,   203,   256,   264,   393,   395,
      87,   200,   205,   361,   362,   364,   404,   451,   469,    14,
      98,   470,   356,   358,   359,   291,   292,   431,   432,   201,
     201,   201,   201,   204,   233,   234,   251,   258,   263,   431,
     348,   206,   208,   209,   217,   477,   478,   490,    35,   165,
     293,   294,   348,   472,   200,   475,   259,   249,   348,   348,
     348,    30,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   405,   348,   348,   453,   453,
     348,   460,   461,   126,   203,   218,   219,   450,   451,   267,
     217,   268,   475,   475,   266,   250,    27,    35,   342,   345,
     348,   372,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   156,   203,   217,   434,   435,   436,
     437,   450,   453,   348,   293,   293,   453,   348,   457,   249,
     201,   348,   200,   427,     9,   414,   201,   201,    35,   348,
      35,   348,   201,   201,   201,   450,   293,   203,   217,   434,
     435,   450,   201,   232,   285,   203,   345,   348,   348,    90,
      30,   234,   279,   202,    28,    98,    14,     9,   201,    30,
     203,   282,   490,    29,    87,   230,   484,   485,   486,   200,
       9,    47,    48,    53,    55,    67,   134,   156,   176,   200,
     227,   228,   230,   369,   403,   409,   410,   411,   217,   489,
     186,    80,   348,    80,    80,   348,   384,   385,   348,   348,
     377,   387,   189,   390,   232,   200,   242,   226,   202,     9,
      98,   226,   202,     9,    98,    98,   223,   217,   348,   296,
     410,    80,     9,   201,   201,   201,   201,   201,   201,   201,
     202,    47,    48,   482,   483,   128,   272,   200,     9,   201,
     201,    80,    81,   217,   466,   217,    67,   204,   204,   213,
     215,    30,   129,   271,   171,    51,   156,   171,   397,   349,
     132,     9,   414,   201,   151,   490,   490,    14,   360,   291,
     232,   198,     9,   415,   490,   491,   433,   438,   433,   204,
       9,   414,   173,   443,   348,   201,     9,   415,    14,   352,
     252,   128,   270,   200,   475,   348,    30,   207,   207,   132,
     204,     9,   414,   348,   476,   200,   262,   257,   265,    14,
     470,   260,   249,    69,   443,   348,   476,   207,   204,   201,
     201,   207,   204,   201,    47,    48,    67,    75,    76,    77,
      87,   134,   147,   176,   217,   417,   419,   420,   423,   426,
     217,   443,   443,   132,   433,   438,   201,   348,   286,    72,
      73,   287,   232,   339,   232,   341,    98,    35,   133,   276,
     443,   410,   217,    30,   234,   280,   202,   283,   202,   283,
       9,   173,    87,   132,   151,     9,   414,   201,   165,   477,
     478,   479,   477,   410,   410,   410,   410,   410,   413,   416,
     200,   151,   200,   410,   151,   203,    10,    11,    12,    29,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    66,   151,   476,   348,   186,   186,    14,   192,
     193,   386,     9,   196,   390,    80,   204,   403,   203,   246,
      98,   224,   217,    98,   225,   217,   217,   204,    14,   443,
     202,     9,   173,   217,   273,   403,   203,   457,   133,   443,
      14,   207,   348,   204,   213,   490,   273,   203,   396,    14,
     201,   348,   361,   126,   218,   450,   202,   490,   198,   204,
      30,   480,   432,    35,    80,   165,   434,   435,   437,   434,
     435,   490,    35,   165,   348,   410,   291,   200,   403,   271,
     353,   253,   348,   348,   348,   204,   200,   293,   272,    30,
     271,   490,    14,   270,   475,   405,   204,   200,    14,    75,
      76,    77,   217,   418,   418,   420,   421,   422,    49,   200,
      86,   148,   200,     9,   414,   201,   427,    35,   348,   204,
      72,    73,   288,   339,   234,   204,   202,    91,   202,   276,
     443,   200,   132,   275,    14,   232,   283,   101,   102,   103,
     283,   204,   490,   132,   490,   217,   484,     9,   201,   414,
     132,   207,     9,   414,   413,   218,   361,   363,   365,   201,
     126,   218,   410,   462,   463,   410,   410,   410,    30,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   489,   348,   348,   348,   385,   348,   375,
      80,   247,   217,   217,   410,   483,    98,    99,   481,     9,
     301,   201,   200,   342,   345,   348,   207,   204,   470,   301,
     157,   170,   203,   392,   399,   157,   203,   398,   132,   202,
     480,   490,   360,   491,    80,   165,    14,    80,   476,   443,
     348,   201,   291,   203,   291,   200,   132,   200,   293,   201,
     203,   490,   203,   202,   490,   271,   254,   408,   293,   132,
     207,     9,   414,   419,   421,   148,   361,   424,   425,   420,
     443,   339,    30,    74,   234,   202,   341,   275,   457,   276,
     201,   410,    97,   101,   202,   348,    30,   202,   284,   204,
     173,   490,   132,   165,    30,   201,   410,   410,   201,   132,
       9,   414,   201,   132,   204,     9,   414,   410,    30,   187,
     201,   232,   217,   490,   490,   403,     4,   108,   113,   119,
     121,   158,   159,   161,   204,   302,   327,   328,   329,   334,
     335,   336,   337,   431,   457,   204,   203,   204,    51,   348,
     348,   348,   360,    35,    80,   165,    14,    80,   348,   200,
     480,   201,   301,   201,   291,   348,   293,   201,   301,   470,
     301,   202,   203,   200,   201,   420,   420,   201,   132,   201,
       9,   414,    30,   232,   202,   201,   201,   201,   239,   202,
     202,   284,   232,   490,   490,   132,   410,   361,   410,   410,
     410,   348,   203,   204,   481,   128,   129,   177,   218,   473,
     490,   274,   403,   108,   337,    29,   121,   134,   135,   156,
     162,   311,   312,   313,   314,   403,   160,   319,   320,   124,
     200,   217,   321,   322,   303,   250,   490,     9,   202,     9,
     202,   202,   470,   328,   201,   298,   156,   394,   204,   204,
      80,   165,    14,    80,   348,   293,   113,   350,   480,   204,
     480,   201,   201,   204,   203,   204,   301,   291,   132,   420,
     361,   232,   237,   240,    30,   234,   278,   232,   201,   410,
     132,   132,   188,   232,   403,   403,   475,    14,   218,     9,
     202,   203,   473,   470,   314,   172,   203,     9,   202,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    27,    28,
      54,    68,    69,    70,    71,    72,    73,    74,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   133,
     134,   136,   137,   138,   139,   140,   152,   153,   154,   164,
     166,   167,   169,   176,   177,   179,   181,   182,   217,   400,
     401,     9,   202,   156,   160,   217,   322,   323,   324,   202,
      80,   333,   249,   304,   473,   473,    14,   250,   204,   299,
     300,   473,    14,    80,   348,   201,   200,   203,   202,   203,
     325,   350,   480,   298,   204,   201,   420,   132,    30,   234,
     277,   278,   232,   410,   410,   348,   204,   202,   202,   410,
     403,   307,   490,   315,   316,   409,   312,    14,    30,    48,
     317,   320,     9,    33,   201,    29,    47,    50,    14,     9,
     202,   219,   474,   333,    14,   490,   249,   202,    14,   348,
      35,    80,   391,   232,   232,   203,   325,   204,   480,   420,
     232,    95,   189,   245,   204,   217,   230,   308,   309,   310,
       9,   173,     9,   414,   204,   410,   401,   401,    56,   318,
     323,   323,    29,    47,    50,   410,    80,   172,   200,   202,
     410,   475,   410,    80,     9,   415,   204,   204,   232,   325,
      93,   202,    80,   111,   241,   151,    98,   490,   409,   163,
      14,   482,   305,   200,    35,    80,   201,   204,   202,   200,
     169,   248,   217,   328,   329,   173,   410,   173,   289,   290,
     432,   306,    80,   403,   246,   166,   217,   202,   201,     9,
     415,   115,   116,   117,   331,   332,   289,    80,   274,   202,
     480,   432,   491,   201,   201,   202,   202,   203,   326,   331,
      35,    80,   165,   480,   203,   232,   491,    80,   165,    14,
      80,   326,   232,   204,    35,    80,   165,    14,    80,   348,
     204,    80,   165,    14,    80,   348,    14,    80,   348,   348
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
#line 2738 "hphp.y"
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
#line 2753 "hphp.y"
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
#line 2896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
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
#line 2972 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { (yyval).reset();;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { (yyval)++;;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 901:

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

  case 902:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 907:

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

  case 908:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { (yyval).reset();;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3040 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3093 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 943:

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

  case 944:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3107 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3119 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3124 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3162 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3174 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3185 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3189 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3195 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3211 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3243 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3249 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3252 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 996:

/* Line 1455 of yacc.c  */
#line 3256 "hphp.y"
    {;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3257 "hphp.y"
    {;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3258 "hphp.y"
    {;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3264 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1000:

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

  case 1001:

/* Line 1455 of yacc.c  */
#line 3280 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3285 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3286 "hphp.y"
    { ;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3291 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3292 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3298 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3308 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3312 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3319 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3322 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3326 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3329 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3335 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3339 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3342 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3345 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3351 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3357 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3365 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3366 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14539 "hphp.5.tab.cpp"
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
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

