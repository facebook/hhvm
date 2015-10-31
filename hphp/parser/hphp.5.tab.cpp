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
#define YYLAST   16513

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  196
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  265
/* YYNRULES -- Number of rules.  */
#define YYNRULES  982
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1788

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
    2688,  2690,  2692,  2694,  2696,  2700,  2704,  2709,  2714,  2718,
    2720,  2722,  2730,  2740,  2748,  2755,  2764,  2766,  2769,  2774,
    2779,  2781,  2783,  2788,  2790,  2791,  2793,  2796,  2798,  2800,
    2802,  2806,  2810,  2814,  2815,  2817,  2819,  2823,  2827,  2830,
    2834,  2841,  2842,  2844,  2849,  2852,  2853,  2859,  2863,  2867,
    2869,  2876,  2881,  2886,  2889,  2892,  2893,  2899,  2903,  2907,
    2909,  2912,  2913,  2919,  2923,  2927,  2929,  2932,  2935,  2937,
    2940,  2942,  2947,  2951,  2955,  2962,  2966,  2968,  2970,  2972,
    2977,  2982,  2987,  2992,  2997,  3002,  3005,  3008,  3013,  3016,
    3019,  3021,  3025,  3029,  3033,  3034,  3037,  3043,  3050,  3057,
    3065,  3067,  3070,  3072,  3075,  3077,  3082,  3084,  3089,  3093,
    3094,  3096,  3100,  3103,  3107,  3109,  3111,  3112,  3113,  3116,
    3119,  3122,  3127,  3130,  3136,  3140,  3142,  3144,  3145,  3149,
    3154,  3160,  3164,  3166,  3169,  3170,  3175,  3177,  3181,  3184,
    3187,  3190,  3192,  3194,  3196,  3198,  3202,  3207,  3214,  3216,
    3225,  3232,  3234
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     197,     0,    -1,    -1,   198,   199,    -1,   199,   200,    -1,
      -1,   220,    -1,   237,    -1,   244,    -1,   241,    -1,   249,
      -1,   440,    -1,   125,   186,   187,   188,    -1,   152,   212,
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
      -1,   213,    -1,   213,   445,    -1,   213,   445,    -1,   217,
       9,   441,    14,   379,    -1,   108,   441,    14,   379,    -1,
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
     144,   326,   188,    -1,   122,   186,   437,   187,   188,    -1,
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
      35,    -1,    -1,   107,    -1,    -1,   236,   235,   444,   238,
     186,   277,   187,   449,   311,    -1,    -1,   315,   236,   235,
     444,   239,   186,   277,   187,   449,   311,    -1,    -1,   400,
     314,   236,   235,   444,   240,   186,   277,   187,   449,   311,
      -1,    -1,   162,   203,   242,    30,   459,   439,   189,   284,
     190,    -1,    -1,   400,   162,   203,   243,    30,   459,   439,
     189,   284,   190,    -1,    -1,   255,   252,   245,   256,   257,
     189,   287,   190,    -1,    -1,   400,   255,   252,   246,   256,
     257,   189,   287,   190,    -1,    -1,   127,   253,   247,   258,
     189,   287,   190,    -1,    -1,   400,   127,   253,   248,   258,
     189,   287,   190,    -1,    -1,   164,   254,   250,   257,   189,
     287,   190,    -1,    -1,   400,   164,   254,   251,   257,   189,
     287,   190,    -1,   444,    -1,   156,    -1,   444,    -1,   444,
      -1,   126,    -1,   119,   126,    -1,   119,   118,   126,    -1,
     118,   119,   126,    -1,   118,   126,    -1,   128,   370,    -1,
      -1,   129,   259,    -1,    -1,   128,   259,    -1,    -1,   370,
      -1,   259,     9,   370,    -1,   370,    -1,   260,     9,   370,
      -1,   132,   262,    -1,    -1,   412,    -1,    35,   412,    -1,
     133,   186,   426,   187,    -1,   220,    -1,    30,   218,    93,
     188,    -1,   220,    -1,    30,   218,    95,   188,    -1,   220,
      -1,    30,   218,    91,   188,    -1,   220,    -1,    30,   218,
      97,   188,    -1,   203,    14,   379,    -1,   267,     9,   203,
      14,   379,    -1,   189,   269,   190,    -1,   189,   188,   269,
     190,    -1,    30,   269,   101,   188,    -1,    30,   188,   269,
     101,   188,    -1,   269,   102,   334,   270,   218,    -1,   269,
     103,   270,   218,    -1,    -1,    30,    -1,   188,    -1,   271,
      72,   325,   220,    -1,    -1,   272,    72,   325,    30,   218,
      -1,    -1,    73,   220,    -1,    -1,    73,    30,   218,    -1,
      -1,   276,     9,   401,   317,   460,   165,    80,    -1,   276,
       9,   401,   317,   460,    35,   165,    80,    -1,   276,     9,
     401,   317,   460,   165,    -1,   276,   384,    -1,   401,   317,
     460,   165,    80,    -1,   401,   317,   460,    35,   165,    80,
      -1,   401,   317,   460,   165,    -1,    -1,   401,   317,   460,
      80,    -1,   401,   317,   460,    35,    80,    -1,   401,   317,
     460,    35,    80,    14,   334,    -1,   401,   317,   460,    80,
      14,   334,    -1,   276,     9,   401,   317,   460,    80,    -1,
     276,     9,   401,   317,   460,    35,    80,    -1,   276,     9,
     401,   317,   460,    35,    80,    14,   334,    -1,   276,     9,
     401,   317,   460,    80,    14,   334,    -1,   278,     9,   401,
     460,   165,    80,    -1,   278,     9,   401,   460,    35,   165,
      80,    -1,   278,     9,   401,   460,   165,    -1,   278,   384,
      -1,   401,   460,   165,    80,    -1,   401,   460,    35,   165,
      80,    -1,   401,   460,   165,    -1,    -1,   401,   460,    80,
      -1,   401,   460,    35,    80,    -1,   401,   460,    35,    80,
      14,   334,    -1,   401,   460,    80,    14,   334,    -1,   278,
       9,   401,   460,    80,    -1,   278,     9,   401,   460,    35,
      80,    -1,   278,     9,   401,   460,    35,    80,    14,   334,
      -1,   278,     9,   401,   460,    80,    14,   334,    -1,   280,
     384,    -1,    -1,   334,    -1,    35,   412,    -1,   165,   334,
      -1,   280,     9,   334,    -1,   280,     9,   165,   334,    -1,
     280,     9,    35,   412,    -1,   281,     9,   282,    -1,   282,
      -1,    80,    -1,   191,   412,    -1,   191,   189,   334,   190,
      -1,   283,     9,    80,    -1,   283,     9,    80,    14,   379,
      -1,    80,    -1,    80,    14,   379,    -1,   284,   285,    -1,
      -1,   286,   188,    -1,   442,    14,   379,    -1,   287,   288,
      -1,    -1,    -1,   313,   289,   319,   188,    -1,    -1,   315,
     459,   290,   319,   188,    -1,   320,   188,    -1,   321,   188,
      -1,   322,   188,    -1,    -1,   314,   236,   235,   443,   186,
     291,   275,   187,   449,   312,    -1,    -1,   400,   314,   236,
     235,   444,   186,   292,   275,   187,   449,   312,    -1,   158,
     297,   188,    -1,   159,   305,   188,    -1,   161,   307,   188,
      -1,     4,   128,   370,   188,    -1,     4,   129,   370,   188,
      -1,   113,   260,   188,    -1,   113,   260,   189,   293,   190,
      -1,   293,   294,    -1,   293,   295,    -1,    -1,   216,   151,
     203,   166,   260,   188,    -1,   296,    98,   314,   203,   188,
      -1,   296,    98,   315,   188,    -1,   216,   151,   203,    -1,
     203,    -1,   298,    -1,   297,     9,   298,    -1,   299,   367,
     303,   304,    -1,   156,    -1,    29,   300,    -1,   300,    -1,
     134,    -1,   134,   172,   459,   173,    -1,   134,   172,   459,
       9,   459,   173,    -1,   370,    -1,   121,    -1,   162,   189,
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
      -1,    80,    14,   379,    -1,   320,     9,   442,    14,   379,
      -1,   108,   442,    14,   379,    -1,   321,     9,   442,    -1,
     119,   108,   442,    -1,   119,   323,   439,    -1,   323,   439,
      14,   459,    -1,   108,   177,   444,    -1,   186,   324,   187,
      -1,    69,   374,   377,    -1,    68,   334,    -1,   359,    -1,
     354,    -1,   186,   334,   187,    -1,   326,     9,   334,    -1,
     334,    -1,   326,    -1,    -1,    27,    -1,    27,   334,    -1,
      27,   334,   132,   334,    -1,   186,   328,   187,    -1,   412,
      14,   328,    -1,   133,   186,   426,   187,    14,   328,    -1,
      28,   334,    -1,   412,    14,   331,    -1,   133,   186,   426,
     187,    14,   331,    -1,   335,    -1,   412,    -1,   324,    -1,
     416,    -1,   415,    -1,   133,   186,   426,   187,    14,   334,
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
     334,    -1,   436,    -1,    63,   334,    -1,    62,   334,    -1,
      61,   334,    -1,    60,   334,    -1,    59,   334,    -1,    58,
     334,    -1,    57,   334,    -1,    70,   375,    -1,    56,   334,
      -1,   381,    -1,   353,    -1,   352,    -1,   192,   376,   192,
      -1,    13,   334,    -1,   356,    -1,   113,   186,   358,   384,
     187,    -1,    -1,    -1,   236,   235,   186,   338,   277,   187,
     449,   336,   189,   218,   190,    -1,    -1,   315,   236,   235,
     186,   339,   277,   187,   449,   336,   189,   218,   190,    -1,
      -1,   182,    80,   341,   346,    -1,    -1,   182,   183,   342,
     277,   184,   449,   346,    -1,    -1,   182,   189,   343,   218,
     190,    -1,    -1,    80,   344,   346,    -1,    -1,   183,   345,
     277,   184,   449,   346,    -1,     8,   334,    -1,     8,   331,
      -1,     8,   189,   218,   190,    -1,    87,    -1,   438,    -1,
     348,     9,   347,   132,   334,    -1,   347,   132,   334,    -1,
     349,     9,   347,   132,   379,    -1,   347,   132,   379,    -1,
     348,   383,    -1,    -1,   349,   383,    -1,    -1,   176,   186,
     350,   187,    -1,   134,   186,   427,   187,    -1,    67,   427,
     193,    -1,   370,   189,   429,   190,    -1,   370,   189,   431,
     190,    -1,   356,    67,   422,   193,    -1,   357,    67,   422,
     193,    -1,   353,    -1,   438,    -1,   415,    -1,    87,    -1,
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
     335,   187,    -1,   371,    -1,   372,   151,   419,    -1,   371,
      -1,   418,    -1,   373,   151,   419,    -1,   370,    -1,   120,
      -1,   424,    -1,   186,   187,    -1,   325,    -1,    -1,    -1,
      86,    -1,   433,    -1,   186,   279,   187,    -1,    -1,    75,
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
      -1,   438,    -1,   378,    -1,   194,   433,   194,    -1,   195,
     433,   195,    -1,   147,   433,   148,    -1,   385,   383,    -1,
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
      -1,   156,    -1,   189,   334,   190,    -1,   403,    -1,   419,
      -1,   203,    -1,   189,   334,   190,    -1,   405,    -1,   419,
      -1,    67,   422,   193,    -1,   189,   334,   190,    -1,   413,
     407,    -1,   186,   324,   187,   407,    -1,   425,   407,    -1,
     186,   324,   187,   407,    -1,   186,   324,   187,   402,   404,
      -1,   186,   335,   187,   402,   404,    -1,   186,   324,   187,
     402,   403,    -1,   186,   335,   187,   402,   403,    -1,   419,
      -1,   369,    -1,   417,    -1,   418,    -1,   408,    -1,   410,
      -1,   412,   402,   404,    -1,   373,   151,   419,    -1,   414,
     186,   279,   187,    -1,   415,   186,   279,   187,    -1,   186,
     412,   187,    -1,   369,    -1,   417,    -1,   418,    -1,   408,
      -1,   412,   402,   403,    -1,   411,    -1,   414,   186,   279,
     187,    -1,   186,   412,   187,    -1,   419,    -1,   408,    -1,
     369,    -1,   353,    -1,   378,    -1,   186,   412,   187,    -1,
     186,   335,   187,    -1,   415,   186,   279,   187,    -1,   414,
     186,   279,   187,    -1,   186,   416,   187,    -1,   337,    -1,
     340,    -1,   412,   402,   406,   445,   186,   279,   187,    -1,
     186,   324,   187,   402,   406,   445,   186,   279,   187,    -1,
     373,   151,   205,   445,   186,   279,   187,    -1,   373,   151,
     419,   186,   279,   187,    -1,   373,   151,   189,   334,   190,
     186,   279,   187,    -1,   420,    -1,   423,   420,    -1,   420,
      67,   422,   193,    -1,   420,   189,   334,   190,    -1,   421,
      -1,    80,    -1,   191,   189,   334,   190,    -1,   334,    -1,
      -1,   191,    -1,   423,   191,    -1,   419,    -1,   409,    -1,
     410,    -1,   424,   402,   404,    -1,   372,   151,   419,    -1,
     186,   412,   187,    -1,    -1,   409,    -1,   411,    -1,   424,
     402,   403,    -1,   186,   412,   187,    -1,   426,     9,    -1,
     426,     9,   412,    -1,   426,     9,   133,   186,   426,   187,
      -1,    -1,   412,    -1,   133,   186,   426,   187,    -1,   428,
     383,    -1,    -1,   428,     9,   334,   132,   334,    -1,   428,
       9,   334,    -1,   334,   132,   334,    -1,   334,    -1,   428,
       9,   334,   132,    35,   412,    -1,   428,     9,    35,   412,
      -1,   334,   132,    35,   412,    -1,    35,   412,    -1,   430,
     383,    -1,    -1,   430,     9,   334,   132,   334,    -1,   430,
       9,   334,    -1,   334,   132,   334,    -1,   334,    -1,   432,
     383,    -1,    -1,   432,     9,   379,   132,   379,    -1,   432,
       9,   379,    -1,   379,   132,   379,    -1,   379,    -1,   433,
     434,    -1,   433,    86,    -1,   434,    -1,    86,   434,    -1,
      80,    -1,    80,    67,   435,   193,    -1,    80,   402,   203,
      -1,   149,   334,   190,    -1,   149,    79,    67,   334,   193,
     190,    -1,   150,   412,   190,    -1,   203,    -1,    81,    -1,
      80,    -1,   123,   186,   326,   187,    -1,   124,   186,   412,
     187,    -1,   124,   186,   335,   187,    -1,   124,   186,   416,
     187,    -1,   124,   186,   415,   187,    -1,   124,   186,   324,
     187,    -1,     7,   334,    -1,     6,   334,    -1,     5,   186,
     334,   187,    -1,     4,   334,    -1,     3,   334,    -1,   412,
      -1,   437,     9,   412,    -1,   373,   151,   204,    -1,   373,
     151,   126,    -1,    -1,    98,   459,    -1,   177,   444,    14,
     459,   188,    -1,   400,   177,   444,    14,   459,   188,    -1,
     179,   444,   439,    14,   459,   188,    -1,   400,   179,   444,
     439,    14,   459,   188,    -1,   205,    -1,   459,   205,    -1,
     204,    -1,   459,   204,    -1,   205,    -1,   205,   172,   451,
     173,    -1,   203,    -1,   203,   172,   451,   173,    -1,   172,
     447,   173,    -1,    -1,   459,    -1,   446,     9,   459,    -1,
     446,   383,    -1,   446,     9,   165,    -1,   447,    -1,   165,
      -1,    -1,    -1,    30,   459,    -1,    98,   459,    -1,    99,
     459,    -1,   451,     9,   452,   203,    -1,   452,   203,    -1,
     451,     9,   452,   203,   450,    -1,   452,   203,   450,    -1,
      47,    -1,    48,    -1,    -1,    87,   132,   459,    -1,    29,
      87,   132,   459,    -1,   216,   151,   203,   132,   459,    -1,
     454,     9,   453,    -1,   453,    -1,   454,   383,    -1,    -1,
     176,   186,   455,   187,    -1,   216,    -1,   203,   151,   458,
      -1,   203,   445,    -1,    29,   459,    -1,    56,   459,    -1,
     216,    -1,   134,    -1,   135,    -1,   456,    -1,   457,   151,
     458,    -1,   134,   172,   459,   173,    -1,   134,   172,   459,
       9,   459,   173,    -1,   156,    -1,   186,   107,   186,   448,
     187,    30,   459,   187,    -1,   186,   459,     9,   446,   383,
     187,    -1,   459,    -1,    -1
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
    2621,  2625,  2626,  2627,  2631,  2633,  2641,  2642,  2646,  2648,
    2656,  2657,  2661,  2662,  2667,  2669,  2674,  2685,  2699,  2711,
    2726,  2727,  2728,  2729,  2730,  2731,  2732,  2742,  2751,  2753,
    2755,  2759,  2760,  2761,  2762,  2763,  2779,  2780,  2782,  2791,
    2792,  2793,  2794,  2795,  2796,  2797,  2798,  2800,  2805,  2809,
    2810,  2814,  2817,  2824,  2828,  2837,  2844,  2846,  2852,  2854,
    2855,  2859,  2860,  2867,  2868,  2873,  2874,  2879,  2880,  2881,
    2882,  2893,  2896,  2899,  2900,  2901,  2902,  2913,  2917,  2918,
    2919,  2921,  2922,  2923,  2927,  2929,  2932,  2934,  2935,  2936,
    2937,  2940,  2942,  2943,  2947,  2949,  2952,  2954,  2955,  2956,
    2960,  2962,  2965,  2968,  2970,  2972,  2976,  2977,  2979,  2980,
    2986,  2987,  2989,  2999,  3001,  3003,  3006,  3007,  3008,  3012,
    3013,  3014,  3015,  3016,  3017,  3018,  3019,  3020,  3021,  3022,
    3026,  3027,  3031,  3033,  3041,  3043,  3047,  3051,  3056,  3060,
    3068,  3069,  3073,  3074,  3080,  3081,  3090,  3091,  3099,  3102,
    3106,  3109,  3114,  3119,  3121,  3122,  3123,  3127,  3128,  3132,
    3133,  3136,  3139,  3141,  3145,  3151,  3152,  3153,  3157,  3161,
    3171,  3179,  3181,  3185,  3187,  3192,  3198,  3201,  3206,  3214,
    3217,  3220,  3221,  3224,  3227,  3228,  3233,  3236,  3240,  3244,
    3250,  3260,  3261
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
     412,   413,   413,   413,   413,   413,   413,   413,   413,   414,
     414,   414,   414,   414,   414,   414,   414,   414,   415,   416,
     416,   417,   417,   418,   418,   418,   419,   419,   420,   420,
     420,   421,   421,   422,   422,   423,   423,   424,   424,   424,
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
       3,     1,     1,     1,     1,     3,     1,     4,     3,     1,
       1,     1,     1,     1,     3,     3,     4,     4,     3,     1,
       1,     7,     9,     7,     6,     8,     1,     2,     4,     4,
       1,     1,     4,     1,     0,     1,     2,     1,     1,     1,
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
       0,   421,     0,   784,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   875,     0,
     863,   666,     0,   672,   673,   674,    22,   731,   851,   151,
     152,   675,     0,   132,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
     388,   389,   390,   393,   392,   391,     0,     0,     0,     0,
     210,     0,     0,     0,   679,   681,   682,   676,   677,     0,
       0,     0,   683,   678,     0,   650,    24,    25,    26,    28,
      27,     0,   680,     0,     0,     0,     0,   684,   394,   519,
       0,   150,   122,   855,   667,     0,     0,     4,   111,   113,
     116,   730,     0,   649,     0,     6,   182,     7,     9,     8,
      10,     0,     0,   386,   432,     0,     0,     0,     0,     0,
       0,     0,   430,   839,   840,   501,   500,   415,   504,     0,
     414,   811,   651,   658,     0,   733,   499,   385,   814,   815,
     826,   431,     0,     0,   434,   433,   812,   813,   810,   846,
     850,     0,   489,   732,    11,   393,   392,   391,     0,     0,
      28,     0,   111,   182,     0,   919,   431,   918,     0,   916,
     915,   503,     0,   422,   427,     0,     0,   472,   473,   474,
     475,   498,   496,   495,   494,   493,   492,   491,   490,   851,
     675,   653,     0,     0,   939,   832,   651,     0,   652,   454,
       0,   452,     0,   879,     0,   740,   413,   662,     0,   939,
     661,   656,     0,   671,   652,   858,   859,   865,   857,   663,
       0,     0,   665,   497,     0,     0,     0,     0,   418,     0,
     130,   420,     0,     0,   136,   138,     0,     0,   140,     0,
      69,    68,    63,    62,    54,    55,    46,    66,    77,     0,
      49,     0,    61,    53,    59,    79,    72,    71,    44,    67,
      86,    87,    45,    82,    42,    83,    43,    84,    41,    88,
      76,    80,    85,    73,    74,    48,    75,    78,    40,    70,
      56,    89,    64,    57,    47,    39,    38,    37,    36,    35,
      34,    58,    90,    92,    51,    32,    33,    60,   972,   973,
      52,   978,    31,    50,    81,     0,     0,   111,    91,   930,
     971,     0,   974,     0,     0,   142,     0,     0,   173,     0,
       0,     0,     0,     0,     0,    94,    99,   299,     0,     0,
     298,     0,   214,     0,   211,   304,     0,     0,     0,     0,
       0,   936,   198,   208,   871,   875,     0,   900,     0,   686,
       0,     0,     0,   898,     0,    16,     0,   115,   190,   202,
     209,   556,   531,     0,   924,   511,   513,   515,   788,   421,
     432,     0,     0,   430,   431,   433,     0,     0,   668,     0,
     669,     0,     0,     0,   172,     0,     0,   118,   290,     0,
      21,   181,     0,   207,   194,   206,   391,   394,   182,   387,
     165,   166,   167,   168,   169,   171,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   863,     0,   164,   854,   854,   885,     0,     0,
       0,     0,     0,     0,     0,     0,   384,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     453,   451,   789,   790,     0,   854,     0,   802,   290,   290,
     854,     0,   856,   847,   871,     0,   182,     0,     0,   144,
       0,   786,   781,   740,     0,   432,   430,     0,   883,     0,
     536,   739,   874,   432,   430,   431,   118,     0,   290,   412,
       0,   804,   664,     0,   122,   250,     0,   518,     0,   147,
       0,     0,   419,     0,     0,     0,     0,     0,   139,   163,
     141,   972,   973,   969,   970,     0,   964,     0,     0,     0,
       0,    65,    30,    52,    29,   931,   170,   143,   122,     0,
     160,   162,     0,     0,    96,   103,     0,     0,    98,   107,
     100,     0,    18,     0,     0,   300,     0,   145,   213,   212,
       0,     0,   146,   920,     0,     0,   432,   430,   431,   434,
     433,     0,   957,   220,     0,   872,     0,     0,   148,     0,
       0,   685,   899,   731,     0,     0,   897,   736,   896,   114,
       5,    13,    14,     0,   218,     0,     0,   524,     0,     0,
       0,   740,     0,     0,   659,   654,   525,     0,     0,     0,
       0,   788,   122,     0,   742,   787,   982,   411,   424,   486,
     820,   838,   127,   121,   123,   124,   125,   126,   385,     0,
     502,   734,   735,   112,   740,     0,   940,     0,     0,     0,
     742,   291,     0,   507,   184,   216,     0,   457,   459,   458,
       0,     0,   455,   456,   460,   462,   461,   477,   476,   479,
     478,   480,   482,   484,   483,   481,   471,   470,   464,   465,
     463,   466,   467,   469,   485,   468,   853,     0,     0,   889,
       0,   740,   923,     0,   922,   939,   817,   846,   200,   192,
     204,     0,   924,   196,   182,     0,   425,   428,   436,   450,
     449,   448,   447,   446,   445,   444,   443,   442,   441,   440,
     439,   792,     0,   791,   794,   816,   798,   939,   795,     0,
       0,     0,     0,     0,     0,     0,     0,   917,   423,   779,
     783,   739,   785,     0,   655,     0,   878,     0,   877,     0,
     655,   862,   861,     0,     0,   791,   794,   860,   795,   416,
     252,   254,   122,   522,   521,   417,     0,   122,   234,   131,
     420,     0,     0,     0,     0,     0,   246,   246,   137,     0,
       0,     0,     0,   962,   740,     0,   946,     0,     0,     0,
       0,     0,   738,     0,   650,     0,     0,   116,   688,   649,
     693,     0,   687,   120,   692,   939,   975,     0,     0,     0,
     104,     0,    19,     0,   108,     0,    20,     0,     0,    93,
     101,     0,   297,   305,   302,     0,     0,   909,   914,   911,
     910,   913,   912,    12,   955,   956,     0,     0,     0,     0,
     871,   868,     0,   535,   908,   907,   906,     0,   902,     0,
     903,   905,     0,     5,     0,     0,     0,   550,   551,   559,
     558,     0,   430,     0,   739,   530,   534,     0,     0,   925,
       0,   512,     0,     0,   947,   788,   276,   981,     0,     0,
     803,     0,   852,   739,   942,   938,   292,   293,   648,   741,
     289,     0,   788,     0,     0,   218,   509,   186,   488,     0,
     539,   540,     0,   537,   739,   884,     0,     0,   290,   220,
       0,   218,     0,     0,   216,     0,   863,   437,     0,     0,
     800,   801,   818,   819,   848,   849,     0,     0,     0,   767,
     747,   748,   749,   756,     0,     0,     0,   760,   758,   759,
     773,   740,     0,   781,   882,   881,     0,     0,   805,   670,
       0,   256,     0,     0,   128,     0,     0,     0,     0,     0,
       0,     0,   226,   227,   238,     0,   122,   236,   157,   246,
       0,   246,     0,     0,   976,     0,     0,     0,   739,   963,
     965,   945,   740,   944,     0,   740,   714,   715,   712,   713,
     746,     0,   740,   738,     0,   533,     0,     0,   891,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   968,   174,     0,
     177,   161,     0,    95,   105,     0,    97,   109,   102,   301,
       0,   921,   149,   957,   937,   952,   219,   221,   311,     0,
       0,   869,     0,   901,     0,    17,     0,   924,   217,   311,
       0,     0,   655,   527,     0,   660,   926,     0,   947,   516,
       0,     0,   982,     0,   281,   279,   794,   806,   939,   794,
     807,   941,     0,     0,   294,   119,     0,   788,   215,     0,
     788,     0,   487,   888,   887,     0,   290,     0,     0,     0,
       0,     0,     0,   218,   188,   671,   793,   290,     0,   752,
     753,   754,   755,   761,   762,   771,     0,   740,     0,   767,
       0,   751,   775,   739,   778,   780,   782,     0,   876,   793,
       0,     0,     0,     0,   253,   523,   133,     0,   420,   226,
     228,   871,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   240,     0,     0,   958,     0,   961,   739,     0,     0,
       0,   690,   739,   737,     0,   728,     0,   740,     0,   694,
     729,   727,   895,     0,   740,   697,   699,   698,     0,     0,
     695,   696,   700,   702,   701,   717,   716,   719,   718,   720,
     722,   724,   723,   721,   710,   709,   704,   705,   703,   706,
     707,   708,   711,   967,     0,   122,   106,   110,   303,     0,
       0,     0,   954,     0,   385,   873,   871,   426,   429,   435,
       0,    15,     0,   385,   562,     0,     0,   564,   557,   560,
       0,   555,     0,   928,     0,   948,   520,     0,   282,     0,
       0,   277,     0,   296,   295,   947,     0,   311,     0,   788,
       0,   290,     0,   844,   311,   924,   311,   927,     0,     0,
       0,   438,     0,     0,   764,   739,   766,   757,     0,   750,
       0,     0,   740,   772,   880,     0,   122,     0,   249,   235,
       0,     0,     0,   225,   153,   239,     0,     0,   242,     0,
     247,   248,   122,   241,   977,   959,     0,   943,     0,   980,
     745,   744,   689,     0,   739,   532,   691,     0,   538,   739,
     890,   726,     0,     0,     0,   951,   949,   950,   222,     0,
       0,     0,   392,   383,     0,     0,     0,   199,   310,   312,
       0,   382,     0,     0,     0,   924,   385,     0,   904,   307,
     203,   553,     0,     0,   526,   514,     0,   285,   275,     0,
     278,   284,   290,   506,   947,   385,   947,     0,   886,     0,
     843,   385,     0,   385,   929,   311,   788,   841,   770,   769,
     763,     0,   765,   739,   774,   122,   255,   129,   134,   155,
     229,     0,   237,   243,   122,   245,   960,     0,     0,   529,
       0,   894,   893,   725,   122,   178,   953,     0,     0,     0,
     932,     0,     0,     0,   223,     0,   924,     0,   348,   344,
     350,   650,    28,     0,   338,     0,   343,   347,   360,     0,
     358,   363,     0,   362,     0,   361,     0,   182,   314,     0,
     316,     0,   317,   318,     0,     0,   870,     0,   554,   552,
     563,   561,   286,     0,     0,   273,   283,     0,     0,     0,
       0,   195,   506,   947,   845,   201,   307,   205,   385,     0,
       0,   777,     0,   251,     0,     0,   122,   232,   154,   244,
     979,   743,     0,     0,     0,     0,     0,   410,     0,   933,
       0,   328,   332,   407,   408,   342,     0,     0,     0,   323,
     614,   613,   610,   612,   611,   631,   633,   632,   602,   573,
     574,   592,   608,   607,   569,   579,   580,   582,   581,   601,
     585,   583,   584,   586,   587,   588,   589,   590,   591,   593,
     594,   595,   596,   597,   598,   600,   599,   570,   571,   572,
     575,   576,   578,   616,   617,   626,   625,   624,   623,   622,
     621,   609,   628,   618,   619,   620,   603,   604,   605,   606,
     629,   630,   634,   636,   635,   637,   638,   615,   640,   639,
     642,   644,   643,   577,   647,   645,   646,   641,   627,   568,
     355,   565,     0,   324,   376,   377,   375,   368,     0,   369,
     325,   402,     0,     0,     0,     0,   406,     0,   182,   191,
     306,     0,     0,     0,   274,   288,   842,     0,   122,   378,
     122,   185,     0,     0,     0,   197,   947,   768,     0,   122,
     230,   135,   156,     0,   528,   892,   176,   326,   327,   405,
     224,     0,     0,   740,     0,   351,   339,     0,     0,     0,
     357,   359,     0,     0,   364,   371,   372,   370,     0,     0,
     313,   934,     0,     0,     0,   409,     0,   308,     0,   287,
       0,   548,   742,     0,     0,   122,   187,   193,     0,   776,
       0,     0,   158,   329,   111,     0,   330,   331,     0,     0,
     345,   739,   353,   349,   354,   566,   567,     0,   340,   373,
     374,   366,   367,   365,   403,   400,   957,   319,   315,   404,
       0,   309,   549,   741,     0,   508,   379,     0,   189,     0,
     233,     0,   180,     0,   385,     0,   352,   356,     0,     0,
     788,   321,     0,   546,   505,   510,   231,     0,     0,   159,
     336,     0,   384,   346,   401,   935,     0,   742,   396,   788,
     547,     0,   179,     0,     0,   335,   947,   788,   260,   397,
     398,   399,   982,   395,     0,     0,     0,   334,     0,   396,
       0,   947,     0,   333,   380,   122,   320,   982,     0,   265,
     263,     0,   122,     0,     0,   266,     0,     0,   261,   322,
       0,   381,     0,   269,   259,     0,   262,   268,   175,   270,
       0,     0,   257,   267,     0,   258,   272,   271
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   853,   600,   172,  1400,   695,
     334,   553,   557,   335,   554,   558,   109,   110,   111,   112,
     113,   114,   386,   633,   634,   521,   239,  1464,   527,  1381,
    1465,  1702,   809,   329,   548,  1662,  1031,  1205,  1719,   402,
     173,   635,   893,  1091,  1260,   118,   603,   910,   636,   655,
     914,   583,   909,   637,   604,   911,   404,   352,   369,   121,
     895,   856,   839,  1046,  1403,  1143,   962,  1611,  1468,   769,
     968,   526,   778,   970,  1292,   761,   951,   954,  1132,  1726,
    1727,   623,   624,   649,   650,   339,   340,   346,  1437,  1590,
    1591,  1214,  1328,  1426,  1584,  1710,  1729,  1621,  1666,  1667,
    1668,  1413,  1414,  1415,  1416,  1623,  1624,  1630,  1678,  1419,
    1420,  1424,  1577,  1578,  1579,  1601,  1756,  1329,  1330,   174,
     123,  1742,  1743,  1582,  1332,  1333,  1334,  1335,   124,   232,
     522,   523,   125,   126,   127,   128,   129,   130,   131,   132,
    1449,   133,   892,  1090,   134,   620,   621,   622,   236,   378,
     517,   610,   611,  1167,   612,  1168,   135,   136,   137,   800,
     138,   139,  1652,   140,   605,  1439,   606,  1060,   861,  1231,
    1228,  1570,  1571,   141,   142,   143,   222,   144,   223,   233,
     389,   509,   145,   990,   804,   146,   991,   884,   876,   992,
     938,  1113,   939,  1115,  1116,  1117,   941,  1271,  1272,   942,
     740,   493,   185,   186,   638,   626,   474,  1076,  1077,   726,
     727,   880,   148,   225,   149,   150,   176,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   687,   161,   229,   230,
     586,   214,   215,   690,   691,  1173,  1174,   362,   363,   847,
     162,   574,   163,   619,   164,   321,  1592,  1642,   353,   397,
     644,   645,   984,  1071,  1212,   836,   837,   783,   784,   785,
     322,   323,   806,  1402,   878
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1464
static const yytype_int16 yypact[] =
{
   -1464,   167, -1464, -1464,  5233, 12760, 12760,   -15, 12760, 12760,
   12760, 10637, 12760, -1464, 12760, 12760, 12760, 12760, 12760, 12760,
   12760, 12760, 12760, 12760, 12760, 12760, 15520, 15520, 10830, 12760,
    3635,     4,    55, -1464, -1464, -1464, -1464, -1464,   239, -1464,
   -1464,   255, 12760, -1464,    55,   173,   212,   315, -1464,    55,
   11023,  1339, 11216, -1464, 13662,  4548,   359, 12760,  1542,    40,
   -1464, -1464, -1464,   510,   404,    66,   389,   396,   416,   420,
   -1464,  1339,   426,   446, -1464, -1464, -1464, -1464, -1464, 12760,
     477,   730, -1464, -1464,  1339, -1464, -1464, -1464, -1464,  1339,
   -1464,  1339, -1464,   491,   493,  1339,  1339, -1464,    32, -1464,
   11409, -1464, -1464,   467,   560,   688,   688, -1464,   657,   529,
     556, -1464,   527, -1464,    76, -1464,   683, -1464, -1464, -1464,
   -1464,  1116,  1021, -1464, -1464,   542,   548,   558,   571,   574,
     581, 12745, -1464, -1464, -1464, -1464,   280, -1464,   629,   677,
   -1464,   141,   559, -1464,   620,    28, -1464,  1396,   143, -1464,
   -1464,  3951,    36,   599,   306, -1464,   103,   149,   625,   211,
   -1464,   206, -1464,   720, -1464, -1464, -1464,   663,   635,   675,
   -1464, 12760, -1464,   683,  1021, 16089,  4123, 16089, 12760, 16089,
   16089, 16339,   630, 15027, 16339,   789,  1339,   770,   770,   294,
     770,   770,   770,   770,   770,   770,   770,   770,   770, -1464,
   -1464, -1464,    43, 12760,   669, -1464, -1464,   694,   671,    21,
     676,    21, 15520, 15681,   656,   854, -1464,   663, 12760,   669,
     715, -1464,   718,   691, -1464,   145, -1464, -1464, -1464,    21,
      36, 11602, -1464, -1464, 12760,  8514,   866,    83, 16089,  9479,
   -1464, 12760, 12760,  1339, -1464, -1464, 14265,   690, -1464, 14310,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,  2865,
   -1464,  2865, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,    67,    80,
     675, -1464, -1464, -1464, -1464,   700,  2203,    91, -1464, -1464,
     728,   876, -1464,   745, 14592, -1464,   713, 14355, -1464,    37,
   14400,  1564,  1846,  1339,    99, -1464,    41, -1464, 15144,   100,
   -1464,   778, -1464,   780, -1464,   894,   101, 15520, 12760, 12760,
     722,   739, -1464, -1464, 15237, 10830,   102,   478,   602, -1464,
   12953, 15520,   485, -1464,  1339, -1464,     5,   529, -1464, -1464,
   -1464, -1464, 13999,   899,   831, -1464, -1464, -1464,    50, 12760,
     744,   749, 16089,   755,  2015,   761,  5426, 12760,    69,   757,
     693,    69,   470,   455, -1464,  1339,  2865,   751,  9865, 13662,
   -1464, -1464,   661, -1464, -1464, -1464, -1464, -1464,   683, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, 12760, 12760, 12760, 11795,
   12760, 12760, 12760, 12760, 12760, 12760, 12760, 12760, 12760, 12760,
   12760, 12760, 12760, 12760, 12760, 12760, 12760, 12760, 12760, 12760,
   12760, 12760,  3635, 12760, -1464, 12760, 12760, 12760, 13115,  1339,
    1339,  1339,  1339,  1339,  1116,   843,   685,  9672, 12760, 12760,
   12760, 12760, 12760, 12760, 12760, 12760, 12760, 12760, 12760, 12760,
   -1464, -1464, -1464, -1464,   999, 12760, 12760, -1464,  9865,  9865,
   12760, 12760,   467,   153, 15237,   765,   683, 11988, 14445, -1464,
   12760, -1464,   767,   946,   812,   774,   775, 13248,    21, 12181,
   -1464, 12374, -1464,   777,   781,  2044, -1464,   248,  9865, -1464,
    1043, -1464, -1464, 14490, -1464, -1464, 10058, -1464, 12760, -1464,
     875,  8707,   972,   799,  4716,   974,    82,    49, -1464, -1464,
   -1464,   818, -1464, -1464, -1464,  2865,   514,   803,   983,  4171,
    1339, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,   809,
   -1464, -1464,  1339,   105, -1464,   237,  1339,   108, -1464,   251,
     412,  1973, -1464,  1339, 12760,    21,    40, -1464, -1464, -1464,
    4171,   916, -1464,    21,    97,   125,   810,   811,  2140,   181,
     815,   816,   328,   871,   819,    21,   126,   824, -1464,  1314,
    1339, -1464, -1464,   939,  2577,   387, -1464, -1464, -1464,   529,
   -1464, -1464, -1464,   985,   888,   849,   417,   877, 12760,   467,
     895,  1017,   842,   879, -1464,   153, -1464,  2865,  2865,  1018,
     866,    50, -1464,   850,  1036, -1464,  2865,    92, -1464,   459,
     215, -1464, -1464, -1464, -1464, -1464, -1464, -1464,  2064,  2746,
   -1464, -1464, -1464, -1464,  1049,   886, -1464, 15520, 12760,   873,
    1052, 16089,  1050, -1464, -1464,   937,   860, 11201, 16262, 16339,
   12760, 12938,  4344, 13116,  9845, 10809, 12159, 12352, 12352, 12352,
   12352,  3408,  3408,  3408,  3408,  3408,  1279,  1279,   704,   704,
     704,   294,   294,   294, -1464,   770, 16089,   874,   878, 15726,
     880,  1060,   -18, 12760,    -9,   669,   369,   153, -1464, -1464,
   -1464,  1058,   831, -1464,   683, 15334, -1464, -1464, 16339, 16339,
   16339, 16339, 16339, 16339, 16339, 16339, 16339, 16339, 16339, 16339,
   16339, -1464, 12760,     2,   157, -1464, -1464,   669,   199,   882,
    2791,   896,   897,   889,  3302,   127,   887, -1464, 16089,  2589,
   -1464,  1339, -1464,    92,   484, 15520, 16089, 15520, 15783,    92,
      21,   165,   930,   900, 12760, -1464,   258, -1464, -1464, -1464,
    8321,    89, -1464, -1464, 16089, 16089,    55, -1464, -1464, -1464,
   12760,   990, 15051,  4171,  1339,  8900,   901,   902, -1464,    73,
    1006,   964,   948, -1464,  1091,   914,  2920,  2865,  4171,  4171,
    4171,  4171,  4171,   918,   957,   923,  4171,   205, -1464,   959,
   -1464,   922, -1464, 16179, -1464,     6, -1464,  5619,  1260,   925,
     425,  1564, -1464,  1339,   456,  1846, -1464,  1339,  1339, -1464,
   -1464,  3512, -1464, 16179,  1102, 15520,   934, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464,    78,  1339,  1260,   935,
   15237, 15427,  1111, -1464, -1464, -1464, -1464,   938, -1464, 12760,
   -1464, -1464,  4841, -1464,  2865,  1260,   943, -1464, -1464, -1464,
   -1464,  1113,   958, 12760, 13999, -1464, -1464, 13115,   947, -1464,
    2865, -1464,   960,  5812,  1117,   139, -1464, -1464,   174,   999,
   -1464,  1043, -1464,  2865, -1464, -1464,    21, 16089, -1464, 10251,
   -1464,  4171,   107,   962,  1260,   888, -1464, -1464, 16411, 12760,
   -1464, -1464, 12760, -1464, 12760, -1464,  3834,   966,  9865,   871,
    1124,   888,  2865,  1132,   937,  1339,  3635,    21,  4020,   973,
   -1464, -1464,   216,   977, -1464, -1464,  1150,  1101,  1101,  2589,
   -1464, -1464, -1464,  1120,   979,    59,   980, -1464, -1464, -1464,
   -1464,  1162,   987,   767,    21,    21, 12567,  1043, -1464, -1464,
    4444,   569,    55,  9479, -1464,  6005,   984,  6198,   992, 15051,
   15520,   989,  1053,    21, 16179,  1172, -1464, -1464, -1464, -1464,
     624, -1464,   346,  2865, -1464,  1055,  2865,  1339,   514, -1464,
   -1464, -1464,  1180, -1464,  1004,  1049,   732,   732,  1126,  1126,
   15885,  1000,  1187,  4171, 14858, 13999, 14535, 14725,  4171,  4171,
    4171,  4171, 14958,  4171,  4171,  4171,  4171,  4171,  4171,  4171,
    4171,  4171,  4171,  4171,  4171,  4171,  4171,  4171,  4171,  4171,
    4171,  4171,  4171,  4171,  4171,  4171,  1339, -1464, -1464,  1127,
   -1464, -1464,  1339, -1464, -1464,  1339, -1464, -1464, -1464, -1464,
    4171,    21, -1464,   328, -1464,   430,  1199, -1464, -1464,   129,
    1023,    21, 10444, -1464,  2459, -1464,  5040,   831,  1199, -1464,
     497,   351, -1464, 16089,  1078,  1027, -1464,  1029,  1117, -1464,
    2865,   866,  2865,    70,  1204,  1139,   278, -1464,   669,   297,
   -1464, -1464, 15520, 12760, 16089, 16179,  1033,   107, -1464,  1035,
     107,  1040, 16411, 16089, 15828,  1042,  9865,  1056,  1041,  2865,
    1048,  1034,  2865,   888, -1464,   691,   508,  9865, 12760, -1464,
   -1464, -1464, -1464, -1464, -1464,  1097,  1047,  1235,  1164,  2589,
    1104, -1464, 13999,  2589, -1464, -1464, -1464, 15520, 16089, -1464,
      55,  1223,  1175,  9479, -1464, -1464, -1464,  1066, 12760,  1053,
      21, 15237, 15051,  1068,  4171,  6391,   632,  1069, 12760,    56,
     403, -1464,  1085,  2865, -1464,  1129, -1464,  3949,  1236,  1080,
    4171, -1464,  4171, -1464,  1081, -1464,  1133,  1261,  1082, -1464,
   -1464, -1464, 15930,  1086,  1273, 16221, 16303, 16375,  4171, 16136,
    4479, 15211, 10231, 11967, 12545, 13249, 13249, 13249, 13249,  3018,
    3018,  3018,  3018,  3018,  1483,  1483,   732,   732,   732,  1126,
    1126,  1126,  1126, -1464,  1096, -1464, -1464, -1464, 16179,  1339,
    2865,  2865, -1464,  1260,    84, -1464, 15237, -1464, -1464, 16339,
    1095, -1464,  1099,   582, -1464,   121, 12760, -1464, -1464, -1464,
   12760, -1464, 12760, -1464,   866, -1464, -1464,   176,  1275,  1215,
   12760, -1464,  1114,    21, 16089,  1117,  1112, -1464,  1119,   107,
   12760,  9865,  1122, -1464, -1464,   831, -1464, -1464,  1115,  1123,
    1118, -1464,  1128,  2589, -1464,  2589, -1464, -1464,  1135, -1464,
    1179,  1136,  1304, -1464,    21,  1284, -1464,  1131, -1464, -1464,
    1145,  1148,   132, -1464, -1464, 16179,  1149,  1151, -1464, 11587,
   -1464, -1464, -1464, -1464, -1464, -1464,  2865, -1464,  2865, -1464,
   16179, 15987, -1464,  4171, 13999, -1464, -1464,  4171, -1464,  4171,
   -1464, 16447,  4171,  1147,  6584,   430, -1464, -1464, -1464,   576,
   13800,  1260,  1212, -1464,   654,  1181,   561, -1464, -1464, -1464,
     843,  2159,   109,   113,  1154,   831,   685,   133, -1464, -1464,
   -1464,  1192, 10622, 11008, 16089, -1464,   200,  1336,  1276, 12760,
   -1464, 16089,  9865,  1240,  1117,   936,  1117,  1168, 16089,  1170,
   -1464,  1573,  1173,  1833, -1464, -1464,   107, -1464, -1464,  1229,
   -1464,  2589, -1464, 13999, -1464, -1464,  8321, -1464, -1464, -1464,
   -1464,  9093, -1464, -1464, -1464,  8321, -1464,  1176,  4171, 16179,
    1232, 16179, 16032, 16447, -1464, -1464, -1464,  1260,  1260,  1339,
   -1464,  1355, 14858,    61, -1464, 13800,   831,  1517, -1464,  1198,
   -1464,   114,  1184,   115, -1464, 14105, -1464, -1464, -1464,   116,
   -1464, -1464,   817, -1464,  1195, -1464,  1305,   683, -1464, 13938,
   -1464, 13938, -1464, -1464,  1370,   843, -1464, 13386, -1464, -1464,
   -1464, -1464,  1372,  1307, 12760, -1464, 16089,  1202,  1205,  1208,
     589, -1464,  1240,  1117, -1464, -1464, -1464, -1464,  1867,  1206,
    2589, -1464,  1266,  8321,  9286,  9093, -1464, -1464, -1464,  8321,
   -1464, 16179,  4171,  4171,  6777,  1216,  1217, -1464,  4171, -1464,
    1260, -1464, -1464, -1464, -1464, -1464,  2865,  1730,   654, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
     623, -1464,  1181, -1464, -1464, -1464, -1464, -1464,   123,   601,
   -1464,  1392,   117, 14592,  1305,  1394, -1464,  2865,   683, -1464,
   -1464,  1222,  1400, 12760, -1464, 16089, -1464,   453, -1464, -1464,
   -1464, -1464,  1238,   589, 13524, -1464,  1117, -1464,  2589, -1464,
   -1464, -1464, -1464,  6970, 16179, 16179, -1464, -1464, -1464, 16179,
   -1464,  1219,   122,  1415,  1241, -1464, -1464,  4171, 14105, 14105,
    1378, -1464,   817,   817,   664, -1464, -1464, -1464,  4171,  1356,
   -1464,  1263,  1251,   118,  4171, -1464,  1339, -1464,  4171, 16089,
    1360, -1464,  1434,  7163,  7356, -1464, -1464, -1464,   589, -1464,
    7549,  1256,  1334, -1464,  1348,  1300, -1464, -1464,  1354,  2865,
   -1464,  1730, -1464, -1464, 16179, -1464, -1464,  1290, -1464,  1426,
   -1464, -1464, -1464, -1464, 16179,  1447,   328, -1464, -1464, 16179,
    1278, 16179, -1464,   481,  1281, -1464, -1464,  7742, -1464,  1277,
   -1464,  1280,  1302,  1339,   685,  1306, -1464, -1464,  4171,   135,
     112, -1464,  1401, -1464, -1464, -1464, -1464,  1260,   925, -1464,
    1318,  1339,   480, -1464, 16179, -1464,  1293,  1480,   711,   112,
   -1464,  1424, -1464,  1260,  1319, -1464,  1117,   137, -1464, -1464,
   -1464, -1464,  2865, -1464,  1322,  1332,   119, -1464,   606,   711,
     192,  1117,  1317, -1464, -1464, -1464, -1464,  2865,   281,  1510,
    1445,   606, -1464,  7935,   193,  1512,  1448, 12760, -1464, -1464,
    8128, -1464,   330,  1513,  1457, 12760, -1464, 16089, -1464,  1524,
    1459, 12760, -1464, 16089, 12760, -1464, 16089, 16089
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1464, -1464, -1464,  -535, -1464, -1464, -1464,   473,    45,   -33,
   -1464, -1464, -1464,   981,   729,   733,    20,  1560,  2651, -1464,
    2607, -1464,  -359, -1464,    18, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464, -1464, -1464, -1464, -1464,  -175, -1464, -1464,  -146,
     111,    24, -1464, -1464, -1464, -1464, -1464, -1464,    29, -1464,
   -1464, -1464, -1464,    30, -1464, -1464,  1090,  1098,  1103,   -89,
     641,  -828,   648,   706,  -174,   424,  -884, -1464,   104, -1464,
   -1464, -1464, -1464,  -704,   275, -1464, -1464, -1464, -1464,  -163,
   -1464,  -575, -1464,  -418, -1464, -1464,  1002, -1464,   120, -1464,
   -1464,  -995, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464,    95, -1464,   163, -1464, -1464, -1464, -1464, -1464,
      -1, -1464,   246,  -781, -1464, -1463,  -182, -1464,  -139,    93,
    -120,  -167, -1464,     7, -1464, -1464, -1464,   262,   -34,     0,
      42,  -701,   -61, -1464, -1464,   -12, -1464, -1464,    -5,   -38,
     142, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464, -1464,
    -582,  -813, -1464, -1464, -1464, -1464, -1464,  1156, -1464, -1464,
   -1464, -1464, -1464,   537, -1464, -1464, -1464, -1464, -1464, -1464,
   -1464, -1464,  -768, -1464,  2114,    23, -1464,   526,  -379, -1464,
   -1464,   494,  3159,  3413, -1464, -1464,   596,  -160,  -621, -1464,
   -1464,   670,   482,  -680,   486, -1464, -1464, -1464, -1464, -1464,
     658, -1464, -1464, -1464,    94,  -827,  -127,  -397,  -394, -1464,
     724,  -111, -1464, -1464,    27,    38,   160, -1464, -1464,  1011,
     -28, -1464,  -332,    77,   289, -1464,  -299, -1464, -1464, -1464,
    -449,  1249, -1464, -1464, -1464, -1464, -1464,   750,  1100, -1464,
   -1464, -1464,  -330,  -677, -1464,  1210, -1022, -1464,   -65,  -148,
     128,   821, -1464,  -975,   295,   -75,   570,   634, -1464, -1464,
   -1464, -1464,   591,   222, -1036
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -967
static const yytype_int16 yytable[] =
{
     175,   177,   409,   179,   180,   181,   183,   184,   455,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   319,   115,   213,   216,   913,   370,   485,   117,   890,
     373,   374,   235,   119,   120,   735,  1237,   238,   871,   381,
     614,   477,   616,   326,   240,   246,   872,   249,  1072,   244,
     327,  1064,   330,   221,   409,   502,   405,   226,   454,   940,
     731,   732,   383,   684,  1223,   852,   380,  1089,   227,   958,
    1480,   506,   385,   972,   238,  1139,   -65,   724,   336,   776,
     725,   -65,   973,  1100,   237,   399,  1290,  1043,  1319,   -30,
     753,   774,   518,  1234,   -30,   382,    13,   122,   147,   318,
     -29,   366,   510,   475,   367,   -29,   825,   228,   561,   566,
     571,   518,   375,   756,   811,   116,   757,   815,  1429,   511,
     337,   356,  1431,  -341,  1488,  1572,  1639,  1639,  1480,   494,
      13,  1669,  1632,   383,   518,   841,   841,   380,   841,   563,
    1656,   841,   841,   385,  1043,  1120,   345,   688,   549,   357,
    1238,   472,   473,    13,   -92,   760,  1633,  1026,    13,   475,
     395,   952,   953,   -91,   151,   496,   382,     3,   -92,   495,
    -822,   178,  1341,   488,  -796,   385,   729,   -91,   396,  -652,
     504,   733,  1166,    13,   503,    13,   209,   211,  -796,   807,
     231,   359,  1320,   601,   602,  1698,   395,  1321,   382,    60,
      61,    62,   165,  1322,   406,  1323,   550,  1121,  -821,  1073,
    -824,  1346,  -864,   382,  -833,   376,  -823,  -653,   360,   361,
     480,   377,   472,   473,  -825,   476,   513,  1758,  1772,   513,
     590,   338,  -867,   408,  -280,  1239,   238,   524,   777,   535,
     456,   234,  1324,  1325,  1291,  1326,   974,  -517,  -543,  1481,
    1482,  1044,  1355,   515,  1074,   -65,  1347,   520,  1283,  1361,
     384,  1363,   656,   873,   400,  1146,   407,  1150,   -30,   775,
    1353,   519,  1759,  1773,  1327,  1259,   324,   371,   480,   -29,
    1442,   476,  -828,  -827,   826,   486,   199,   562,   567,   572,
     588,   545,  -822,   812,  -280,  1670,   816,  1430,  1401,  -264,
    -659,  1432,  -341,  1489,  1573,  1640,  1688,  1753,  1725,  1270,
    1634,   577,   827,   842,   926,   576,  1215,  1086,  1056,  1380,
    1436,   580,  -544,  -741,  -741,  -866,  -741,  -831,   199,  -830,
    -821,   384,  -824,   742,  -864,   813,   409,   654,  -823,  1075,
     736,  1348,   481,   238,   382,  -808,  -825,  -541,   442,   817,
     213,   555,   559,   560,  -867,   594,  -939,  1760,  1774,   241,
     443,  1765,  -654,   384,  -809,  1443,   319,   479,   831,   318,
    1458,  -799,   498,  -543,   183,   834,   835,   396,   505,  1450,
    1222,  1452,   639,  1483,   599,  -799,   370,   701,   702,   405,
     575,  1049,   395,   651,  -939,   151,   706,   482,   242,   151,
     481,  -834,  -837,   955,  -828,  -827,   395,  1585,   957,  1586,
    1779,   657,   658,   659,   661,   662,   663,   664,   665,   666,
     667,   668,   669,   670,   671,   672,   673,   674,   675,   676,
     677,   678,   679,   680,   681,   682,   683,  1280,   685,   103,
     686,   686,   689,  1273,   318,   707,  1766,  -866,  1148,  1149,
     483,   865,   708,   709,   710,   711,   712,   713,   714,   715,
     716,   717,   718,   719,   720,   221,  -832,  -808,   858,   226,
     686,   730,   625,   651,   651,   686,   734,   108,  1603,   122,
     227,   533,   708,   534,  1079,   738,  -809,  1080,  1650,  1236,
    1097,  1390,   479,   694,   746,  1780,   748,   116,   565,   455,
     879,   243,   881,   651,   763,  1148,  1149,   573,  1229,   578,
     818,   764,  1246,   765,   585,  1248,  1712,   472,   473,   228,
    -660,   595,   343,  1032,   247,   696,  -545,   317,  1210,  1211,
     344,   905,   614,  1651,   616,   357,  1151,  1105,   538,   768,
    1230,   596,   318,   780,   351,   589,   151,   907,   328,   454,
     357,   728,   207,   207,  1035,   908,   596,   357,   915,   821,
    1462,  1713,   368,   358,   351,   357,   704,   395,   351,   351,
     862,   596,   810,   859,   696,   347,   814,   851,  1362,   919,
     395,   336,   348,  1368,   752,  1369,  1319,   758,   860,   472,
     473,   897,    36,  1293,   351,    60,    61,    62,   165,   166,
     406,   781,   349,   382,   360,   361,   350,  1145,   472,   473,
    -655,   395,   354,    48,   472,   473,   879,   881,   646,   360,
     361,   324,   947,   881,   979,   359,   360,   361,    13,   341,
    1635,  1658,   355,   597,   360,   361,   342,  1627,   948,    36,
     357,  1130,  1131,   887,   585,  -835,   388,   371,  1636,   506,
     642,  1637,  1345,  1628,  1224,   898,   387,  1027,  1434,   492,
      48,   615,   407,   614,   641,   616,   169,  1225,  1735,    84,
    -835,  1629,    86,    87,  1357,    88,   170,    90,  1252,   372,
    -797,   151,   357,  1407,   395,  1421,  1226,   394,   906,  1262,
    1320,  1461,  1282,  1681,  -797,  1321,   445,    60,    61,    62,
     165,  1322,   406,  1323,  1397,  1398,  1750,  -939,   108,   360,
     361,  1682,   108,   398,  1683,   625,   525,   918,   401,    86,
      87,  1764,    88,   170,    90,  1147,  1148,  1149,   396,  1484,
     410,   456,    36,  1287,  1148,  1149,   411,   697,   207,    36,
    1324,  1325,  -939,  1326,   446,  -939,   412,  1422,   447,   950,
     591,   360,   361,    48,   439,   440,   441,   779,   442,   413,
      48,  1748,   414,   697,   407,   238,   956,  1337,   357,   415,
     443,   448,  1340,   357,   391,  1408,  1761,  1599,  1600,   596,
    1607,  1124,  1022,  1023,  1024,   478,   697,  -542,  1409,  1410,
     614,  1459,   616,   967,  1754,  1755,   697,   544,  1025,   697,
      60,    61,    62,   165,   166,   406,   169,   886,    36,    84,
    1411,  -829,    86,    87,  -653,    88,  1412,    90,   489,    86,
      87,   484,    88,   170,    90,  1159,  1739,  1740,  1741,    48,
     364,   555,  1163,  1359,   491,   559,   443,   360,   361,   868,
     869,   396,   360,   361,  1054,   497,  1314,   653,   877,   500,
    1104,  1679,  1680,   122,   390,   392,   393,  -833,  1063,   108,
    1675,  1676,   479,   501,   207,   917,  -651,   407,   643,   507,
     115,   116,   317,   207,   516,   351,   117,   508,   529,  -966,
     207,   119,   120,  1728,  1084,   364,   536,   207,    86,    87,
     539,    88,   170,    90,  1092,    36,   540,  1093,   613,  1094,
     122,   546,  1728,   651,   568,   944,   569,   945,   570,   581,
    1749,   582,   694,   617,   982,   985,    48,  1376,   116,   365,
     151,   544,   351,   699,   351,   351,   351,   351,  1659,   618,
    1242,   627,   963,  1385,  1447,   151,   628,  -117,    36,   221,
    1319,  1128,   629,   226,  1065,   122,   147,   723,   631,   640,
      53,   653,  1133,   739,   227,   741,   728,  1266,   758,    48,
     591,   743,   744,   116,   749,   766,   122,   151,   750,   625,
     544,  1134,   614,  1574,   616,    86,    87,  1575,    88,   170,
      90,   518,    13,   755,   116,  1041,   625,   770,   773,   786,
     535,  1217,   787,   228,   108,   808,   824,   828,   829,   838,
     585,  1051,   832,  1422,   833,   840,   849,  1305,   646,   646,
     207,   843,   151,   805,  1310,   854,  1463,   855,    86,    87,
     857,    88,   170,    90,   758,  1469,   864,   863,  -675,   866,
     867,  1694,   870,   151,   874,  1474,   820,   210,   210,  1165,
    1218,   614,  1171,   616,  1320,   875,   896,  1219,   122,  1321,
     122,    60,    61,    62,   165,  1322,   406,  1323,   883,   885,
     888,   889,   846,   848,   891,   894,   116,   900,   116,   904,
     903,   901,   912,   896,   115,   920,  1057,    36,  1244,   199,
     117,  -657,   924,   922,   923,   119,   120,   949,   959,   969,
     971,   651,  1067,   975,  1324,  1325,   976,  1326,    48,   977,
     978,   980,   651,  1219,   993,  1081,  1738,  1613,   994,   995,
     997,   998,  1374,   151,  1030,   151,  1040,   151,   407,   963,
    1140,    36,  1042,   199,  1048,  1052,  1451,  1061,    53,   351,
    1275,  1053,  1059,   238,  1101,  1066,    60,    61,    62,   165,
     166,   406,    48,  1289,  1068,  1062,  1102,  1070,  1087,   122,
     147,  1278,  1096,   615,  1099,   721,   697,    86,    87,  1107,
      88,   170,    90,  -836,  1108,  1119,  1122,   116,   697,  1118,
     697,  1123,  1136,   207,  1125,  1141,  1109,  1110,  1111,    36,
    1138,   625,   205,   205,   625,  1142,  1144,  1153,   722,  1157,
     103,  1158,  1025,  1161,    36,  1152,  1162,  1435,  1154,   721,
      48,    86,    87,   407,    88,   170,    90,  1204,  1213,  1216,
    1232,   409,   937,   908,   943,    48,   151,  1233,  1240,  1241,
    1245,  1342,  1257,   210,  1247,  1343,  1249,  1344,  1251,  1263,
    1254,   207,   754,   108,   103,  1351,   697,  1256,   122,  1653,
    1264,  1654,  1243,  1253,  1265,  1358,   651,   965,   108,  1277,
    1660,   933,  1269,  1276,  1279,  1284,   116,  1288,  1294,    86,
      87,  1296,    88,   170,    90,  1303,  1298,  1299,  1302,  1306,
    1304,   207,   403,   207,    86,    87,  1308,    88,   170,    90,
     108,  1583,  1309,  1313,   615,  1338,  1034,  1274,  1339,  1349,
    1037,  1038,  1235,   151,   877,  1350,  1697,    36,   207,  1354,
    1352,   585,   963,  1364,  1366,   151,  1356,  1331,  1336,  1360,
    1045,  1371,  1365,  1373,  1375,  1367,  1331,  1336,    48,  1377,
    1405,  1255,  1370,  1372,  1258,   108,   436,   437,   438,   439,
     440,   441,  1378,   442,  1477,  1379,  1394,  1382,    36,  1383,
     544,  1418,  1433,   625,  1446,   443,   108,   651,  1438,   210,
    1444,   207,   723,  1448,   755,  1453,  1445,  1454,   210,    48,
     579,  1460,  1456,  1470,  1472,   210,   207,   207,   205,  1478,
    1486,   169,   210,  1487,    84,  1295,   585,    86,    87,  1081,
      88,   170,    90,  1580,  1587,  1581,  1593,  1594,   351,  1596,
     613,  1597,    36,  1606,   844,   845,  1763,  1598,  1608,  1467,
    1112,  1112,   937,  1770,  1617,  1618,  1638,   122,  1644,  1663,
    1647,   615,   169,    48,  1648,    84,    85,    36,    86,    87,
     755,    88,   170,    90,  1671,   116,   108,  1655,   108,   456,
     108,  1673,  1316,  1317,  1677,  1686,  1685,  1687,    48,  1595,
    1692,  1427,  1646,  1693,  1700,  1701,  -337,  1479,  1331,  1336,
    1155,  1703,  1704,  1707,  1331,  1336,  1331,  1336,   592,  1633,
     625,  1708,   598,  1672,  1711,  1716,  1717,   544,  1714,   122,
     544,  1718,    86,    87,   151,    88,   170,    90,   122,  1723,
    1736,  1730,  1610,  1467,  1733,   207,   207,   116,   592,  1737,
     598,   592,   598,   598,   205,   210,   116,    86,    87,   805,
      88,   170,    90,   205,  1745,  1206,  1762,  1747,  1207,  1751,
     205,    60,    61,    62,    63,    64,   406,   205,  1386,  1752,
    1387,   613,    70,   449,  1767,  1768,  1775,  1781,  1776,   108,
    1019,  1020,  1021,  1022,  1023,  1024,   151,  1782,  1784,  1785,
    1033,   151,   819,  1732,   703,   151,  1588,   698,  1036,  1025,
    1641,  1331,  1336,  1428,   700,  1103,   122,  1098,   450,  1746,
     451,  1058,   122,  1281,  1384,  1721,  1744,   122,   822,  1612,
    1485,  1631,  1425,   452,   116,   453,  1604,  1319,   407,  1769,
     116,  1690,  1757,  1626,  1406,   116,   204,   204,  1649,  1164,
     219,  1643,   937,   615,  1602,    36,   937,  1227,  1114,  1261,
    1267,  1126,   409,  1078,   587,  1268,   108,   983,   207,   652,
    1396,  1709,  1156,  1209,   219,     0,    48,  1203,   108,    13,
      36,     0,     0,   151,   151,   151,     0,     0,   318,   151,
       0,     0,     0,     0,   151,     0,     0,     0,  1408,     0,
     205,    48,    36,     0,     0,     0,     0,     0,   613,   331,
     332,  1409,  1410,   207,     0,     0,     0,     0,   210,     0,
       0,     0,   615,    48,     0,     0,     0,   207,   207,   169,
       0,     0,    84,    85,     0,    86,    87,     0,    88,  1412,
      90,  1320,  1315,     0,     0,     0,  1321,     0,    60,    61,
      62,   165,  1322,   406,  1323,     0,     0,   333,     0,     0,
      86,    87,     0,    88,   170,    90,   122,     0,  1622,     0,
       0,     0,     0,     0,     0,     0,   210,     0,     0,   552,
       0,     0,    86,    87,   116,    88,   170,    90,     0,     0,
       0,  1324,  1325,     0,  1326,     0,   937,     0,   937,     0,
       0,     0,   207,     0,     0,     0,   122,   122,     0,     0,
       0,     0,     0,   122,     0,   407,   210,     0,   210,     0,
       0,     0,  1777,  1455,   116,   116,     0,     0,     0,     0,
    1783,   116,   204,   151,     0,     0,  1786,     0,     0,  1787,
       0,     0,     0,   210,     0,     0,     0,   108,     0,     0,
     122,     0,     0,   317,     0,     0,     0,  1722,     0,  1423,
       0,     0,     0,   205,   625,    33,    34,    35,   116,  1645,
       0,     0,     0,   151,   151,     0,     0,   200,     0,   219,
     151,   219,     0,   625,     0,     0,     0,     0,     0,     0,
     613,   625,     0,     0,     0,     0,   210,  1319,     0,     0,
       0,     0,     0,     0,   937,     0,     0,     0,     0,   108,
       0,   210,   210,     0,   108,     0,   122,   151,   108,     0,
       0,   205,     0,   122,     0,     0,    74,    75,    76,    77,
      78,  1319,   351,     0,   116,   544,   219,   202,   317,    13,
       0,   116,     0,    82,    83,     0,     0,     0,  1569,     0,
       0,  1705,     0,     0,     0,  1576,     0,    92,   204,   613,
       0,   205,   317,   205,   317,     0,     0,   204,     0,     0,
     317,    97,     0,    13,   204,     0,     0,     0,     0,     0,
       0,   204,     0,   151,    36,     0,     0,     0,   205,     0,
     151,     0,   219,   937,     0,     0,   108,   108,   108,     0,
       0,  1320,   108,     0,     0,    48,  1321,   108,    60,    61,
      62,   165,  1322,   406,  1323,     0,   219,     0,     0,   219,
       0,     0,     0,     0,   877,     0,     0,     0,     0,     0,
     210,   210,     0,     0,     0,  1320,     0,     0,     0,   877,
    1321,   205,    60,    61,    62,   165,  1322,   406,  1323,     0,
       0,  1324,  1325,     0,  1326,     0,   205,   205,     0,     0,
       0,   556,   219,     0,    86,    87,     0,    88,   170,    90,
       0,     0,     0,     0,     0,   407,     0,     0,     0,     0,
       0,     0,     0,  1457,     0,  1324,  1325,     0,  1326,   487,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,     0,     0,   204,     0,     0,     0,     0,   407,
       0,    36,     0,     0,     0,     0,   544,  1605,   487,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,     0,    48,     0,     0,     0,     0,   317,     0,   470,
     471,   937,     0,     0,     0,     0,   108,     0,     0,     0,
       0,     0,     0,   210,  1664,   219,   219,     0,     0,   797,
       0,  1569,  1569,     0,     0,  1576,  1576,     0,   470,   471,
       0,     0,     0,     0,     0,   205,   205,     0,     0,   351,
       0,     0,     0,     0,     0,     0,   108,   108,   333,     0,
     797,    86,    87,   108,    88,   170,    90,     0,   210,     0,
     206,   206,     0,     0,   220,   472,   473,     0,     0,     0,
       0,     0,   210,   210,   487,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,     0,     0,     0,
     108,     0,     0,     0,   472,   473,  1720,   219,   219,    60,
      61,    62,    63,    64,   406,     0,   219,     0,   259,     0,
      70,   449,     0,     0,  1734,     0,     0,     0,     0,     0,
       0,     0,   630,     0,   470,   471,     0,   204,     0,     0,
       0,     0,     0,     0,     0,   261,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   210,   451,     0,
       0,   751,   259,     0,     0,     0,   108,    36,   205,     0,
       0,     0,     0,   108,     0,     0,   407,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,   261,
       0,     0,     0,     0,     0,   204,  -384,     0,     0,     0,
     472,   473,     0,     0,    60,    61,    62,   165,   166,   406,
       0,    36,     0,   205,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   531,   532,     0,     0,   205,   205,     0,
       0,     0,    48,     0,     0,   204,     0,   204,     0,     0,
     537,   169,     0,     0,    84,   311,     0,    86,    87,     0,
      88,   170,    90,     0,     0,     0,   206,   830,     0,     0,
       0,     0,   204,   797,     0,   315,     0,   531,   532,     0,
       0,   407,     0,     0,     0,   316,   219,   219,   797,   797,
     797,   797,   797,     0,     0,   169,   797,     0,    84,   311,
       0,    86,    87,     0,    88,   170,    90,     0,   219,     0,
       0,     0,   205,     0,     0,     0,     0,     0,     0,   315,
       0,     0,     0,     0,     0,   204,     0,     0,     0,   316,
       0,     0,     0,     0,     0,     0,     0,     0,   219,     0,
     204,   204,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   219,   219,     0,     0,     0,     0,
       0,     0,     0,     0,   219,     0,     0,     0,     0,     0,
     219,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,     0,     0,     0,     0,     0,     0,
       0,   797,   206,     0,   219,     0,     0,     0,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,   206,   416,
     417,   418,   219,     0,     0,   206,   219,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   206,     0,   419,     0,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,     0,   442,     0,     0,     0,     0,     0,   204,
     204,     0,     0,     0,     0,   443,     0,     0,     0,     0,
       0,     0,     0,   219,     0,     0,   219,     0,   219,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   797,     0,   219,   220,     0,   797,   797,
     797,   797,   797,   797,   797,   797,   797,   797,   797,   797,
     797,   797,   797,   797,   797,   797,   797,   797,   797,   797,
     797,   797,   797,   797,   797,   797,     0,   416,   417,   418,
       0,     0,     0,     0,     0,     0,     0,     0,   206,     0,
     797,     0,     0,     0,     0,     0,   419,     0,   420,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     219,   442,   219,     0,     0,     0,   927,   928,     0,     0,
       0,     0,   204,   443,     0,     0,     0,     0,     0,     0,
       0,     0,  1220,   801,     0,     0,   929,     0,     0,   219,
       0,   320,   219,     0,   930,   931,   932,    36,     0,     0,
       0,     0,     0,     0,     0,     0,   933,     0,     0,     0,
       0,     0,   219,     0,   801,     0,     0,   204,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   204,   204,     0,   797,     0,     0,     0,     0,     0,
       0,     0,     0,   219,     0,     0,     0,   219,     0,     0,
     797,     0,   797,   934,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   935,     0,   797,     0,
       0,     0,     0,     0,     0,     0,     0,    86,    87,     0,
      88,   170,    90,     0,     0,     0,   416,   417,   418,     0,
       0,   206,     0,     0,     0,   936,     0,   850,     0,     0,
     219,   219,     0,   219,     0,   419,   204,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,     0,
     442,   416,   417,   418,     0,     0,     0,     0,     0,     0,
       0,     0,   443,     0,     0,     0,     0,     0,     0,   206,
     419,     0,   420,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,     0,   442,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   219,   443,   219,   206,
       0,   206,     0,   797,   219,     0,   320,   797,   320,   797,
       0,     0,   797,     0,     0,     0,     0,     0,     0,     0,
     219,   219,     0,     0,   219,     0,   206,   801,     0,     0,
       0,   219,     0,     0,   259,     0,     0,     0,     0,     0,
       0,     0,   801,   801,   801,   801,   801,     0,     0,     0,
     801,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   261,  1029,   320,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,     0,     0,   882,     0,     0,   206,
       0,     0,     0,    36,     0,     0,     0,     0,   797,   259,
       0,     0,  1047,     0,   206,   206,     0,   219,   219,     0,
       0,     0,     0,     0,    48,   219,     0,   219,     0,  1047,
       0,     0,     0,     0,     0,     0,   261,     0,   206,     0,
       0,   921,     0,     0,     0,     0,     0,     0,     0,   219,
       0,   219,     0,     0,     0,     0,     0,   219,    36,   531,
     532,     0,     0,   320,     0,   801,   320,     0,  1088,     0,
       0,     0,     0,     0,     0,     0,     0,   169,     0,    48,
      84,   311,     0,    86,    87,     0,    88,   170,    90,     0,
     220,     0,   797,   797,     0,     0,     0,     0,   797,     0,
     219,   315,     0,     0,     0,     0,   219,     0,   219,     0,
       0,   316,     0,     0,   531,   532,     0,     0,  -967,  -967,
    -967,  -967,  -967,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,     0,   169,   206,   206,    84,   311,     0,    86,    87,
       0,    88,   170,    90,  1025,   981,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   315,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   316,   801,     0,   206,
       0,     0,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
       0,     0,   320,   782,     0,     0,   799,   219,     0,     0,
       0,     0,     0,     0,   801,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   219,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   799,     0,     0,
       0,   219,     0,     0,     0,   208,   208,   797,     0,   224,
     798,     0,     0,     0,     0,     0,   206,     0,   797,     0,
       0,     0,     0,     0,   797,     0,     0,     0,   797,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   798,     0,     0,   320,   320,     0,     0,     0,   219,
       0,     0,     0,   320,     0,     0,   206,     0,     0,     0,
       0,   206,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   206,   206,     0,   801,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   797,     0,
       0,     0,     0,     0,   801,     0,   801,   219,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   801,   219,     0,     0,     0,     0,     0,     0,
       0,     0,   219,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   416,   417,   418,     0,     0,   219,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1318,     0,     0,
     206,   419,     0,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,     0,   442,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   443,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
     799,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   320,   320,   799,   799,   799,   799,   799,
       0,     0,     0,   799,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   801,   206,     0,
       0,   801,     0,   801,   798,     0,   801,     0,     0,     0,
       0,     0,     0,     0,     0,  1404,     0,     0,  1417,   798,
     798,   798,   798,   798,     0,     0,     0,   798,  -967,  -967,
    -967,  -967,  -967,   434,   435,   436,   437,   438,   439,   440,
     441,   320,   442,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   443,     0,     0,   320,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   206,     0,     0,
     320,     0,   925,     0,     0,     0,     0,   208,   799,     0,
       0,     0,   801,     0,     0,     0,   208,     0,     0,     0,
       0,  1475,  1476,   208,     0,     0,     0,     0,     0,   320,
     208,  1417,   416,   417,   418,     0,     0,     0,     0,     0,
       0,   224,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   419,   798,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,     0,   442,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   443,     0,
     320,     0,     0,   320,     0,   782,   801,   801,     0,     0,
       0,     0,   801,     0,  1620,     0,     0,     0,     0,     0,
     799,   224,  1417,     0,     0,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   208,   798,     0,     0,   799,     0,   798,
     798,   798,   798,   798,   798,   798,   798,   798,   798,   798,
     798,   798,   798,   798,   798,   798,   798,   798,   798,   798,
     798,   798,   798,   798,   798,   798,   798,   320,     0,   320,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   798,     0,     0,     0,     0,     0,     0,   802,     0,
       0,     0,  1039,     0,     0,     0,   320,     0,     0,   320,
      33,    34,    35,    36,     0,   199,     0,     0,     0,     0,
       0,     0,   200,     0,     0,     0,     0,     0,     0,   802,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,   801,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   799,   801,     0,     0,   217,     0,     0,   801,     0,
     320,     0,   801,     0,   320,     0,     0,   799,     0,   799,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   202,     0,     0,   799,     0,   169,    82,    83,
      84,    85,     0,    86,    87,   798,    88,   170,    90,     0,
       0,     0,    92,     0,     0,     0,   208,     0,     0,     0,
       0,   798,     0,   798,     0,     0,    97,   320,   320,     0,
       0,   218,   801,     0,     0,     0,   103,     0,     0,   798,
       0,  1731,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   416,   417,   418,  1404,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   419,   208,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,     0,   442,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     443,     0,     0,   320,   208,   320,   208,     0,     0,     0,
     799,     0,     0,     0,   799,     0,   799,     0,     0,   799,
       0,     0,     0,     0,     0,     0,     0,   320,     0,     0,
       0,   208,   802,     0,     0,     0,     0,     0,   320,     0,
       0,     0,     0,     0,     0,     0,     0,   802,   802,   802,
     802,   802,   803,     0,   798,   802,     0,     0,   798,     0,
     798,     0,     0,   798,     0,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   259,     0,
       0,     0,     0,   823,   208,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   799,     0,     0,     0,   208,
     208,     0,     0,     0,     0,   261,     0,     0,     0,     0,
       0,     0,   320,     0,     0,   470,   471,     0,     0,     0,
       0,     0,     0,   224,  1095,     0,     0,    36,     0,     0,
     416,   417,   418,     0,     0,     0,   320,     0,   320,   798,
       0,     0,     0,     0,   320,     0,     0,     0,    48,   419,
     802,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,     0,   442,   224,     0,     0,     0,   799,
     799,   472,   473,   531,   532,   799,   443,     0,     0,     0,
       0,     0,     0,   320,     0,     0,     0,     0,     0,     0,
       0,   169,     0,     0,    84,   311,     0,    86,    87,     0,
      88,   170,    90,     0,  1297,     0,     0,     0,   208,   208,
       0,     0,     0,   798,   798,   315,     0,     0,     0,   798,
       0,     0,     0,     0,     0,   316,     0,   487,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
       0,     0,   802,     0,   224,     0,     0,   802,   802,   802,
     802,   802,   802,   802,   802,   802,   802,   802,   802,   802,
     802,   802,   802,   802,   802,   802,   802,   802,   802,   802,
     802,   802,   802,   802,   802,     0,   964,   470,   471,     0,
       0,     0,     0,     0,   320,     0,     0,     0,     0,   802,
       0,   986,   987,   988,   989,     0,     0,     0,     0,   996,
    1106,   320,     0,     0,     0,     0,     0,     0,   788,   789,
       0,     0,     0,     0,   790,     0,   791,     0,  1665,     0,
       0,     0,     0,     0,   799,     0,     0,     0,   792,     0,
       0,   208,     0,     0,     0,   799,    33,    34,    35,    36,
       0,   799,     0,   472,   473,   799,     0,     0,   200,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,   320,     0,   798,     0,
       0,   224,     0,     0,     0,     0,   208,     0,     0,   798,
       0,     0,     0,     0,     0,   798,     0,     0,     0,   798,
     208,   208,     0,   802,  1085,   793,     0,    74,    75,    76,
      77,    78,     0,     0,     0,   799,     0,     0,   202,   802,
       0,   802,     0,   169,    82,    83,    84,   794,     0,    86,
      87,     0,    88,   170,    90,     0,     0,   802,    92,     0,
       0,     0,     0,     0,     0,     0,     0,   795,     0,   320,
       0,     0,    97,     0,     0,     0,     0,   796,     0,   798,
       0,     0,     0,     0,   320,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   208,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,     0,   442,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     443,  1172,  1175,  1176,  1177,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,  1188,  1189,  1190,  1191,  1192,  1193,
    1194,  1195,  1196,  1197,  1198,  1199,  1200,  1201,  1202,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1208,   416,   417,   418,     0,     0,     0,
       0,     0,   802,   224,     0,     0,   802,     0,   802,     0,
       0,   802,     0,   419,     0,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,     0,   442,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     443,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,   224,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1025,     0,   802,     0,     0,
       0,     5,     6,     7,     8,     9,     0,  1285,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1300,     0,  1301,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1311,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,   802,   802,     0,  1129,    41,     0,   802,     0,     0,
       0,     0,     0,     0,     0,     0,  1625,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   165,   166,   167,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   168,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     169,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     170,    90,     0,     0,     0,    92,  1389,     0,    93,     0,
    1391,     0,  1392,     0,    94,  1393,   416,   417,   418,    97,
      98,    99,     0,     0,   171,     0,   325,     0,     0,   103,
     104,     0,   105,   106,   771,   419,     0,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,     0,
     442,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   443,     0,     0,     0,   802,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   802,     0,     0,
       0,  1471,     0,   802,     0,     0,     0,   802,     0,     0,
       0,     0,     0,     0,   772,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1706,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   802,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1614,  1615,    13,    14,    15,
       0,  1619,     0,     0,    16,     0,    17,    18,    19,    20,
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
     102,  1055,   103,   104,     0,   105,   106,     0,     0,     0,
    1674,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,  1684,     0,    10,     0,     0,     0,  1689,     0,     0,
       0,  1691,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,  1724,    39,    40,     0,     0,     0,    41,    42,    43,
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
    1221,   103,   104,     0,   105,   106,     5,     6,     7,     8,
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
      79,     0,     0,    80,     0,     0,     0,     0,   169,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   170,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,   632,   103,   104,     0,
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
       0,   169,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   170,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1028,
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
       0,     0,     0,     0,   169,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   170,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,  1069,   103,   104,     0,   105,   106,     5,     6,
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
       0,     0,    80,     0,     0,     0,     0,   169,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   170,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1135,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,  1137,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     169,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     170,    90,    91,     0,     0,    92,     0,     0,    93,     0,
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
      43,    44,     0,    45,     0,    46,     0,    47,  1286,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,    79,     0,     0,    80,     0,
       0,     0,     0,   169,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   170,    90,    91,     0,     0,    92,     0,
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
       0,    80,     0,     0,     0,     0,   169,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   170,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,  1395,   103,   104,     0,   105,   106,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   169,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   170,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,  1616,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,  1661,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,     0,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   169,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   170,    90,    91,     0,     0,    92,     0,     0,
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
      80,     0,     0,     0,     0,   169,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   170,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,  1695,   103,   104,     0,   105,   106,     5,
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
      79,     0,     0,    80,     0,     0,     0,     0,   169,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   170,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,  1696,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,  1699,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   169,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   170,    90,    91,     0,     0,    92,     0,     0,    93,
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
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
      63,    64,    65,     0,    66,    67,    68,     0,    70,    71,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   169,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   170,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,  1715,   103,   104,     0,   105,   106,     5,     6,
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
       0,     0,    80,     0,     0,     0,     0,   169,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   170,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1771,   103,   104,     0,   105,
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
     169,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     170,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   101,   102,  1778,   103,
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
       0,     0,     0,   169,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   170,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,   514,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,     0,    50,    51,
      52,    53,     0,    55,    56,     0,    57,     0,    59,    60,
      61,    62,   165,   166,    65,     0,    66,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,   169,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   170,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   767,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,   165,   166,    65,     0,    66,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   169,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   170,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     966,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,   165,   166,
      65,     0,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   169,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   170,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,  1466,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,   165,   166,    65,     0,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   169,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   170,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,  1609,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,   165,   166,    65,     0,    66,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   169,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   170,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
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
       0,    57,     0,    59,    60,    61,    62,   165,   166,    65,
       0,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   169,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   170,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   379,
      12,     0,     0,     0,     0,     0,     0,   705,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     165,   166,   167,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   168,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   169,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   170,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
       0,     0,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,     0,   442,
     647,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   443,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   165,   166,   167,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   168,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   169,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   170,    90,     0,
     648,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   171,     0,     0,     0,     0,   103,   104,     0,   105,
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
       0,     0,     0,    60,    61,    62,   165,   166,   167,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   168,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     169,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     170,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   171,     0,     0,   762,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,     0,     0,  1082,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1025,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   165,
     166,   167,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   168,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   169,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   170,    90,     0,  1083,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   171,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   379,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   165,   166,   167,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   168,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   169,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   170,    90,     0,     0,
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
       0,   182,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   165,   166,   167,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     168,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   169,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   170,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,  1440,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   171,     0,     0,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,     0,   442,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   443,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   165,   166,
     167,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   168,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   169,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   170,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   171,     0,   416,   417,
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
      62,   165,   166,   167,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   168,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   169,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   170,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,  1441,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   171,
       0,   245,   417,   418,   103,   104,     0,   105,   106,     5,
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
       0,    60,    61,    62,   165,   166,   167,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   168,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   169,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   170,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   171,     0,   248,     0,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   379,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   165,   166,   167,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   168,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   169,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   170,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   416,   417,   418,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   419,  1290,   420,   421,
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
     165,   166,   167,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   168,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   169,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   170,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,  1291,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   171,   512,
       0,     0,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   660,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   165,   166,   167,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   168,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   169,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   170,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   171,     0,     0,     0,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
       0,     0,     0,   705,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1025,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   165,   166,   167,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   168,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     169,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     170,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   171,     0,     0,     0,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,     0,   442,     0,     0,   745,     0,     0,     0,
       0,     0,     0,     0,     0,   443,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   165,
     166,   167,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   168,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   169,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   170,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   171,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  -967,  -967,
    -967,  -967,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,     0,   442,     0,     0,   747,
       0,     0,     0,     0,     0,     0,     0,     0,   443,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   165,   166,   167,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   168,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   169,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   170,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     171,     0,     0,     0,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,     0,     0,
       0,     0,  1127,     0,     0,     0,     0,     0,     0,     0,
       0,  1025,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   165,   166,   167,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     168,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   169,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   170,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   171,     0,   416,   417,   418,   103,   104,
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
       0,     0,     0,     0,     0,    60,    61,    62,   165,   166,
     167,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   168,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   169,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   170,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,   444,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   171,     0,   416,   417,
     418,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   419,   899,   420,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,     0,   442,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   443,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,   593,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   165,   166,   167,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   168,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   169,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   170,    90,     0,   250,   251,
      92,   252,   253,    93,     0,   254,   255,   256,   257,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   171,
       0,     0,   258,     0,   103,   104,     0,   105,   106,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   260,
     442,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   443,   262,   263,   264,   265,   266,   267,   268,
       0,     0,     0,    36,     0,   199,     0,     0,     0,     0,
       0,     0,     0,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,    48,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,     0,     0,
       0,   692,   304,   305,   306,     0,     0,     0,   307,   541,
     542,   250,   251,     0,   252,   253,     0,     0,   254,   255,
     256,   257,     0,     0,     0,     0,     0,   543,     0,     0,
       0,     0,     0,    86,    87,   258,    88,   170,    90,   312,
       0,   313,     0,     0,   314,  -967,  -967,  -967,  -967,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,   260,     0,   693,     0,   103,     0,     0,     0,
       0,     0,     0,     0,     0,  1025,   262,   263,   264,   265,
     266,   267,   268,     0,     0,     0,    36,     0,   199,     0,
       0,     0,     0,     0,     0,     0,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,    48,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,     0,     0,     0,   303,   304,   305,   306,     0,     0,
       0,   307,   541,   542,     0,     0,     0,     0,     0,   250,
     251,     0,   252,   253,     0,     0,   254,   255,   256,   257,
     543,     0,     0,     0,     0,     0,    86,    87,     0,    88,
     170,    90,   312,   258,   313,   259,     0,   314,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   693,     0,   103,
     260,     0,   261,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   262,   263,   264,   265,   266,   267,
     268,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    48,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,     0,
       0,     0,     0,   304,   305,   306,     0,     0,     0,   307,
     308,   309,     0,     0,     0,     0,     0,   250,   251,     0,
     252,   253,     0,     0,   254,   255,   256,   257,   310,     0,
       0,    84,   311,     0,    86,    87,     0,    88,   170,    90,
     312,   258,   313,   259,     0,   314,     0,     0,     0,     0,
       0,     0,   315,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   316,     0,     0,     0,  1589,     0,   260,     0,
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
     311,     0,    86,    87,     0,    88,   170,    90,   312,   258,
     313,   259,     0,   314,     0,     0,     0,     0,     0,     0,
     315,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     316,     0,     0,     0,  1657,     0,   260,     0,   261,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     262,   263,   264,   265,   266,   267,   268,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,    48,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,     0,     0,     0,   303,   304,
     305,   306,     0,     0,     0,   307,   308,   309,     0,     0,
       0,     0,     0,   250,   251,     0,   252,   253,     0,     0,
     254,   255,   256,   257,   310,     0,     0,    84,   311,     0,
      86,    87,     0,    88,   170,    90,   312,   258,   313,   259,
       0,   314,     0,     0,     0,     0,     0,     0,   315,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   316,     0,
       0,     0,     0,     0,   260,     0,   261,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   262,   263,
     264,   265,   266,   267,   268,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,    48,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,     0,     0,     0,     0,   304,   305,   306,
       0,     0,     0,   307,   308,   309,     0,     0,     0,     0,
       0,   250,   251,     0,   252,   253,     0,     0,   254,   255,
     256,   257,   310,     0,     0,    84,   311,     0,    86,    87,
       0,    88,   170,    90,   312,   258,   313,   259,     0,   314,
       0,     0,     0,     0,     0,     0,   315,  1399,     0,     0,
       0,     0,     0,     0,     0,     0,   316,     0,     0,     0,
       0,     0,   260,     0,   261,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   262,   263,   264,   265,
     266,   267,   268,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,    48,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,     0,     0,     0,     0,   304,   305,   306,     0,     0,
       0,   307,   308,   309,    33,    34,    35,    36,     0,   199,
       0,     0,     0,     0,     0,     0,   607,     0,     0,     0,
     310,     0,     0,    84,   311,     0,    86,    87,    48,    88,
     170,    90,   312,     0,   313,     0,     0,   314,  1490,  1491,
    1492,  1493,  1494,     0,   315,  1495,  1496,  1497,  1498,   201,
       0,     0,     0,     0,   316,     0,     0,     0,     0,     0,
       0,     0,  1499,  1500,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   202,     0,     0,     0,
       0,   169,    82,    83,    84,    85,     0,    86,    87,  1501,
      88,   170,    90,     0,     0,     0,    92,     0,     0,     0,
       0,     0,     0,  1502,  1503,  1504,  1505,  1506,  1507,  1508,
      97,     0,     0,    36,     0,   608,     0,     0,     0,     0,
     609,     0,     0,  1509,  1510,  1511,  1512,  1513,  1514,  1515,
    1516,  1517,  1518,  1519,    48,  1520,  1521,  1522,  1523,  1524,
    1525,  1526,  1527,  1528,  1529,  1530,  1531,  1532,  1533,  1534,
    1535,  1536,  1537,  1538,  1539,  1540,  1541,  1542,  1543,  1544,
    1545,  1546,  1547,  1548,  1549,     0,     0,     0,  1550,  1551,
       0,  1552,  1553,  1554,  1555,  1556,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1557,  1558,  1559,
       0,     0,     0,    86,    87,     0,    88,   170,    90,  1560,
       0,  1561,  1562,     0,  1563,   416,   417,   418,     0,     0,
       0,  1564,  1565,     0,  1566,     0,  1567,  1568,     0,     0,
       0,     0,     0,     0,   419,     0,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,     0,   442,
     416,   417,   418,     0,     0,     0,     0,     0,     0,     0,
       0,   443,     0,     0,     0,     0,     0,     0,     0,   419,
       0,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,     0,   442,   416,   417,   418,     0,     0,
       0,     0,     0,     0,     0,     0,   443,     0,     0,     0,
       0,     0,     0,     0,   419,     0,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,     0,   442,
     416,   417,   418,     0,     0,     0,     0,     0,     0,     0,
       0,   443,     0,     0,     0,     0,     0,     0,     0,   419,
       0,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   528,   442,   416,   417,   418,     0,     0,
       0,     0,     0,     0,     0,     0,   443,     0,     0,     0,
       0,     0,     0,     0,   419,     0,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   530,   442,
     416,   417,   418,     0,     0,     0,     0,     0,     0,     0,
       0,   443,     0,     0,     0,     0,     0,     0,     0,   419,
       0,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   547,   442,   999,  1000,  1001,     0,     0,
       0,     0,     0,     0,     0,     0,   443,     0,     0,     0,
       0,     0,     0,     0,  1002,     0,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,   551,     0,
       0,     0,     0,     0,     0,   250,   251,     0,   252,   253,
       0,  1025,   254,   255,   256,   257,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   258,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   737,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   260,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     262,   263,   264,   265,   266,   267,   268,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,   759,     0,     0,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,    48,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,     0,     0,     0,   303,   304,
     305,   306,  1169,     0,     0,   307,   541,   542,   250,   251,
       0,   252,   253,     0,     0,   254,   255,   256,   257,     0,
       0,     0,     0,     0,   543,     0,     0,     0,     0,     0,
      86,    87,   258,    88,   170,    90,   312,     0,   313,     0,
       0,   314,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   260,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   262,   263,   264,   265,   266,   267,   268,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,    48,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,     0,     0,
       0,  1170,   304,   305,   306,     0,     0,     0,   307,   541,
     542,   250,   251,     0,   252,   253,     0,     0,   254,   255,
     256,   257,     0,     0,     0,     0,     0,   543,     0,     0,
       0,     0,     0,    86,    87,   258,    88,   170,    90,   312,
       0,   313,     0,     0,   314,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   260,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   262,   263,   264,   265,
     266,   267,   268,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,    48,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,     0,     0,     0,     0,   304,   305,   306,  1178,     0,
       0,   307,   541,   542,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   788,   789,     0,     0,     0,
     543,   790,     0,   791,     0,     0,    86,    87,     0,    88,
     170,    90,   312,     0,   313,   792,     0,   314,     0,     0,
       0,     0,     0,    33,    34,    35,    36,   416,   417,   418,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   419,    48,   420,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
       0,   442,     0,     0,     0,     0,   960,     0,     0,     0,
       0,     0,   793,   443,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   202,     0,     0,     0,     0,
     169,    82,    83,    84,   794,     0,    86,    87,    28,    88,
     170,    90,     0,     0,     0,    92,    33,    34,    35,    36,
       0,   199,     0,     0,   795,     0,     0,     0,   200,    97,
       0,     0,     0,     0,   796,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,   490,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   201,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   961,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   202,     0,
       0,     0,     0,   169,    82,    83,    84,    85,     0,    86,
      87,    28,    88,   170,    90,     0,     0,     0,    92,    33,
      34,    35,    36,     0,   199,     0,     0,     0,     0,     0,
       0,   200,    97,     0,     0,     0,     0,   203,     0,     0,
       0,     0,   103,    48,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,   201,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1025,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   202,     0,     0,     0,     0,   169,    82,    83,    84,
      85,     0,    86,    87,    28,    88,   170,    90,     0,     0,
       0,    92,    33,    34,    35,    36,     0,   199,     0,     0,
       0,     0,     0,     0,   200,    97,     0,     0,     0,     0,
     203,     0,     0,   564,     0,   103,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   201,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     584,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   202,     0,     0,     0,     0,   169,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   170,
      90,    28,     0,   916,    92,     0,     0,     0,     0,    33,
      34,    35,    36,     0,   199,     0,     0,     0,    97,     0,
       0,   200,     0,   203,     0,     0,     0,     0,   103,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   201,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   202,     0,     0,     0,     0,   169,    82,    83,    84,
      85,     0,    86,    87,    28,    88,   170,    90,     0,     0,
       0,    92,    33,    34,    35,    36,     0,   199,     0,     0,
       0,     0,     0,     0,   200,    97,     0,     0,     0,     0,
     203,     0,     0,     0,     0,   103,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   201,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1050,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   202,     0,     0,     0,     0,   169,
      82,    83,    84,    85,     0,    86,    87,    28,    88,   170,
      90,     0,     0,     0,    92,    33,    34,    35,    36,     0,
     199,     0,     0,     0,     0,     0,     0,   200,    97,     0,
       0,     0,     0,   203,     0,     0,     0,     0,   103,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     201,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   202,     0,     0,
       0,     0,   169,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   170,    90,     0,     0,     0,    92,     0,     0,
       0,   416,   417,   418,     0,     0,     0,     0,     0,     0,
       0,    97,     0,     0,     0,     0,   203,     0,     0,     0,
     419,   103,   420,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,     0,   442,   416,   417,   418,     0,
       0,     0,     0,     0,     0,     0,     0,   443,     0,     0,
       0,     0,     0,     0,     0,   419,     0,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,     0,
     442,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   443,   416,   417,   418,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   419,   499,   420,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,     0,   442,   416,   417,
     418,     0,     0,     0,     0,     0,     0,     0,     0,   443,
       0,     0,     0,     0,     0,     0,     0,   419,   902,   420,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,     0,   442,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   443,   999,  1000,  1001,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1002,   946,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,     0,     0,
     999,  1000,  1001,     0,     0,     0,     0,     0,     0,     0,
       0,  1025,     0,     0,     0,     0,     0,     0,     0,  1002,
    1250,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1025,   999,  1000,  1001,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1002,  1160,  1003,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
       0,     0,   999,  1000,  1001,     0,     0,     0,     0,     0,
       0,     0,     0,  1025,     0,     0,     0,     0,     0,     0,
       0,  1002,  1307,  1003,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1025,   416,
     417,   418,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   419,  1388,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,     0,   442,     0,     0,   999,  1000,  1001,     0,
       0,     0,     0,     0,     0,   443,     0,     0,     0,     0,
       0,     0,     0,     0,  1473,  1002,  1312,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,   999,
    1000,  1001,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1025,     0,     0,     0,     0,     0,  1002,     0,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1000,  1001,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1025,     0,     0,     0,     0,
    1002,     0,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,   418,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1025,     0,     0,
       0,   419,     0,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,  1001,   442,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   443,     0,
       0,     0,  1002,     0,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   419,  1025,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,     0,   442,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1002,   443,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1025,   420,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,     0,   442,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   443,  1003,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1025
};

static const yytype_int16 yycheck[] =
{
       5,     6,   122,     8,     9,    10,    11,    12,   147,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    54,     4,    28,    29,   702,    91,   173,     4,   650,
      95,    96,    32,     4,     4,   484,  1072,    42,   620,   100,
     372,   152,   372,    55,    44,    50,   621,    52,   875,    49,
      55,   864,    57,    30,   174,   215,   121,    30,   147,   739,
     478,   479,   100,   442,  1059,   600,   100,   895,    30,   770,
       9,   219,   100,   777,    79,   959,     9,   474,    58,    30,
     474,    14,     9,   911,    42,     9,    30,     9,     4,     9,
     508,     9,     9,  1068,    14,   100,    46,     4,     4,    54,
       9,    81,   229,    67,    84,    14,     9,    30,     9,     9,
       9,     9,    80,   510,     9,     4,   510,     9,     9,   230,
      80,    79,     9,     9,     9,     9,     9,     9,     9,    86,
      46,     9,     9,   171,     9,     9,     9,   171,     9,    98,
    1603,     9,     9,   171,     9,    86,    80,   446,   111,    80,
      80,   130,   131,    46,   172,   514,    33,   151,    46,    67,
     155,    72,    73,   172,     4,   203,   171,     0,   186,   203,
      67,   186,    51,   178,   172,   203,   475,   186,   172,   151,
     218,   480,   995,    46,   218,    46,    26,    27,   186,   548,
     186,   148,   108,   188,   189,  1658,   155,   113,   203,   115,
     116,   117,   118,   119,   120,   121,   169,   148,    67,    35,
      67,    35,    67,   218,   186,   183,    67,   151,   149,   150,
      67,   189,   130,   131,    67,   189,   231,    35,    35,   234,
     357,   191,    67,   122,   184,   165,   241,   242,   189,   172,
     147,   186,   158,   159,   188,   161,   173,     8,    67,   188,
     189,   173,  1247,   235,    80,   188,    80,   239,  1142,  1254,
     100,  1256,   408,   622,   188,   969,   182,   971,   188,   187,
    1245,   188,    80,    80,   190,  1103,    54,   156,    67,   188,
      80,   189,    67,    67,   187,   174,    80,   188,   188,   188,
     188,   324,   189,   188,   187,   173,   188,   188,  1320,   187,
     151,   188,   188,   188,   188,   188,   188,   188,   173,  1122,
     187,   349,   187,   187,   187,   349,   187,   892,   853,   187,
     187,   349,    67,   184,   187,    67,   187,   186,    80,   186,
     189,   171,   189,   493,   189,    98,   456,   402,   189,   165,
     486,   165,   189,   348,   349,    67,   189,    67,    54,    98,
     355,   331,   332,   333,   189,   360,   151,   165,   165,   186,
      66,    80,   151,   203,    67,   165,   399,   186,   187,   324,
    1365,   172,   212,    67,   379,    47,    48,   172,   218,  1354,
    1057,  1356,   387,  1405,   364,   186,   451,   452,   453,   454,
     348,   840,   155,   398,   189,   235,   457,   191,   186,   239,
     189,   186,   186,   762,   189,   189,   155,  1429,   767,  1431,
      80,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,  1138,   443,   191,
     445,   446,   447,  1123,   399,   457,   165,   189,   102,   103,
     161,   611,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   442,   186,   189,    51,   442,
     475,   476,   378,   478,   479,   480,   481,     4,  1453,   386,
     442,   259,   487,   261,   881,   490,   189,   881,    35,  1071,
     908,  1304,   186,   448,   499,   165,   501,   386,   338,   638,
     627,   186,   629,   508,   516,   102,   103,   347,   157,   349,
      98,   516,  1087,   518,   354,  1090,    35,   130,   131,   442,
     151,   361,   118,    98,    51,   448,    67,    54,    98,    99,
     126,   691,   864,    80,   864,    80,   190,   916,   316,   521,
     189,    86,   497,    29,    71,    67,   386,   695,   189,   638,
      80,   474,    26,    27,    98,   186,    86,    80,   704,   564,
    1373,    80,    89,    86,    91,    80,   455,   155,    95,    96,
     608,    86,   552,   156,   497,   186,   556,   190,  1255,   727,
     155,   561,   186,  1263,   507,  1265,     4,   510,   171,   130,
     131,   656,    78,   190,   121,   115,   116,   117,   118,   119,
     120,    87,   186,   608,   149,   150,   186,   966,   130,   131,
     151,   155,   186,    99,   130,   131,   743,   744,   396,   149,
     150,   399,   749,   750,   784,   148,   149,   150,    46,   119,
      29,  1606,   186,   148,   149,   150,   126,    14,   749,    78,
      80,    72,    73,   648,   484,   186,    86,   156,    47,   797,
     195,    50,  1234,    30,   157,   660,   189,   805,  1335,   186,
      99,   372,   182,   995,   194,   995,   152,   170,   188,   155,
     186,    48,   158,   159,  1249,   161,   162,   163,  1096,   186,
     172,   521,    80,    29,   155,   124,   189,    30,   693,  1107,
     108,  1371,  1141,    29,   186,   113,    67,   115,   116,   117,
     118,   119,   120,   121,   128,   129,  1742,   151,   235,   149,
     150,    47,   239,   186,    50,   621,   243,   722,    35,   158,
     159,  1757,   161,   162,   163,   101,   102,   103,   172,  1406,
     188,   638,    78,   101,   102,   103,   188,   448,   212,    78,
     158,   159,   186,   161,    67,   189,   188,   186,   189,   754,
     148,   149,   150,    99,    50,    51,    52,   535,    54,   188,
      99,  1736,   188,   474,   182,   770,   766,  1216,    80,   188,
      66,   151,   190,    80,    86,   121,  1751,   188,   189,    86,
    1460,   941,    50,    51,    52,   186,   497,    67,   134,   135,
    1122,  1366,  1122,   775,   188,   189,   507,   324,    66,   510,
     115,   116,   117,   118,   119,   120,   152,   647,    78,   155,
     156,   186,   158,   159,   151,   161,   162,   163,   188,   158,
     159,   186,   161,   162,   163,   985,   115,   116,   117,    99,
     155,   811,   992,  1251,    45,   815,    66,   149,   150,   617,
     618,   172,   149,   150,   849,   151,  1205,   186,   626,   193,
     915,  1632,  1633,   760,   104,   105,   106,   186,   863,   386,
    1628,  1629,   186,     9,   338,   705,   151,   182,   395,   151,
     852,   760,   399,   347,     8,   402,   852,   186,   188,   151,
     354,   852,   852,  1710,   889,   155,   186,   361,   158,   159,
      14,   161,   162,   163,   899,    78,   151,   902,   372,   904,
     807,   188,  1729,   908,   126,   745,   126,   747,    14,   187,
    1737,   172,   867,    14,   786,   787,    99,  1276,   807,   189,
     760,   448,   449,   450,   451,   452,   453,   454,  1608,    98,
    1078,   187,   772,  1292,  1352,   775,   187,   186,    78,   916,
       4,   946,   187,   916,   867,   852,   852,   474,   187,   192,
     107,   186,   952,   186,   916,     9,   879,  1117,   881,    99,
     148,   187,   187,   852,   187,    90,   873,   807,   187,   875,
     497,   953,  1304,   156,  1304,   158,   159,   160,   161,   162,
     163,     9,    46,   510,   873,   825,   892,   188,    14,   186,
     172,  1052,     9,   916,   521,   186,    80,   187,   187,   128,
     840,   841,   187,   186,   188,   186,    67,  1167,   786,   787,
     484,   187,   852,   540,  1174,    30,  1375,   129,   158,   159,
     171,   161,   162,   163,   947,  1384,     9,   132,   151,   187,
     151,  1652,    14,   873,   184,  1394,   563,    26,    27,   994,
    1052,  1373,   997,  1373,   108,     9,   186,  1052,   955,   113,
     957,   115,   116,   117,   118,   119,   120,   121,     9,   173,
     187,     9,   589,   590,    14,   128,   955,   193,   957,     9,
     190,   193,    14,   186,  1056,   193,   854,    78,  1083,    80,
    1056,   151,   193,   187,   187,  1056,  1056,   187,    98,   188,
     188,  1096,   870,    87,   158,   159,   132,   161,    99,   151,
       9,   187,  1107,  1108,   186,   883,  1727,  1466,   151,   186,
     151,   189,  1272,   953,   189,   955,    14,   957,   182,   959,
     960,    78,   188,    80,   189,    14,   190,    14,   107,   656,
    1130,   193,   189,  1138,   912,   188,   115,   116,   117,   118,
     119,   120,    99,  1148,   184,   187,    14,    30,   186,  1056,
    1056,  1133,   186,   864,    30,   156,   867,   158,   159,   186,
     161,   162,   163,   186,    14,   186,   186,  1056,   879,    49,
     881,     9,   188,   647,   187,   186,    75,    76,    77,    78,
     188,  1087,    26,    27,  1090,   132,    14,   132,   189,     9,
     191,   187,    66,   193,    78,   973,     9,  1336,   976,   156,
      99,   158,   159,   182,   161,   162,   163,    80,     9,   186,
     132,  1331,   739,   186,   741,    99,  1056,   188,    14,    80,
     187,  1226,   188,   212,   189,  1230,   186,  1232,   186,   132,
     189,   705,   189,   760,   191,  1240,   947,   189,  1145,  1598,
     193,  1600,  1082,   187,     9,  1250,  1251,   774,   775,    74,
    1609,    87,   148,    30,   188,   187,  1145,   188,   173,   158,
     159,   132,   161,   162,   163,   132,    30,   187,   187,   187,
       9,   745,   156,   747,   158,   159,   190,   161,   162,   163,
     807,  1427,     9,   187,   995,   190,   813,  1127,   189,    14,
     817,   818,  1070,  1133,  1072,    80,  1655,    78,   772,   187,
     186,  1141,  1142,   188,   186,  1145,   187,  1214,  1214,   187,
     837,   132,   189,     9,    30,   187,  1223,  1223,    99,   188,
     108,  1099,   187,   187,  1102,   852,    47,    48,    49,    50,
      51,    52,   187,    54,  1399,   187,   189,   188,    78,   188,
     867,   160,   188,  1249,  1349,    66,   873,  1352,   156,   338,
      14,   825,   879,   113,   881,   187,    80,   187,   347,    99,
     349,   132,   189,   187,   132,   354,   840,   841,   212,    14,
     172,   152,   361,   189,   155,  1153,  1216,   158,   159,  1157,
     161,   162,   163,   188,    14,    80,    14,    80,   915,   187,
     864,   186,    78,   187,    80,    81,  1755,   189,   132,  1381,
     927,   928,   929,  1762,   188,   188,    14,  1314,    14,   190,
     188,  1122,   152,    99,    14,   155,   156,    78,   158,   159,
     947,   161,   162,   163,     9,  1314,   953,   189,   955,  1336,
     957,   190,  1210,  1211,    56,   172,    80,   186,    99,  1444,
      80,  1330,  1588,     9,   188,   111,    98,  1402,  1355,  1355,
     977,   151,    98,   163,  1361,  1361,  1363,  1363,   358,    33,
    1366,    14,   362,  1623,   186,   188,   186,   994,   187,  1376,
     997,   169,   158,   159,  1314,   161,   162,   163,  1385,   173,
     187,    80,  1464,  1465,   166,   959,   960,  1376,   388,     9,
     390,   391,   392,   393,   338,   484,  1385,   158,   159,  1026,
     161,   162,   163,   347,    80,  1032,   189,   188,  1035,   187,
     354,   115,   116,   117,   118,   119,   120,   361,  1296,   187,
    1298,   995,   126,   127,    14,    80,    14,    14,    80,  1056,
      47,    48,    49,    50,    51,    52,  1376,    80,    14,    80,
     811,  1381,   561,  1718,   454,  1385,  1435,   449,   815,    66,
    1583,  1458,  1458,  1331,   451,   914,  1463,   909,   162,  1733,
     164,   855,  1469,  1139,  1289,  1704,  1729,  1474,   566,  1465,
    1407,  1572,  1326,   177,  1463,   179,  1456,     4,   182,  1761,
    1469,  1646,  1749,  1488,  1322,  1474,    26,    27,  1593,   993,
      30,  1584,  1119,  1304,  1452,    78,  1123,  1060,   928,  1105,
    1118,   943,  1722,   879,   355,  1119,  1133,   786,  1082,   399,
    1315,  1686,   978,  1043,    54,    -1,    99,  1026,  1145,    46,
      78,    -1,    -1,  1463,  1464,  1465,    -1,    -1,  1583,  1469,
      -1,    -1,    -1,    -1,  1474,    -1,    -1,    -1,   121,    -1,
     484,    99,    78,    -1,    -1,    -1,    -1,    -1,  1122,   107,
     108,   134,   135,  1127,    -1,    -1,    -1,    -1,   647,    -1,
      -1,    -1,  1373,    99,    -1,    -1,    -1,  1141,  1142,   152,
      -1,    -1,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,   108,  1209,    -1,    -1,    -1,   113,    -1,   115,   116,
     117,   118,   119,   120,   121,    -1,    -1,   155,    -1,    -1,
     158,   159,    -1,   161,   162,   163,  1613,    -1,  1486,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   705,    -1,    -1,   155,
      -1,    -1,   158,   159,  1613,   161,   162,   163,    -1,    -1,
      -1,   158,   159,    -1,   161,    -1,  1263,    -1,  1265,    -1,
      -1,    -1,  1216,    -1,    -1,    -1,  1653,  1654,    -1,    -1,
      -1,    -1,    -1,  1660,    -1,   182,   745,    -1,   747,    -1,
      -1,    -1,  1767,   190,  1653,  1654,    -1,    -1,    -1,    -1,
    1775,  1660,   212,  1613,    -1,    -1,  1781,    -1,    -1,  1784,
      -1,    -1,    -1,   772,    -1,    -1,    -1,  1314,    -1,    -1,
    1697,    -1,    -1,  1320,    -1,    -1,    -1,  1704,    -1,  1326,
      -1,    -1,    -1,   647,  1710,    75,    76,    77,  1697,  1587,
      -1,    -1,    -1,  1653,  1654,    -1,    -1,    87,    -1,   259,
    1660,   261,    -1,  1729,    -1,    -1,    -1,    -1,    -1,    -1,
    1304,  1737,    -1,    -1,    -1,    -1,   825,     4,    -1,    -1,
      -1,    -1,    -1,    -1,  1371,    -1,    -1,    -1,    -1,  1376,
      -1,   840,   841,    -1,  1381,    -1,  1763,  1697,  1385,    -1,
      -1,   705,    -1,  1770,    -1,    -1,   136,   137,   138,   139,
     140,     4,  1399,    -1,  1763,  1402,   316,   147,  1405,    46,
      -1,  1770,    -1,   153,   154,    -1,    -1,    -1,  1415,    -1,
      -1,  1669,    -1,    -1,    -1,  1422,    -1,   167,   338,  1373,
      -1,   745,  1429,   747,  1431,    -1,    -1,   347,    -1,    -1,
    1437,   181,    -1,    46,   354,    -1,    -1,    -1,    -1,    -1,
      -1,   361,    -1,  1763,    78,    -1,    -1,    -1,   772,    -1,
    1770,    -1,   372,  1460,    -1,    -1,  1463,  1464,  1465,    -1,
      -1,   108,  1469,    -1,    -1,    99,   113,  1474,   115,   116,
     117,   118,   119,   120,   121,    -1,   396,    -1,    -1,   399,
      -1,    -1,    -1,    -1,  1742,    -1,    -1,    -1,    -1,    -1,
     959,   960,    -1,    -1,    -1,   108,    -1,    -1,    -1,  1757,
     113,   825,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,   158,   159,    -1,   161,    -1,   840,   841,    -1,    -1,
      -1,   155,   442,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   190,    -1,   158,   159,    -1,   161,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,   484,    -1,    -1,    -1,    -1,   182,
      -1,    78,    -1,    -1,    -1,    -1,  1583,   190,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    99,    -1,    -1,    -1,    -1,  1604,    -1,    64,
      65,  1608,    -1,    -1,    -1,    -1,  1613,    -1,    -1,    -1,
      -1,    -1,    -1,  1082,  1621,   535,   536,    -1,    -1,   539,
      -1,  1628,  1629,    -1,    -1,  1632,  1633,    -1,    64,    65,
      -1,    -1,    -1,    -1,    -1,   959,   960,    -1,    -1,  1646,
      -1,    -1,    -1,    -1,    -1,    -1,  1653,  1654,   155,    -1,
     570,   158,   159,  1660,   161,   162,   163,    -1,  1127,    -1,
      26,    27,    -1,    -1,    30,   130,   131,    -1,    -1,    -1,
      -1,    -1,  1141,  1142,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
    1697,    -1,    -1,    -1,   130,   131,  1703,   617,   618,   115,
     116,   117,   118,   119,   120,    -1,   626,    -1,    29,    -1,
     126,   127,    -1,    -1,  1721,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    64,    65,    -1,   647,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1216,   164,    -1,
      -1,   187,    29,    -1,    -1,    -1,  1763,    78,  1082,    -1,
      -1,    -1,    -1,  1770,    -1,    -1,   182,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    56,
      -1,    -1,    -1,    -1,    -1,   705,   107,    -1,    -1,    -1,
     130,   131,    -1,    -1,   115,   116,   117,   118,   119,   120,
      -1,    78,    -1,  1127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,   135,    -1,    -1,  1141,  1142,    -1,
      -1,    -1,    99,    -1,    -1,   745,    -1,   747,    -1,    -1,
     107,   152,    -1,    -1,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,   212,   187,    -1,    -1,
      -1,    -1,   772,   773,    -1,   176,    -1,   134,   135,    -1,
      -1,   182,    -1,    -1,    -1,   186,   786,   787,   788,   789,
     790,   791,   792,    -1,    -1,   152,   796,    -1,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,   808,    -1,
      -1,    -1,  1216,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,   825,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   838,    -1,
     840,   841,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   854,   855,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   864,    -1,    -1,    -1,    -1,    -1,
     870,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   883,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   891,   338,    -1,   894,    -1,    -1,    -1,    -1,    -1,
      -1,   347,    -1,    -1,    -1,    -1,    -1,    -1,   354,    10,
      11,    12,   912,    -1,    -1,   361,   916,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   372,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,   959,
     960,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   973,    -1,    -1,   976,    -1,   978,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   993,    -1,   995,   442,    -1,   998,   999,
    1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   484,    -1,
    1040,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
    1070,    54,  1072,    -1,    -1,    -1,    47,    48,    -1,    -1,
      -1,    -1,  1082,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   193,   539,    -1,    -1,    67,    -1,    -1,  1099,
      -1,    54,  1102,    -1,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,  1122,    -1,   570,    -1,    -1,  1127,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1141,  1142,    -1,  1144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1153,    -1,    -1,    -1,  1157,    -1,    -1,
    1160,    -1,  1162,   134,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,  1178,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,   647,    -1,    -1,    -1,   176,    -1,   190,    -1,    -1,
    1210,  1211,    -1,  1213,    -1,    29,  1216,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,   705,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1296,    66,  1298,   745,
      -1,   747,    -1,  1303,  1304,    -1,   259,  1307,   261,  1309,
      -1,    -1,  1312,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1320,  1321,    -1,    -1,  1324,    -1,   772,   773,    -1,    -1,
      -1,  1331,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   788,   789,   790,   791,   792,    -1,    -1,    -1,
     796,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    56,   808,   316,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1373,    -1,    -1,   190,    -1,    -1,   825,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,  1388,    29,
      -1,    -1,   838,    -1,   840,   841,    -1,  1397,  1398,    -1,
      -1,    -1,    -1,    -1,    99,  1405,    -1,  1407,    -1,   855,
      -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,   864,    -1,
      -1,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1429,
      -1,  1431,    -1,    -1,    -1,    -1,    -1,  1437,    78,   134,
     135,    -1,    -1,   396,    -1,   891,   399,    -1,   894,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,    99,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
     916,    -1,  1472,  1473,    -1,    -1,    -1,    -1,  1478,    -1,
    1480,   176,    -1,    -1,    -1,    -1,  1486,    -1,  1488,    -1,
      -1,   186,    -1,    -1,   134,   135,    -1,    -1,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,   152,   959,   960,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    66,   165,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   186,   993,    -1,   995,
      -1,    -1,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
      -1,    -1,   535,   536,    -1,    -1,   539,  1587,    -1,    -1,
      -1,    -1,    -1,    -1,  1040,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1604,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   570,    -1,    -1,
      -1,  1621,    -1,    -1,    -1,    26,    27,  1627,    -1,    30,
     539,    -1,    -1,    -1,    -1,    -1,  1082,    -1,  1638,    -1,
      -1,    -1,    -1,    -1,  1644,    -1,    -1,    -1,  1648,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   570,    -1,    -1,   617,   618,    -1,    -1,    -1,  1669,
      -1,    -1,    -1,   626,    -1,    -1,  1122,    -1,    -1,    -1,
      -1,  1127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1141,  1142,    -1,  1144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1708,    -1,
      -1,    -1,    -1,    -1,  1160,    -1,  1162,  1717,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1178,  1733,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1742,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,  1757,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1213,    -1,    -1,
    1216,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,   212,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     773,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   786,   787,   788,   789,   790,   791,   792,
      -1,    -1,    -1,   796,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1303,  1304,    -1,
      -1,  1307,    -1,  1309,   773,    -1,  1312,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1321,    -1,    -1,  1324,   788,
     789,   790,   791,   792,    -1,    -1,    -1,   796,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,   854,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,   870,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1373,    -1,    -1,
     883,    -1,   190,    -1,    -1,    -1,    -1,   338,   891,    -1,
      -1,    -1,  1388,    -1,    -1,    -1,   347,    -1,    -1,    -1,
      -1,  1397,  1398,   354,    -1,    -1,    -1,    -1,    -1,   912,
     361,  1407,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,   372,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   891,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
     973,    -1,    -1,   976,    -1,   978,  1472,  1473,    -1,    -1,
      -1,    -1,  1478,    -1,  1480,    -1,    -1,    -1,    -1,    -1,
     993,   442,  1488,    -1,    -1,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   484,   993,    -1,    -1,  1040,    -1,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1070,    -1,  1072,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1040,    -1,    -1,    -1,    -1,    -1,    -1,   539,    -1,
      -1,    -1,   190,    -1,    -1,    -1,  1099,    -1,    -1,  1102,
      75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,   570,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,  1627,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1144,  1638,    -1,    -1,   120,    -1,    -1,  1644,    -1,
    1153,    -1,  1648,    -1,  1157,    -1,    -1,  1160,    -1,  1162,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,  1178,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,  1144,   161,   162,   163,    -1,
      -1,    -1,   167,    -1,    -1,    -1,   647,    -1,    -1,    -1,
      -1,  1160,    -1,  1162,    -1,    -1,   181,  1210,  1211,    -1,
      -1,   186,  1708,    -1,    -1,    -1,   191,    -1,    -1,  1178,
      -1,  1717,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,  1733,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   705,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,  1296,   745,  1298,   747,    -1,    -1,    -1,
    1303,    -1,    -1,    -1,  1307,    -1,  1309,    -1,    -1,  1312,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1320,    -1,    -1,
      -1,   772,   773,    -1,    -1,    -1,    -1,    -1,  1331,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   788,   789,   790,
     791,   792,   539,    -1,  1303,   796,    -1,    -1,  1307,    -1,
    1309,    -1,    -1,  1312,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    29,    -1,
      -1,    -1,    -1,   570,   825,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1388,    -1,    -1,    -1,   840,
     841,    -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,  1405,    -1,    -1,    64,    65,    -1,    -1,    -1,
      -1,    -1,    -1,   864,   190,    -1,    -1,    78,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,  1429,    -1,  1431,  1388,
      -1,    -1,    -1,    -1,  1437,    -1,    -1,    -1,    99,    29,
     891,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,   916,    -1,    -1,    -1,  1472,
    1473,   130,   131,   134,   135,  1478,    66,    -1,    -1,    -1,
      -1,    -1,    -1,  1486,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,   165,    -1,    -1,    -1,   959,   960,
      -1,    -1,    -1,  1472,  1473,   176,    -1,    -1,    -1,  1478,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,   993,    -1,   995,    -1,    -1,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,    -1,   773,    64,    65,    -1,
      -1,    -1,    -1,    -1,  1587,    -1,    -1,    -1,    -1,  1040,
      -1,   788,   789,   790,   791,    -1,    -1,    -1,    -1,   796,
     190,  1604,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      -1,    -1,    -1,    -1,    53,    -1,    55,    -1,  1621,    -1,
      -1,    -1,    -1,    -1,  1627,    -1,    -1,    -1,    67,    -1,
      -1,  1082,    -1,    -1,    -1,  1638,    75,    76,    77,    78,
      -1,  1644,    -1,   130,   131,  1648,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,    -1,    -1,  1669,    -1,  1627,    -1,
      -1,  1122,    -1,    -1,    -1,    -1,  1127,    -1,    -1,  1638,
      -1,    -1,    -1,    -1,    -1,  1644,    -1,    -1,    -1,  1648,
    1141,  1142,    -1,  1144,   891,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,  1708,    -1,    -1,   147,  1160,
      -1,  1162,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,  1178,   167,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,  1742,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   186,    -1,  1708,
      -1,    -1,    -1,    -1,  1757,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1216,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1040,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,  1303,  1304,    -1,    -1,  1307,    -1,  1309,    -1,
      -1,  1312,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,  1373,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,  1388,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,  1144,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1160,    -1,  1162,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1178,    -1,    -1,    -1,    47,    48,    -1,    -1,    -1,
      -1,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    70,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    -1,
      -1,  1472,  1473,    -1,   190,    87,    -1,  1478,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1487,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,   116,   117,   118,   119,   120,    -1,
      -1,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,   134,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,   167,  1303,    -1,   170,    -1,
    1307,    -1,  1309,    -1,   176,  1312,    10,    11,    12,   181,
     182,   183,    -1,    -1,   186,    -1,   188,    -1,    -1,   191,
     192,    -1,   194,   195,    28,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,  1627,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1638,    -1,    -1,
      -1,  1388,    -1,  1644,    -1,    -1,    -1,  1648,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1671,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1708,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1472,  1473,    46,    47,    48,
      -1,  1478,    -1,    -1,    53,    -1,    55,    56,    57,    58,
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
     179,    -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,
     189,   190,   191,   192,    -1,   194,   195,    -1,    -1,    -1,
    1627,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,  1638,    -1,    13,    -1,    -1,    -1,  1644,    -1,    -1,
      -1,  1648,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,
      -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,  1708,    82,    83,    -1,    -1,    -1,    87,    88,    89,
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
     107,   108,   109,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,   122,   123,   124,   125,   126,
     127,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,   164,    -1,    -1,
     167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,
     177,    -1,   179,    -1,   181,   182,   183,    -1,    -1,   186,
      -1,   188,   189,    -1,   191,   192,    -1,   194,   195,     3,
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
      82,    83,    -1,    -1,    -1,    87,    88,    89,    90,    91,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,
      -1,    -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    71,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    96,    97,    -1,
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
      90,    -1,    92,    -1,    94,    95,    96,    -1,    -1,    99,
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
      -1,    92,    93,    94,    -1,    96,    -1,    -1,    99,   100,
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
      -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,
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
     181,   182,   183,    -1,    -1,   186,    -1,   188,   189,    -1,
     191,   192,    -1,   194,   195,     3,     4,     5,     6,     7,
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
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,   192,    -1,   194,   195,     3,     4,
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
      -1,    -1,   190,   176,    -1,    -1,    -1,    -1,   181,   182,
     183,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
      -1,   194,   195,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    35,    -1,    -1,    -1,    -1,
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
     167,    -1,    -1,   170,    -1,    -1,    -1,    -1,   190,   176,
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
      -1,    -1,    -1,    -1,    -1,    13,    29,    30,    31,    32,
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
      -1,    -1,   170,    -1,    -1,   188,    -1,    -1,   176,    -1,
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
      -1,    13,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
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
     182,   183,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
     192,    -1,   194,   195,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,    -1,   194,   195,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    35,
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
     176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,    -1,   194,   195,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    -1,
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
     170,    -1,    -1,   188,    -1,    -1,   176,    -1,    -1,    -1,
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
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   166,    -1,    -1,   169,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    54,    -1,   189,    -1,   191,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    68,    69,    70,    71,
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
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,   153,   154,
      -1,    -1,    -1,   158,   159,    -1,   161,   162,   163,   164,
      -1,   166,   167,    -1,   169,    10,    11,    12,    -1,    -1,
      -1,   176,   177,    -1,   179,    -1,   181,   182,    -1,    -1,
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
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,   188,    54,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,   188,    54,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,   188,    54,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,   188,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    66,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    69,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,   126,   127,
     128,   129,   187,    -1,    -1,   133,   134,   135,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,    -1,    -1,
     158,   159,    27,   161,   162,   163,   164,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    69,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,    -1,   127,   128,   129,    30,    -1,
      -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    -1,    -1,
     152,    53,    -1,    55,    -1,    -1,   158,   159,    -1,   161,
     162,   163,   164,    -1,   166,    67,    -1,   169,    -1,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    99,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,   134,    66,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    67,   161,
     162,   163,    -1,    -1,    -1,   167,    75,    76,    77,    78,
      -1,    80,    -1,    -1,   176,    -1,    -1,    -1,    87,   181,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    67,   161,   162,   163,    -1,    -1,    -1,   167,    75,
      76,    77,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    87,   181,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,    99,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,   120,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    67,   161,   162,   163,    -1,    -1,
      -1,   167,    75,    76,    77,    78,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,   181,    -1,    -1,    -1,    -1,
     186,    -1,    -1,   189,    -1,   191,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    67,    -1,    69,   167,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    -1,    80,    -1,    -1,    -1,   181,    -1,
      -1,    87,    -1,   186,    -1,    -1,    -1,    -1,   191,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    67,   161,   162,   163,    -1,    -1,
      -1,   167,    75,    76,    77,    78,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,   181,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    67,   161,   162,
     163,    -1,    -1,    -1,   167,    75,    76,    77,    78,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    87,   181,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      29,   191,    31,    32,    33,    34,    35,    36,    37,    38,
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
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   132,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    12,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    66,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    66,    31,    32,    33,    34,
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
      -1,    -1,    -1,    66
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
     421,   423,   436,   438,   440,   118,   119,   120,   133,   152,
     162,   186,   203,   236,   315,   334,   412,   334,   186,   334,
     334,   334,   104,   334,   334,   398,   399,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,    80,
      87,   120,   147,   186,   213,   353,   370,   373,   378,   412,
     415,   412,    35,   334,   427,   428,   334,   120,   186,   213,
     370,   371,   372,   374,   378,   409,   410,   411,   419,   424,
     425,   186,   325,   375,   186,   325,   344,   326,   334,   222,
     325,   186,   186,   186,   325,   188,   334,   203,   188,   334,
       3,     4,     6,     7,    10,    11,    12,    13,    27,    29,
      54,    56,    68,    69,    70,    71,    72,    73,    74,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   126,   127,   128,   129,   133,   134,   135,
     152,   156,   164,   166,   169,   176,   186,   203,   204,   205,
     216,   441,   456,   457,   459,   188,   331,   334,   189,   229,
     334,   107,   108,   155,   206,   209,   212,    80,   191,   281,
     282,   119,   126,   118,   126,    80,   283,   186,   186,   186,
     186,   203,   253,   444,   186,   186,   326,    80,    86,   148,
     149,   150,   433,   434,   155,   189,   212,   212,   203,   254,
     444,   156,   186,   444,   444,    80,   183,   189,   345,    27,
     324,   328,   334,   335,   412,   416,   218,   189,    86,   376,
     433,    86,   433,   433,    30,   155,   172,   445,   186,     9,
     188,    35,   235,   156,   252,   444,   120,   182,   236,   316,
     188,   188,   188,   188,   188,   188,    10,    11,    12,    29,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    54,    66,   188,    67,    67,   189,   151,   127,
     162,   164,   177,   179,   255,   314,   315,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      64,    65,   130,   131,   402,    67,   189,   407,   186,   186,
      67,   189,   191,   420,   186,   235,   236,    14,   334,   188,
     132,    45,   203,   397,    86,   324,   335,   151,   412,   132,
     193,     9,   383,   324,   335,   412,   445,   151,   186,   377,
     402,   407,   187,   334,    30,   220,     8,   346,     9,   188,
     220,   221,   326,   327,   334,   203,   267,   224,   188,   188,
     188,   134,   135,   459,   459,   172,   186,   107,   459,    14,
     151,   134,   135,   152,   203,   205,   188,   188,   230,   111,
     169,   188,   155,   207,   210,   212,   155,   208,   211,   212,
     212,     9,   188,    98,   189,   412,     9,   188,   126,   126,
      14,     9,   188,   412,   437,   326,   324,   335,   412,   415,
     416,   187,   172,   247,   133,   412,   426,   427,   188,    67,
     402,   148,   434,    79,   334,   412,    86,   148,   434,   212,
     202,   188,   189,   242,   250,   360,   362,    87,   186,   191,
     347,   348,   350,   373,   418,   420,   438,    14,    98,   439,
     341,   342,   343,   277,   278,   400,   401,   187,   187,   187,
     187,   187,   190,   219,   220,   237,   244,   249,   400,   334,
     192,   194,   195,   203,   446,   447,   459,    35,   165,   279,
     280,   334,   441,   186,   444,   245,   235,   334,   334,   334,
      30,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   374,   334,   334,   422,   422,   334,
     429,   430,   126,   189,   204,   205,   419,   420,   253,   203,
     254,   444,   444,   252,   236,    35,   328,   331,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   156,   189,   203,   403,   404,   405,   406,   419,   422,
     334,   279,   279,   422,   334,   426,   235,   187,   334,   186,
     396,     9,   383,   187,   187,    35,   334,    35,   334,   187,
     187,   187,   419,   279,   189,   203,   403,   404,   419,   187,
     218,   271,   189,   331,   334,   334,    90,    30,   220,   265,
     188,    28,    98,    14,     9,   187,    30,   189,   268,   459,
      29,    87,   216,   453,   454,   455,   186,     9,    47,    48,
      53,    55,    67,   134,   156,   176,   186,   213,   214,   216,
     355,   370,   378,   379,   380,   203,   458,   218,   186,   228,
     212,     9,   188,    98,   212,     9,   188,    98,    98,   209,
     203,   334,   282,   379,    80,     9,   187,   187,   187,   187,
     187,   187,   187,   188,    47,    48,   451,   452,   128,   258,
     186,     9,   187,   187,    80,    81,   203,   435,   203,    67,
     190,   190,   199,   201,    30,   129,   257,   171,    51,   156,
     171,   364,   335,   132,     9,   383,   187,   151,   459,   459,
      14,   346,   277,   218,   184,     9,   384,   459,   460,   402,
     407,   402,   190,     9,   383,   173,   412,   334,   187,     9,
     384,    14,   338,   238,   128,   256,   186,   444,   334,    30,
     193,   193,   132,   190,     9,   383,   334,   445,   186,   248,
     243,   251,    14,   439,   246,   235,    69,   412,   334,   445,
     193,   190,   187,   187,   193,   190,   187,    47,    48,    67,
      75,    76,    77,    87,   134,   147,   176,   203,   386,   388,
     389,   392,   395,   203,   412,   412,   132,   402,   407,   187,
     334,   272,    72,    73,   273,   218,   325,   218,   327,    98,
      35,   133,   262,   412,   379,   203,    30,   220,   266,   188,
     269,   188,   269,     9,   173,    87,   132,   151,     9,   383,
     187,   165,   446,   447,   448,   446,   379,   379,   379,   379,
     379,   382,   385,   186,   151,   186,   379,   151,   189,    10,
      11,    12,    29,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    66,   151,   445,   190,   370,
     189,   232,    98,   210,   203,    98,   211,   203,   203,   190,
      14,   412,   188,     9,   173,   203,   259,   370,   189,   426,
     133,   412,    14,   193,   334,   190,   199,   459,   259,   189,
     363,    14,   187,   334,   347,   419,   188,   459,   184,   190,
      30,   449,   401,    35,    80,   165,   403,   404,   406,   403,
     404,   459,    35,   165,   334,   379,   277,   186,   370,   257,
     339,   239,   334,   334,   334,   190,   186,   279,   258,    30,
     257,   459,    14,   256,   444,   374,   190,   186,    14,    75,
      76,    77,   203,   387,   387,   389,   390,   391,    49,   186,
      86,   148,   186,     9,   383,   187,   396,    35,   334,   190,
      72,    73,   274,   325,   220,   190,   188,    91,   188,   262,
     412,   186,   132,   261,    14,   218,   269,   101,   102,   103,
     269,   190,   459,   132,   459,   203,   453,     9,   187,   383,
     132,   193,     9,   383,   382,   204,   347,   349,   351,   187,
     126,   204,   379,   431,   432,   379,   379,   379,    30,   379,
     379,   379,   379,   379,   379,   379,   379,   379,   379,   379,
     379,   379,   379,   379,   379,   379,   379,   379,   379,   379,
     379,   379,   379,   458,    80,   233,   203,   203,   379,   452,
      98,    99,   450,     9,   287,   187,   186,   328,   331,   334,
     193,   190,   439,   287,   157,   170,   189,   359,   366,   157,
     189,   365,   132,   188,   449,   459,   346,   460,    80,   165,
      14,    80,   445,   412,   334,   187,   277,   189,   277,   186,
     132,   186,   279,   187,   189,   459,   189,   188,   459,   257,
     240,   377,   279,   132,   193,     9,   383,   388,   390,   148,
     347,   393,   394,   389,   412,   325,    30,    74,   220,   188,
     327,   261,   426,   262,   187,   379,    97,   101,   188,   334,
      30,   188,   270,   190,   173,   459,   132,   165,    30,   187,
     379,   379,   187,   132,     9,   383,   187,   132,   190,     9,
     383,   379,    30,   187,   218,   203,   459,   459,   370,     4,
     108,   113,   119,   121,   158,   159,   161,   190,   288,   313,
     314,   315,   320,   321,   322,   323,   400,   426,   190,   189,
     190,    51,   334,   334,   334,   346,    35,    80,   165,    14,
      80,   334,   186,   449,   187,   287,   187,   277,   334,   279,
     187,   287,   439,   287,   188,   189,   186,   187,   389,   389,
     187,   132,   187,     9,   383,    30,   218,   188,   187,   187,
     187,   225,   188,   188,   270,   218,   459,   459,   132,   379,
     347,   379,   379,   379,   189,   190,   450,   128,   129,   177,
     204,   442,   459,   260,   370,   108,   323,    29,   121,   134,
     135,   156,   162,   297,   298,   299,   300,   370,   160,   305,
     306,   124,   186,   203,   307,   308,   289,   236,   459,     9,
     188,     9,   188,   188,   439,   314,   187,   284,   156,   361,
     190,   190,    80,   165,    14,    80,   334,   279,   113,   336,
     449,   190,   449,   187,   187,   190,   189,   190,   287,   277,
     132,   389,   347,   218,   223,   226,    30,   220,   264,   218,
     187,   379,   132,   132,   218,   370,   370,   444,    14,   204,
       9,   188,   189,   442,   439,   300,   172,   189,     9,   188,
       3,     4,     5,     6,     7,    10,    11,    12,    13,    27,
      28,    54,    68,    69,    70,    71,    72,    73,    74,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     133,   134,   136,   137,   138,   139,   140,   152,   153,   154,
     164,   166,   167,   169,   176,   177,   179,   181,   182,   203,
     367,   368,     9,   188,   156,   160,   203,   308,   309,   310,
     188,    80,   319,   235,   290,   442,   442,    14,   236,   190,
     285,   286,   442,    14,    80,   334,   187,   186,   189,   188,
     189,   311,   336,   449,   284,   190,   187,   389,   132,    30,
     220,   263,   264,   218,   379,   379,   190,   188,   188,   379,
     370,   293,   459,   301,   302,   378,   298,    14,    30,    48,
     303,   306,     9,    33,   187,    29,    47,    50,    14,     9,
     188,   205,   443,   319,    14,   459,   235,   188,    14,   334,
      35,    80,   358,   218,   218,   189,   311,   190,   449,   389,
     218,    95,   231,   190,   203,   216,   294,   295,   296,     9,
     173,     9,   383,   190,   379,   368,   368,    56,   304,   309,
     309,    29,    47,    50,   379,    80,   172,   186,   188,   379,
     444,   379,    80,     9,   384,   190,   190,   218,   311,    93,
     188,   111,   227,   151,    98,   459,   378,   163,    14,   451,
     291,   186,    35,    80,   187,   190,   188,   186,   169,   234,
     203,   314,   315,   173,   379,   173,   275,   276,   401,   292,
      80,   370,   232,   166,   203,   188,   187,     9,   384,   115,
     116,   117,   317,   318,   275,    80,   260,   188,   449,   401,
     460,   187,   187,   188,   188,   189,   312,   317,    35,    80,
     165,   449,   189,   218,   460,    80,   165,    14,    80,   312,
     218,   190,    35,    80,   165,    14,    80,   334,   190,    80,
     165,    14,    80,   334,    14,    80,   334,   334
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
#line 2633 "hphp.y"
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
#line 2648 "hphp.y"
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
#line 2791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2816 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
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
#line 2867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2868 "hphp.y"
    { (yyval).reset();;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2874 "hphp.y"
    { (yyval)++;;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

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

  case 861:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 866:

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

  case 867:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { (yyval).reset();;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 902:

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

  case 903:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
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
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
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
#line 3080 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3090 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3094 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3106 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    {;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    {;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    {;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 959:

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

  case 960:

/* Line 1455 of yacc.c  */
#line 3175 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3180 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3203 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3207 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3217 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3221 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3224 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3230 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3237 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3240 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3252 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3261 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13639 "hphp.5.tab.cpp"
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
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

