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
#define YYLAST   19126

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  210
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  284
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1029
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1865

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
     220,   222,   224,   226,   228,   230,   232,   236,   238,   242,
     244,   248,   250,   252,   255,   259,   264,   266,   269,   273,
     278,   280,   283,   287,   292,   294,   298,   300,   304,   307,
     309,   312,   315,   321,   326,   329,   330,   332,   334,   336,
     338,   342,   348,   357,   358,   363,   364,   371,   372,   383,
     384,   389,   392,   396,   399,   403,   406,   410,   414,   418,
     422,   426,   430,   436,   438,   440,   442,   443,   453,   454,
     465,   471,   472,   486,   487,   493,   497,   501,   504,   507,
     510,   513,   516,   519,   523,   526,   529,   533,   536,   537,
     542,   552,   553,   554,   559,   562,   563,   565,   566,   568,
     569,   579,   580,   591,   592,   604,   605,   615,   616,   627,
     628,   637,   638,   648,   649,   657,   658,   667,   668,   676,
     677,   686,   688,   690,   692,   694,   696,   699,   703,   707,
     710,   713,   714,   717,   718,   721,   722,   724,   728,   730,
     734,   737,   738,   740,   743,   748,   750,   755,   757,   762,
     764,   769,   771,   776,   780,   786,   790,   795,   800,   806,
     812,   817,   818,   820,   822,   827,   828,   834,   835,   838,
     839,   843,   844,   852,   861,   868,   871,   877,   884,   889,
     890,   895,   901,   909,   916,   923,   931,   941,   950,   957,
     965,   971,   974,   979,   985,   989,   990,   994,   999,  1006,
    1012,  1018,  1025,  1034,  1042,  1045,  1046,  1048,  1051,  1054,
    1058,  1063,  1068,  1072,  1074,  1076,  1079,  1084,  1088,  1094,
    1096,  1100,  1103,  1104,  1107,  1111,  1114,  1115,  1116,  1121,
    1122,  1128,  1131,  1134,  1137,  1138,  1149,  1150,  1162,  1166,
    1170,  1174,  1179,  1184,  1188,  1194,  1197,  1200,  1201,  1208,
    1214,  1219,  1223,  1225,  1227,  1231,  1236,  1238,  1241,  1243,
    1245,  1250,  1257,  1259,  1261,  1266,  1268,  1270,  1274,  1277,
    1280,  1281,  1284,  1285,  1287,  1291,  1293,  1295,  1297,  1299,
    1303,  1308,  1313,  1318,  1320,  1322,  1325,  1328,  1331,  1335,
    1339,  1341,  1343,  1345,  1347,  1351,  1353,  1357,  1359,  1361,
    1363,  1364,  1366,  1369,  1371,  1373,  1375,  1377,  1379,  1381,
    1383,  1385,  1386,  1388,  1390,  1392,  1396,  1402,  1404,  1408,
    1414,  1419,  1423,  1427,  1431,  1436,  1440,  1444,  1448,  1451,
    1453,  1455,  1459,  1463,  1465,  1467,  1468,  1470,  1473,  1478,
    1482,  1486,  1493,  1496,  1500,  1507,  1509,  1511,  1513,  1515,
    1517,  1524,  1528,  1533,  1540,  1544,  1548,  1552,  1556,  1560,
    1564,  1568,  1572,  1576,  1580,  1584,  1588,  1591,  1594,  1597,
    1600,  1604,  1608,  1612,  1616,  1620,  1624,  1628,  1632,  1636,
    1640,  1644,  1648,  1652,  1656,  1660,  1664,  1668,  1671,  1674,
    1677,  1680,  1684,  1688,  1692,  1696,  1700,  1704,  1708,  1712,
    1716,  1720,  1724,  1730,  1735,  1737,  1740,  1743,  1746,  1749,
    1752,  1755,  1758,  1761,  1764,  1766,  1768,  1770,  1774,  1777,
    1779,  1785,  1786,  1787,  1799,  1800,  1813,  1814,  1818,  1819,
    1824,  1825,  1832,  1833,  1841,  1842,  1848,  1851,  1854,  1859,
    1861,  1863,  1869,  1873,  1879,  1883,  1886,  1887,  1890,  1891,
    1896,  1901,  1905,  1910,  1915,  1920,  1925,  1927,  1929,  1931,
    1933,  1937,  1940,  1944,  1949,  1952,  1956,  1958,  1961,  1963,
    1966,  1968,  1970,  1972,  1974,  1976,  1978,  1983,  1988,  1991,
    2000,  2011,  2014,  2016,  2020,  2022,  2025,  2027,  2029,  2031,
    2033,  2036,  2041,  2045,  2049,  2054,  2056,  2059,  2064,  2067,
    2074,  2075,  2077,  2082,  2083,  2086,  2087,  2089,  2091,  2095,
    2097,  2101,  2103,  2105,  2109,  2113,  2115,  2117,  2119,  2121,
    2123,  2125,  2127,  2129,  2131,  2133,  2135,  2137,  2139,  2141,
    2143,  2145,  2147,  2149,  2151,  2153,  2155,  2157,  2159,  2161,
    2163,  2165,  2167,  2169,  2171,  2173,  2175,  2177,  2179,  2181,
    2183,  2185,  2187,  2189,  2191,  2193,  2195,  2197,  2199,  2201,
    2203,  2205,  2207,  2209,  2211,  2213,  2215,  2217,  2219,  2221,
    2223,  2225,  2227,  2229,  2231,  2233,  2235,  2237,  2239,  2241,
    2243,  2245,  2247,  2249,  2251,  2253,  2255,  2257,  2259,  2261,
    2263,  2265,  2267,  2269,  2271,  2273,  2278,  2280,  2282,  2284,
    2286,  2288,  2290,  2294,  2296,  2300,  2302,  2304,  2308,  2310,
    2312,  2314,  2317,  2319,  2320,  2321,  2323,  2325,  2329,  2330,
    2332,  2334,  2336,  2338,  2340,  2342,  2344,  2346,  2348,  2350,
    2352,  2354,  2356,  2360,  2363,  2365,  2367,  2372,  2376,  2381,
    2383,  2385,  2389,  2393,  2397,  2401,  2405,  2409,  2413,  2417,
    2421,  2425,  2429,  2433,  2437,  2441,  2445,  2449,  2453,  2457,
    2460,  2463,  2466,  2469,  2473,  2477,  2481,  2485,  2489,  2493,
    2497,  2501,  2505,  2511,  2516,  2520,  2524,  2528,  2530,  2532,
    2534,  2536,  2540,  2544,  2548,  2551,  2552,  2554,  2555,  2557,
    2558,  2564,  2568,  2572,  2574,  2576,  2578,  2580,  2584,  2587,
    2589,  2591,  2593,  2595,  2597,  2601,  2603,  2605,  2607,  2610,
    2613,  2618,  2622,  2627,  2630,  2631,  2637,  2641,  2645,  2647,
    2651,  2653,  2656,  2657,  2663,  2667,  2670,  2671,  2675,  2676,
    2681,  2684,  2685,  2689,  2693,  2695,  2696,  2698,  2700,  2702,
    2704,  2708,  2710,  2712,  2714,  2718,  2720,  2722,  2726,  2730,
    2733,  2738,  2741,  2746,  2752,  2758,  2764,  2770,  2772,  2774,
    2776,  2778,  2780,  2782,  2786,  2790,  2795,  2800,  2804,  2806,
    2808,  2810,  2812,  2816,  2818,  2823,  2827,  2829,  2831,  2833,
    2835,  2837,  2841,  2845,  2850,  2855,  2859,  2861,  2863,  2871,
    2881,  2889,  2896,  2905,  2907,  2910,  2915,  2920,  2922,  2924,
    2929,  2931,  2932,  2934,  2937,  2939,  2941,  2943,  2947,  2951,
    2955,  2956,  2958,  2960,  2964,  2968,  2971,  2975,  2982,  2983,
    2985,  2990,  2993,  2994,  3000,  3004,  3008,  3010,  3017,  3022,
    3027,  3030,  3033,  3034,  3040,  3044,  3048,  3050,  3053,  3054,
    3060,  3064,  3068,  3070,  3073,  3076,  3078,  3081,  3083,  3088,
    3092,  3096,  3103,  3107,  3109,  3111,  3113,  3118,  3123,  3128,
    3133,  3138,  3143,  3146,  3149,  3154,  3157,  3160,  3162,  3166,
    3170,  3174,  3175,  3178,  3184,  3191,  3198,  3206,  3208,  3211,
    3213,  3216,  3218,  3223,  3225,  3230,  3234,  3235,  3237,  3241,
    3244,  3248,  3250,  3252,  3253,  3254,  3257,  3260,  3263,  3268,
    3271,  3277,  3281,  3283,  3285,  3286,  3290,  3295,  3301,  3305,
    3307,  3310,  3311,  3316,  3318,  3322,  3325,  3328,  3331,  3333,
    3335,  3337,  3339,  3343,  3348,  3355,  3357,  3366,  3373,  3375
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     211,     0,    -1,    -1,   212,   213,    -1,   213,   214,    -1,
      -1,   234,    -1,   251,    -1,   258,    -1,   255,    -1,   263,
      -1,   473,    -1,   125,   200,   201,   202,    -1,   152,   226,
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
      90,    -1,    98,    -1,   111,    -1,   122,    -1,   218,    -1,
     126,    -1,   220,     9,   223,    -1,   223,    -1,   224,     9,
     224,    -1,   224,    -1,   225,     9,   225,    -1,   225,    -1,
     226,    -1,   155,   226,    -1,   226,    98,   217,    -1,   155,
     226,    98,   217,    -1,   226,    -1,   155,   226,    -1,   226,
      98,   217,    -1,   155,   226,    98,   217,    -1,   226,    -1,
     155,   226,    -1,   226,    98,   217,    -1,   155,   226,    98,
     217,    -1,   217,    -1,   226,   155,   217,    -1,   226,    -1,
     152,   155,   226,    -1,   155,   226,    -1,   227,    -1,   227,
     478,    -1,   227,   478,    -1,   231,     9,   474,    14,   412,
      -1,   108,   474,    14,   412,    -1,   232,   233,    -1,    -1,
     234,    -1,   251,    -1,   258,    -1,   263,    -1,   203,   232,
     204,    -1,    71,   339,   234,   285,   287,    -1,    71,   339,
      30,   232,   286,   288,    74,   202,    -1,    -1,    90,   339,
     235,   279,    -1,    -1,    89,   236,   234,    90,   339,   202,
      -1,    -1,    92,   200,   341,   202,   341,   202,   341,   201,
     237,   277,    -1,    -1,   100,   339,   238,   282,    -1,   104,
     202,    -1,   104,   348,   202,    -1,   106,   202,    -1,   106,
     348,   202,    -1,   109,   202,    -1,   109,   348,   202,    -1,
      27,   104,   202,    -1,   114,   295,   202,    -1,   120,   297,
     202,    -1,    88,   340,   202,    -1,   144,   340,   202,    -1,
     122,   200,   470,   201,   202,    -1,   202,    -1,    82,    -1,
      83,    -1,    -1,    94,   200,   348,    98,   276,   275,   201,
     239,   278,    -1,    -1,    94,   200,   348,    28,    98,   276,
     275,   201,   240,   278,    -1,    96,   200,   281,   201,   280,
      -1,    -1,   110,   243,   111,   200,   403,    80,   201,   203,
     232,   204,   245,   241,   248,    -1,    -1,   110,   243,   169,
     242,   246,    -1,   112,   348,   202,    -1,   105,   217,   202,
      -1,   348,   202,    -1,   342,   202,    -1,   343,   202,    -1,
     344,   202,    -1,   345,   202,    -1,   346,   202,    -1,   109,
     345,   202,    -1,   347,   202,    -1,   373,   202,    -1,   109,
     372,   202,    -1,   217,    30,    -1,    -1,   203,   244,   232,
     204,    -1,   245,   111,   200,   403,    80,   201,   203,   232,
     204,    -1,    -1,    -1,   203,   247,   232,   204,    -1,   169,
     246,    -1,    -1,    35,    -1,    -1,   107,    -1,    -1,   250,
     249,   477,   252,   200,   291,   201,   482,   325,    -1,    -1,
     329,   250,   249,   477,   253,   200,   291,   201,   482,   325,
      -1,    -1,   433,   328,   250,   249,   477,   254,   200,   291,
     201,   482,   325,    -1,    -1,   162,   217,   256,    30,   492,
     472,   203,   298,   204,    -1,    -1,   433,   162,   217,   257,
      30,   492,   472,   203,   298,   204,    -1,    -1,   269,   266,
     259,   270,   271,   203,   301,   204,    -1,    -1,   433,   269,
     266,   260,   270,   271,   203,   301,   204,    -1,    -1,   127,
     267,   261,   272,   203,   301,   204,    -1,    -1,   433,   127,
     267,   262,   272,   203,   301,   204,    -1,    -1,   164,   268,
     264,   271,   203,   301,   204,    -1,    -1,   433,   164,   268,
     265,   271,   203,   301,   204,    -1,   477,    -1,   156,    -1,
     477,    -1,   477,    -1,   126,    -1,   119,   126,    -1,   119,
     118,   126,    -1,   118,   119,   126,    -1,   118,   126,    -1,
     128,   403,    -1,    -1,   129,   273,    -1,    -1,   128,   273,
      -1,    -1,   403,    -1,   273,     9,   403,    -1,   403,    -1,
     274,     9,   403,    -1,   132,   276,    -1,    -1,   445,    -1,
      35,   445,    -1,   133,   200,   459,   201,    -1,   234,    -1,
      30,   232,    93,   202,    -1,   234,    -1,    30,   232,    95,
     202,    -1,   234,    -1,    30,   232,    91,   202,    -1,   234,
      -1,    30,   232,    97,   202,    -1,   217,    14,   412,    -1,
     281,     9,   217,    14,   412,    -1,   203,   283,   204,    -1,
     203,   202,   283,   204,    -1,    30,   283,   101,   202,    -1,
      30,   202,   283,   101,   202,    -1,   283,   102,   348,   284,
     232,    -1,   283,   103,   284,   232,    -1,    -1,    30,    -1,
     202,    -1,   285,    72,   339,   234,    -1,    -1,   286,    72,
     339,    30,   232,    -1,    -1,    73,   234,    -1,    -1,    73,
      30,   232,    -1,    -1,   290,     9,   434,   331,   493,   165,
      80,    -1,   290,     9,   434,   331,   493,    35,   165,    80,
      -1,   290,     9,   434,   331,   493,   165,    -1,   290,   417,
      -1,   434,   331,   493,   165,    80,    -1,   434,   331,   493,
      35,   165,    80,    -1,   434,   331,   493,   165,    -1,    -1,
     434,   331,   493,    80,    -1,   434,   331,   493,    35,    80,
      -1,   434,   331,   493,    35,    80,    14,   348,    -1,   434,
     331,   493,    80,    14,   348,    -1,   290,     9,   434,   331,
     493,    80,    -1,   290,     9,   434,   331,   493,    35,    80,
      -1,   290,     9,   434,   331,   493,    35,    80,    14,   348,
      -1,   290,     9,   434,   331,   493,    80,    14,   348,    -1,
     292,     9,   434,   493,   165,    80,    -1,   292,     9,   434,
     493,    35,   165,    80,    -1,   292,     9,   434,   493,   165,
      -1,   292,   417,    -1,   434,   493,   165,    80,    -1,   434,
     493,    35,   165,    80,    -1,   434,   493,   165,    -1,    -1,
     434,   493,    80,    -1,   434,   493,    35,    80,    -1,   434,
     493,    35,    80,    14,   348,    -1,   434,   493,    80,    14,
     348,    -1,   292,     9,   434,   493,    80,    -1,   292,     9,
     434,   493,    35,    80,    -1,   292,     9,   434,   493,    35,
      80,    14,   348,    -1,   292,     9,   434,   493,    80,    14,
     348,    -1,   294,   417,    -1,    -1,   348,    -1,    35,   445,
      -1,   165,   348,    -1,   294,     9,   348,    -1,   294,     9,
     165,   348,    -1,   294,     9,    35,   445,    -1,   295,     9,
     296,    -1,   296,    -1,    80,    -1,   205,   445,    -1,   205,
     203,   348,   204,    -1,   297,     9,    80,    -1,   297,     9,
      80,    14,   412,    -1,    80,    -1,    80,    14,   412,    -1,
     298,   299,    -1,    -1,   300,   202,    -1,   475,    14,   412,
      -1,   301,   302,    -1,    -1,    -1,   327,   303,   333,   202,
      -1,    -1,   329,   492,   304,   333,   202,    -1,   334,   202,
      -1,   335,   202,    -1,   336,   202,    -1,    -1,   328,   250,
     249,   476,   200,   305,   289,   201,   482,   326,    -1,    -1,
     433,   328,   250,   249,   477,   200,   306,   289,   201,   482,
     326,    -1,   158,   311,   202,    -1,   159,   319,   202,    -1,
     161,   321,   202,    -1,     4,   128,   403,   202,    -1,     4,
     129,   403,   202,    -1,   113,   274,   202,    -1,   113,   274,
     203,   307,   204,    -1,   307,   308,    -1,   307,   309,    -1,
      -1,   230,   151,   217,   166,   274,   202,    -1,   310,    98,
     328,   217,   202,    -1,   310,    98,   329,   202,    -1,   230,
     151,   217,    -1,   217,    -1,   312,    -1,   311,     9,   312,
      -1,   313,   400,   317,   318,    -1,   156,    -1,    29,   314,
      -1,   314,    -1,   134,    -1,   134,   172,   492,   173,    -1,
     134,   172,   492,     9,   492,   173,    -1,   403,    -1,   121,
      -1,   162,   203,   316,   204,    -1,   135,    -1,   411,    -1,
     315,     9,   411,    -1,   315,   416,    -1,    14,   412,    -1,
      -1,    56,   163,    -1,    -1,   320,    -1,   319,     9,   320,
      -1,   160,    -1,   322,    -1,   217,    -1,   124,    -1,   200,
     323,   201,    -1,   200,   323,   201,    50,    -1,   200,   323,
     201,    29,    -1,   200,   323,   201,    47,    -1,   322,    -1,
     324,    -1,   324,    50,    -1,   324,    29,    -1,   324,    47,
      -1,   323,     9,   323,    -1,   323,    33,   323,    -1,   217,
      -1,   156,    -1,   160,    -1,   202,    -1,   203,   232,   204,
      -1,   202,    -1,   203,   232,   204,    -1,   329,    -1,   121,
      -1,   329,    -1,    -1,   330,    -1,   329,   330,    -1,   115,
      -1,   116,    -1,   117,    -1,   120,    -1,   119,    -1,   118,
      -1,   182,    -1,   332,    -1,    -1,   115,    -1,   116,    -1,
     117,    -1,   333,     9,    80,    -1,   333,     9,    80,    14,
     412,    -1,    80,    -1,    80,    14,   412,    -1,   334,     9,
     475,    14,   412,    -1,   108,   475,    14,   412,    -1,   335,
       9,   475,    -1,   119,   108,   475,    -1,   119,   337,   472,
      -1,   337,   472,    14,   492,    -1,   108,   177,   477,    -1,
     200,   338,   201,    -1,    69,   407,   410,    -1,    68,   348,
      -1,   392,    -1,   368,    -1,   200,   348,   201,    -1,   340,
       9,   348,    -1,   348,    -1,   340,    -1,    -1,    27,    -1,
      27,   348,    -1,    27,   348,   132,   348,    -1,   200,   342,
     201,    -1,   445,    14,   342,    -1,   133,   200,   459,   201,
      14,   342,    -1,    28,   348,    -1,   445,    14,   345,    -1,
     133,   200,   459,   201,    14,   345,    -1,   349,    -1,   445,
      -1,   338,    -1,   449,    -1,   448,    -1,   133,   200,   459,
     201,    14,   348,    -1,   445,    14,   348,    -1,   445,    14,
      35,   445,    -1,   445,    14,    35,    69,   407,   410,    -1,
     445,    26,   348,    -1,   445,    25,   348,    -1,   445,    24,
     348,    -1,   445,    23,   348,    -1,   445,    22,   348,    -1,
     445,    21,   348,    -1,   445,    20,   348,    -1,   445,    19,
     348,    -1,   445,    18,   348,    -1,   445,    17,   348,    -1,
     445,    16,   348,    -1,   445,    15,   348,    -1,   445,    65,
      -1,    65,   445,    -1,   445,    64,    -1,    64,   445,    -1,
     348,    31,   348,    -1,   348,    32,   348,    -1,   348,    10,
     348,    -1,   348,    12,   348,    -1,   348,    11,   348,    -1,
     348,    33,   348,    -1,   348,    35,   348,    -1,   348,    34,
     348,    -1,   348,    49,   348,    -1,   348,    47,   348,    -1,
     348,    48,   348,    -1,   348,    50,   348,    -1,   348,    51,
     348,    -1,   348,    66,   348,    -1,   348,    52,   348,    -1,
     348,    46,   348,    -1,   348,    45,   348,    -1,    47,   348,
      -1,    48,   348,    -1,    53,   348,    -1,    55,   348,    -1,
     348,    37,   348,    -1,   348,    36,   348,    -1,   348,    39,
     348,    -1,   348,    38,   348,    -1,   348,    40,   348,    -1,
     348,    44,   348,    -1,   348,    41,   348,    -1,   348,    43,
     348,    -1,   348,    42,   348,    -1,   348,    54,   407,    -1,
     200,   349,   201,    -1,   348,    29,   348,    30,   348,    -1,
     348,    29,    30,   348,    -1,   469,    -1,    63,   348,    -1,
      62,   348,    -1,    61,   348,    -1,    60,   348,    -1,    59,
     348,    -1,    58,   348,    -1,    57,   348,    -1,    70,   408,
      -1,    56,   348,    -1,   414,    -1,   367,    -1,   366,    -1,
     206,   409,   206,    -1,    13,   348,    -1,   370,    -1,   113,
     200,   391,   417,   201,    -1,    -1,    -1,   250,   249,   200,
     352,   291,   201,   482,   350,   203,   232,   204,    -1,    -1,
     329,   250,   249,   200,   353,   291,   201,   482,   350,   203,
     232,   204,    -1,    -1,    80,   355,   360,    -1,    -1,   182,
      80,   356,   360,    -1,    -1,   197,   357,   291,   198,   482,
     360,    -1,    -1,   182,   197,   358,   291,   198,   482,   360,
      -1,    -1,   182,   203,   359,   232,   204,    -1,     8,   348,
      -1,     8,   345,    -1,     8,   203,   232,   204,    -1,    87,
      -1,   471,    -1,   362,     9,   361,   132,   348,    -1,   361,
     132,   348,    -1,   363,     9,   361,   132,   412,    -1,   361,
     132,   412,    -1,   362,   416,    -1,    -1,   363,   416,    -1,
      -1,   176,   200,   364,   201,    -1,   134,   200,   460,   201,
      -1,    67,   460,   207,    -1,   403,   203,   462,   204,    -1,
     403,   203,   464,   204,    -1,   370,    67,   455,   207,    -1,
     371,    67,   455,   207,    -1,   367,    -1,   471,    -1,   448,
      -1,    87,    -1,   200,   349,   201,    -1,   374,   375,    -1,
     445,    14,   372,    -1,   183,    80,   186,   348,    -1,   376,
     387,    -1,   376,   387,   390,    -1,   387,    -1,   387,   390,
      -1,   377,    -1,   376,   377,    -1,   378,    -1,   379,    -1,
     380,    -1,   381,    -1,   382,    -1,   383,    -1,   183,    80,
     186,   348,    -1,   190,    80,    14,   348,    -1,   184,   348,
      -1,   185,    80,   186,   348,   187,   348,   188,   348,    -1,
     185,    80,   186,   348,   187,   348,   188,   348,   189,    80,
      -1,   191,   384,    -1,   385,    -1,   384,     9,   385,    -1,
     348,    -1,   348,   386,    -1,   192,    -1,   193,    -1,   388,
      -1,   389,    -1,   194,   348,    -1,   195,   348,   196,   348,
      -1,   189,    80,   375,    -1,   391,     9,    80,    -1,   391,
       9,    35,    80,    -1,    80,    -1,    35,    80,    -1,   170,
     156,   393,   171,    -1,   395,    51,    -1,   395,   171,   396,
     170,    51,   394,    -1,    -1,   156,    -1,   395,   397,    14,
     398,    -1,    -1,   396,   399,    -1,    -1,   156,    -1,   157,
      -1,   203,   348,   204,    -1,   157,    -1,   203,   348,   204,
      -1,   392,    -1,   401,    -1,   400,    30,   401,    -1,   400,
      48,   401,    -1,   217,    -1,    70,    -1,   107,    -1,   108,
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
      -1,   176,    -1,   229,   200,   293,   201,    -1,   230,    -1,
     156,    -1,   403,    -1,   411,    -1,   120,    -1,   453,    -1,
     200,   349,   201,    -1,   404,    -1,   405,   151,   452,    -1,
     404,    -1,   451,    -1,   406,   151,   452,    -1,   403,    -1,
     120,    -1,   457,    -1,   200,   201,    -1,   339,    -1,    -1,
      -1,    86,    -1,   466,    -1,   200,   293,   201,    -1,    -1,
      75,    -1,    76,    -1,    77,    -1,    87,    -1,   139,    -1,
     140,    -1,   154,    -1,   136,    -1,   167,    -1,   137,    -1,
     138,    -1,   153,    -1,   181,    -1,   147,    86,   148,    -1,
     147,   148,    -1,   411,    -1,   228,    -1,   134,   200,   415,
     201,    -1,    67,   415,   207,    -1,   176,   200,   365,   201,
      -1,   413,    -1,   369,    -1,   200,   412,   201,    -1,   412,
      31,   412,    -1,   412,    32,   412,    -1,   412,    10,   412,
      -1,   412,    12,   412,    -1,   412,    11,   412,    -1,   412,
      33,   412,    -1,   412,    35,   412,    -1,   412,    34,   412,
      -1,   412,    49,   412,    -1,   412,    47,   412,    -1,   412,
      48,   412,    -1,   412,    50,   412,    -1,   412,    51,   412,
      -1,   412,    52,   412,    -1,   412,    46,   412,    -1,   412,
      45,   412,    -1,   412,    66,   412,    -1,    53,   412,    -1,
      55,   412,    -1,    47,   412,    -1,    48,   412,    -1,   412,
      37,   412,    -1,   412,    36,   412,    -1,   412,    39,   412,
      -1,   412,    38,   412,    -1,   412,    40,   412,    -1,   412,
      44,   412,    -1,   412,    41,   412,    -1,   412,    43,   412,
      -1,   412,    42,   412,    -1,   412,    29,   412,    30,   412,
      -1,   412,    29,    30,   412,    -1,   230,   151,   218,    -1,
     156,   151,   218,    -1,   230,   151,   126,    -1,   228,    -1,
      79,    -1,   471,    -1,   411,    -1,   208,   466,   208,    -1,
     209,   466,   209,    -1,   147,   466,   148,    -1,   418,   416,
      -1,    -1,     9,    -1,    -1,     9,    -1,    -1,   418,     9,
     412,   132,   412,    -1,   418,     9,   412,    -1,   412,   132,
     412,    -1,   412,    -1,    75,    -1,    76,    -1,    77,    -1,
     147,    86,   148,    -1,   147,   148,    -1,    75,    -1,    76,
      -1,    77,    -1,   217,    -1,    87,    -1,    87,    49,   421,
      -1,   419,    -1,   421,    -1,   217,    -1,    47,   420,    -1,
      48,   420,    -1,   134,   200,   423,   201,    -1,    67,   423,
     207,    -1,   176,   200,   426,   201,    -1,   424,   416,    -1,
      -1,   424,     9,   422,   132,   422,    -1,   424,     9,   422,
      -1,   422,   132,   422,    -1,   422,    -1,   425,     9,   422,
      -1,   422,    -1,   427,   416,    -1,    -1,   427,     9,   361,
     132,   422,    -1,   361,   132,   422,    -1,   425,   416,    -1,
      -1,   200,   428,   201,    -1,    -1,   430,     9,   217,   429,
      -1,   217,   429,    -1,    -1,   432,   430,   416,    -1,    46,
     431,    45,    -1,   433,    -1,    -1,   130,    -1,   131,    -1,
     217,    -1,   156,    -1,   203,   348,   204,    -1,   436,    -1,
     452,    -1,   217,    -1,   203,   348,   204,    -1,   438,    -1,
     452,    -1,    67,   455,   207,    -1,   203,   348,   204,    -1,
     446,   440,    -1,   200,   338,   201,   440,    -1,   458,   440,
      -1,   200,   338,   201,   440,    -1,   200,   338,   201,   435,
     437,    -1,   200,   349,   201,   435,   437,    -1,   200,   338,
     201,   435,   436,    -1,   200,   349,   201,   435,   436,    -1,
     452,    -1,   402,    -1,   450,    -1,   451,    -1,   441,    -1,
     443,    -1,   445,   435,   437,    -1,   406,   151,   452,    -1,
     447,   200,   293,   201,    -1,   448,   200,   293,   201,    -1,
     200,   445,   201,    -1,   402,    -1,   450,    -1,   451,    -1,
     441,    -1,   445,   435,   436,    -1,   444,    -1,   447,   200,
     293,   201,    -1,   200,   445,   201,    -1,   452,    -1,   441,
      -1,   402,    -1,   367,    -1,   411,    -1,   200,   445,   201,
      -1,   200,   349,   201,    -1,   448,   200,   293,   201,    -1,
     447,   200,   293,   201,    -1,   200,   449,   201,    -1,   351,
      -1,   354,    -1,   445,   435,   439,   478,   200,   293,   201,
      -1,   200,   338,   201,   435,   439,   478,   200,   293,   201,
      -1,   406,   151,   219,   478,   200,   293,   201,    -1,   406,
     151,   452,   200,   293,   201,    -1,   406,   151,   203,   348,
     204,   200,   293,   201,    -1,   453,    -1,   456,   453,    -1,
     453,    67,   455,   207,    -1,   453,   203,   348,   204,    -1,
     454,    -1,    80,    -1,   205,   203,   348,   204,    -1,   348,
      -1,    -1,   205,    -1,   456,   205,    -1,   452,    -1,   442,
      -1,   443,    -1,   457,   435,   437,    -1,   405,   151,   452,
      -1,   200,   445,   201,    -1,    -1,   442,    -1,   444,    -1,
     457,   435,   436,    -1,   200,   445,   201,    -1,   459,     9,
      -1,   459,     9,   445,    -1,   459,     9,   133,   200,   459,
     201,    -1,    -1,   445,    -1,   133,   200,   459,   201,    -1,
     461,   416,    -1,    -1,   461,     9,   348,   132,   348,    -1,
     461,     9,   348,    -1,   348,   132,   348,    -1,   348,    -1,
     461,     9,   348,   132,    35,   445,    -1,   461,     9,    35,
     445,    -1,   348,   132,    35,   445,    -1,    35,   445,    -1,
     463,   416,    -1,    -1,   463,     9,   348,   132,   348,    -1,
     463,     9,   348,    -1,   348,   132,   348,    -1,   348,    -1,
     465,   416,    -1,    -1,   465,     9,   412,   132,   412,    -1,
     465,     9,   412,    -1,   412,   132,   412,    -1,   412,    -1,
     466,   467,    -1,   466,    86,    -1,   467,    -1,    86,   467,
      -1,    80,    -1,    80,    67,   468,   207,    -1,    80,   435,
     217,    -1,   149,   348,   204,    -1,   149,    79,    67,   348,
     207,   204,    -1,   150,   445,   204,    -1,   217,    -1,    81,
      -1,    80,    -1,   123,   200,   340,   201,    -1,   124,   200,
     445,   201,    -1,   124,   200,   349,   201,    -1,   124,   200,
     449,   201,    -1,   124,   200,   448,   201,    -1,   124,   200,
     338,   201,    -1,     7,   348,    -1,     6,   348,    -1,     5,
     200,   348,   201,    -1,     4,   348,    -1,     3,   348,    -1,
     445,    -1,   470,     9,   445,    -1,   406,   151,   218,    -1,
     406,   151,   126,    -1,    -1,    98,   492,    -1,   177,   477,
      14,   492,   202,    -1,   433,   177,   477,    14,   492,   202,
      -1,   179,   477,   472,    14,   492,   202,    -1,   433,   179,
     477,   472,    14,   492,   202,    -1,   219,    -1,   492,   219,
      -1,   218,    -1,   492,   218,    -1,   219,    -1,   219,   172,
     484,   173,    -1,   217,    -1,   217,   172,   484,   173,    -1,
     172,   480,   173,    -1,    -1,   492,    -1,   479,     9,   492,
      -1,   479,   416,    -1,   479,     9,   165,    -1,   480,    -1,
     165,    -1,    -1,    -1,    30,   492,    -1,    98,   492,    -1,
      99,   492,    -1,   484,     9,   485,   217,    -1,   485,   217,
      -1,   484,     9,   485,   217,   483,    -1,   485,   217,   483,
      -1,    47,    -1,    48,    -1,    -1,    87,   132,   492,    -1,
      29,    87,   132,   492,    -1,   230,   151,   217,   132,   492,
      -1,   487,     9,   486,    -1,   486,    -1,   487,   416,    -1,
      -1,   176,   200,   488,   201,    -1,   230,    -1,   217,   151,
     491,    -1,   217,   478,    -1,    29,   492,    -1,    56,   492,
      -1,   230,    -1,   134,    -1,   135,    -1,   489,    -1,   490,
     151,   491,    -1,   134,   172,   492,   173,    -1,   134,   172,
     492,     9,   492,   173,    -1,   156,    -1,   200,   107,   200,
     481,   201,    30,   492,   201,    -1,   200,   492,     9,   479,
     416,   201,    -1,   492,    -1,    -1
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
     860,   861,   862,   864,   868,   869,   873,   875,   879,   881,
     885,   887,   891,   892,   893,   894,   899,   900,   901,   902,
     907,   908,   909,   910,   915,   916,   920,   921,   923,   926,
     932,   939,   946,   950,   956,   958,   961,   962,   963,   964,
     967,   968,   972,   977,   977,   983,   983,   990,   989,   995,
     995,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1018,  1016,  1025,  1023,
    1030,  1038,  1032,  1042,  1040,  1044,  1045,  1049,  1050,  1051,
    1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1067,  1067,
    1072,  1078,  1082,  1082,  1090,  1091,  1095,  1096,  1100,  1106,
    1104,  1119,  1116,  1132,  1129,  1146,  1145,  1154,  1152,  1164,
    1163,  1182,  1180,  1199,  1198,  1207,  1205,  1217,  1216,  1228,
    1226,  1239,  1240,  1244,  1247,  1250,  1251,  1252,  1255,  1256,
    1259,  1261,  1264,  1265,  1268,  1269,  1272,  1273,  1277,  1278,
    1283,  1284,  1287,  1288,  1289,  1293,  1294,  1298,  1299,  1303,
    1304,  1308,  1309,  1314,  1315,  1320,  1321,  1322,  1323,  1326,
    1329,  1331,  1334,  1335,  1339,  1341,  1344,  1347,  1350,  1351,
    1354,  1355,  1359,  1365,  1371,  1378,  1380,  1385,  1390,  1396,
    1400,  1404,  1408,  1413,  1418,  1423,  1428,  1434,  1443,  1448,
    1453,  1459,  1461,  1465,  1469,  1474,  1478,  1481,  1484,  1488,
    1492,  1496,  1500,  1505,  1513,  1515,  1518,  1519,  1520,  1521,
    1523,  1525,  1530,  1531,  1534,  1535,  1536,  1540,  1541,  1543,
    1544,  1548,  1550,  1553,  1557,  1563,  1565,  1568,  1568,  1572,
    1571,  1575,  1577,  1580,  1583,  1581,  1597,  1593,  1607,  1609,
    1611,  1613,  1615,  1617,  1619,  1623,  1624,  1625,  1628,  1634,
    1638,  1644,  1647,  1652,  1654,  1659,  1664,  1668,  1669,  1673,
    1674,  1676,  1678,  1684,  1685,  1687,  1691,  1692,  1697,  1701,
    1702,  1706,  1707,  1711,  1713,  1719,  1724,  1725,  1727,  1731,
    1732,  1733,  1734,  1738,  1739,  1740,  1741,  1742,  1743,  1745,
    1750,  1753,  1754,  1758,  1759,  1763,  1764,  1767,  1768,  1771,
    1772,  1775,  1776,  1780,  1781,  1782,  1783,  1784,  1785,  1786,
    1790,  1791,  1794,  1795,  1796,  1799,  1801,  1803,  1804,  1807,
    1809,  1813,  1815,  1819,  1823,  1827,  1832,  1833,  1835,  1836,
    1837,  1840,  1844,  1845,  1849,  1850,  1854,  1855,  1856,  1857,
    1861,  1865,  1870,  1874,  1878,  1883,  1884,  1885,  1886,  1887,
    1891,  1893,  1894,  1895,  1898,  1899,  1900,  1901,  1902,  1903,
    1904,  1905,  1906,  1907,  1908,  1909,  1910,  1911,  1912,  1913,
    1914,  1915,  1916,  1917,  1918,  1919,  1920,  1921,  1922,  1923,
    1924,  1925,  1926,  1927,  1928,  1929,  1930,  1931,  1932,  1933,
    1934,  1935,  1936,  1937,  1938,  1939,  1940,  1942,  1943,  1945,
    1946,  1948,  1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,
    1957,  1958,  1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,
    1970,  1974,  1979,  1978,  1993,  1991,  2008,  2008,  2024,  2023,
    2041,  2041,  2057,  2056,  2075,  2074,  2095,  2096,  2097,  2102,
    2104,  2108,  2112,  2118,  2122,  2128,  2130,  2134,  2136,  2140,
    2144,  2145,  2149,  2156,  2163,  2165,  2170,  2171,  2172,  2173,
    2175,  2179,  2183,  2187,  2191,  2193,  2195,  2197,  2202,  2203,
    2208,  2209,  2210,  2211,  2212,  2213,  2217,  2221,  2225,  2229,
    2234,  2239,  2243,  2244,  2248,  2249,  2253,  2254,  2258,  2259,
    2263,  2267,  2271,  2275,  2276,  2277,  2278,  2282,  2288,  2297,
    2310,  2311,  2314,  2317,  2320,  2321,  2324,  2328,  2331,  2334,
    2341,  2342,  2346,  2347,  2349,  2353,  2354,  2355,  2356,  2357,
    2358,  2359,  2360,  2361,  2362,  2363,  2364,  2365,  2366,  2367,
    2368,  2369,  2370,  2371,  2372,  2373,  2374,  2375,  2376,  2377,
    2378,  2379,  2380,  2381,  2382,  2383,  2384,  2385,  2386,  2387,
    2388,  2389,  2390,  2391,  2392,  2393,  2394,  2395,  2396,  2397,
    2398,  2399,  2400,  2401,  2402,  2403,  2404,  2405,  2406,  2407,
    2408,  2409,  2410,  2411,  2412,  2413,  2414,  2415,  2416,  2417,
    2418,  2419,  2420,  2421,  2422,  2423,  2424,  2425,  2426,  2427,
    2428,  2429,  2430,  2431,  2432,  2436,  2441,  2442,  2446,  2447,
    2448,  2449,  2451,  2455,  2456,  2467,  2468,  2470,  2482,  2483,
    2484,  2488,  2489,  2490,  2494,  2495,  2496,  2499,  2501,  2505,
    2506,  2507,  2508,  2510,  2511,  2512,  2513,  2514,  2515,  2516,
    2517,  2518,  2519,  2522,  2527,  2528,  2529,  2531,  2532,  2534,
    2535,  2536,  2537,  2539,  2541,  2543,  2545,  2547,  2548,  2549,
    2550,  2551,  2552,  2553,  2554,  2555,  2556,  2557,  2558,  2559,
    2560,  2561,  2562,  2563,  2565,  2567,  2569,  2571,  2572,  2575,
    2576,  2580,  2584,  2586,  2590,  2593,  2596,  2602,  2603,  2604,
    2605,  2606,  2607,  2608,  2613,  2615,  2619,  2620,  2623,  2624,
    2628,  2631,  2633,  2635,  2639,  2640,  2641,  2642,  2645,  2649,
    2650,  2651,  2652,  2656,  2658,  2665,  2666,  2667,  2668,  2669,
    2670,  2672,  2673,  2678,  2680,  2683,  2686,  2688,  2690,  2693,
    2695,  2699,  2701,  2704,  2707,  2713,  2715,  2718,  2719,  2724,
    2727,  2731,  2731,  2736,  2739,  2740,  2744,  2745,  2749,  2750,
    2751,  2755,  2757,  2765,  2766,  2770,  2772,  2780,  2781,  2785,
    2786,  2791,  2793,  2798,  2809,  2823,  2835,  2850,  2851,  2852,
    2853,  2854,  2855,  2856,  2866,  2875,  2877,  2879,  2883,  2884,
    2885,  2886,  2887,  2903,  2904,  2906,  2915,  2916,  2917,  2918,
    2919,  2920,  2921,  2922,  2924,  2929,  2933,  2934,  2938,  2941,
    2948,  2952,  2961,  2968,  2970,  2976,  2978,  2979,  2983,  2984,
    2991,  2992,  2997,  2998,  3003,  3004,  3005,  3006,  3017,  3020,
    3023,  3024,  3025,  3026,  3037,  3041,  3042,  3043,  3045,  3046,
    3047,  3051,  3053,  3056,  3058,  3059,  3060,  3061,  3064,  3066,
    3067,  3071,  3073,  3076,  3078,  3079,  3080,  3084,  3086,  3089,
    3092,  3094,  3096,  3100,  3101,  3103,  3104,  3110,  3111,  3113,
    3123,  3125,  3127,  3130,  3131,  3132,  3136,  3137,  3138,  3139,
    3140,  3141,  3142,  3143,  3144,  3145,  3146,  3150,  3151,  3155,
    3157,  3165,  3167,  3171,  3175,  3180,  3184,  3192,  3193,  3197,
    3198,  3204,  3205,  3214,  3215,  3223,  3226,  3230,  3233,  3238,
    3243,  3245,  3246,  3247,  3251,  3252,  3256,  3257,  3260,  3263,
    3265,  3269,  3275,  3276,  3277,  3281,  3285,  3295,  3303,  3305,
    3309,  3311,  3316,  3322,  3325,  3330,  3338,  3341,  3344,  3345,
    3348,  3351,  3352,  3357,  3360,  3364,  3368,  3374,  3384,  3385
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
     218,   218,   218,   218,   219,   219,   220,   220,   221,   221,
     222,   222,   223,   223,   223,   223,   224,   224,   224,   224,
     225,   225,   225,   225,   226,   226,   227,   227,   227,   228,
     229,   230,   231,   231,   232,   232,   233,   233,   233,   233,
     234,   234,   234,   235,   234,   236,   234,   237,   234,   238,
     234,   234,   234,   234,   234,   234,   234,   234,   234,   234,
     234,   234,   234,   234,   234,   234,   239,   234,   240,   234,
     234,   241,   234,   242,   234,   234,   234,   234,   234,   234,
     234,   234,   234,   234,   234,   234,   234,   234,   244,   243,
     245,   245,   247,   246,   248,   248,   249,   249,   250,   252,
     251,   253,   251,   254,   251,   256,   255,   257,   255,   259,
     258,   260,   258,   261,   258,   262,   258,   264,   263,   265,
     263,   266,   266,   267,   268,   269,   269,   269,   269,   269,
     270,   270,   271,   271,   272,   272,   273,   273,   274,   274,
     275,   275,   276,   276,   276,   277,   277,   278,   278,   279,
     279,   280,   280,   281,   281,   282,   282,   282,   282,   283,
     283,   283,   284,   284,   285,   285,   286,   286,   287,   287,
     288,   288,   289,   289,   289,   289,   289,   289,   289,   289,
     290,   290,   290,   290,   290,   290,   290,   290,   291,   291,
     291,   291,   291,   291,   291,   291,   292,   292,   292,   292,
     292,   292,   292,   292,   293,   293,   294,   294,   294,   294,
     294,   294,   295,   295,   296,   296,   296,   297,   297,   297,
     297,   298,   298,   299,   300,   301,   301,   303,   302,   304,
     302,   302,   302,   302,   305,   302,   306,   302,   302,   302,
     302,   302,   302,   302,   302,   307,   307,   307,   308,   309,
     309,   310,   310,   311,   311,   312,   312,   313,   313,   314,
     314,   314,   314,   314,   314,   314,   315,   315,   316,   317,
     317,   318,   318,   319,   319,   320,   321,   321,   321,   322,
     322,   322,   322,   323,   323,   323,   323,   323,   323,   323,
     324,   324,   324,   325,   325,   326,   326,   327,   327,   328,
     328,   329,   329,   330,   330,   330,   330,   330,   330,   330,
     331,   331,   332,   332,   332,   333,   333,   333,   333,   334,
     334,   335,   335,   336,   336,   337,   338,   338,   338,   338,
     338,   339,   340,   340,   341,   341,   342,   342,   342,   342,
     343,   344,   345,   346,   347,   348,   348,   348,   348,   348,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     350,   350,   352,   351,   353,   351,   355,   354,   356,   354,
     357,   354,   358,   354,   359,   354,   360,   360,   360,   361,
     361,   362,   362,   363,   363,   364,   364,   365,   365,   366,
     367,   367,   368,   369,   370,   370,   371,   371,   371,   371,
     371,   372,   373,   374,   375,   375,   375,   375,   376,   376,
     377,   377,   377,   377,   377,   377,   378,   379,   380,   381,
     382,   383,   384,   384,   385,   385,   386,   386,   387,   387,
     388,   389,   390,   391,   391,   391,   391,   392,   393,   393,
     394,   394,   395,   395,   396,   396,   397,   398,   398,   399,
     399,   399,   400,   400,   400,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   402,   403,   403,   404,   404,
     404,   404,   404,   405,   405,   406,   406,   406,   407,   407,
     407,   408,   408,   408,   409,   409,   409,   410,   410,   411,
     411,   411,   411,   411,   411,   411,   411,   411,   411,   411,
     411,   411,   411,   411,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   413,   413,   413,   414,   414,   414,
     414,   414,   414,   414,   415,   415,   416,   416,   417,   417,
     418,   418,   418,   418,   419,   419,   419,   419,   419,   420,
     420,   420,   420,   421,   421,   422,   422,   422,   422,   422,
     422,   422,   422,   423,   423,   424,   424,   424,   424,   425,
     425,   426,   426,   427,   427,   428,   428,   429,   429,   430,
     430,   432,   431,   433,   434,   434,   435,   435,   436,   436,
     436,   437,   437,   438,   438,   439,   439,   440,   440,   441,
     441,   442,   442,   443,   443,   444,   444,   445,   445,   445,
     445,   445,   445,   445,   445,   445,   445,   445,   446,   446,
     446,   446,   446,   446,   446,   446,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   448,   449,   449,   450,   450,
     451,   451,   451,   452,   452,   453,   453,   453,   454,   454,
     455,   455,   456,   456,   457,   457,   457,   457,   457,   457,
     458,   458,   458,   458,   458,   459,   459,   459,   459,   459,
     459,   460,   460,   461,   461,   461,   461,   461,   461,   461,
     461,   462,   462,   463,   463,   463,   463,   464,   464,   465,
     465,   465,   465,   466,   466,   466,   466,   467,   467,   467,
     467,   467,   467,   468,   468,   468,   469,   469,   469,   469,
     469,   469,   469,   469,   469,   469,   469,   470,   470,   471,
     471,   472,   472,   473,   473,   473,   473,   474,   474,   475,
     475,   476,   476,   477,   477,   478,   478,   479,   479,   480,
     481,   481,   481,   481,   482,   482,   483,   483,   484,   484,
     484,   484,   485,   485,   485,   486,   486,   486,   487,   487,
     488,   488,   489,   490,   491,   491,   492,   492,   492,   492,
     492,   492,   492,   492,   492,   492,   492,   492,   493,   493
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
       1,     1,     1,     1,     1,     1,     3,     1,     3,     1,
       3,     1,     1,     2,     3,     4,     1,     2,     3,     4,
       1,     2,     3,     4,     1,     3,     1,     3,     2,     1,
       2,     2,     5,     4,     2,     0,     1,     1,     1,     1,
       3,     5,     8,     0,     4,     0,     6,     0,    10,     0,
       4,     2,     3,     2,     3,     2,     3,     3,     3,     3,
       3,     3,     5,     1,     1,     1,     0,     9,     0,    10,
       5,     0,    13,     0,     5,     3,     3,     2,     2,     2,
       2,     2,     2,     3,     2,     2,     3,     2,     0,     4,
       9,     0,     0,     4,     2,     0,     1,     0,     1,     0,
       9,     0,    10,     0,    11,     0,     9,     0,    10,     0,
       8,     0,     9,     0,     7,     0,     8,     0,     7,     0,
       8,     1,     1,     1,     1,     1,     2,     3,     3,     2,
       2,     0,     2,     0,     2,     0,     1,     3,     1,     3,
       2,     0,     1,     2,     4,     1,     4,     1,     4,     1,
       4,     1,     4,     3,     5,     3,     4,     4,     5,     5,
       4,     0,     1,     1,     4,     0,     5,     0,     2,     0,
       3,     0,     7,     8,     6,     2,     5,     6,     4,     0,
       4,     5,     7,     6,     6,     7,     9,     8,     6,     7,
       5,     2,     4,     5,     3,     0,     3,     4,     6,     5,
       5,     6,     8,     7,     2,     0,     1,     2,     2,     3,
       4,     4,     3,     1,     1,     2,     4,     3,     5,     1,
       3,     2,     0,     2,     3,     2,     0,     0,     4,     0,
       5,     2,     2,     2,     0,    10,     0,    11,     3,     3,
       3,     4,     4,     3,     5,     2,     2,     0,     6,     5,
       4,     3,     1,     1,     3,     4,     1,     2,     1,     1,
       4,     6,     1,     1,     4,     1,     1,     3,     2,     2,
       0,     2,     0,     1,     3,     1,     1,     1,     1,     3,
       4,     4,     4,     1,     1,     2,     2,     2,     3,     3,
       1,     1,     1,     1,     3,     1,     3,     1,     1,     1,
       0,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     1,     1,     1,     3,     5,     1,     3,     5,
       4,     3,     3,     3,     4,     3,     3,     3,     2,     1,
       1,     3,     3,     1,     1,     0,     1,     2,     4,     3,
       3,     6,     2,     3,     6,     1,     1,     1,     1,     1,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     4,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     1,     1,     3,     2,     1,
       5,     0,     0,    11,     0,    12,     0,     3,     0,     4,
       0,     6,     0,     7,     0,     5,     2,     2,     4,     1,
       1,     5,     3,     5,     3,     2,     0,     2,     0,     4,
       4,     3,     4,     4,     4,     4,     1,     1,     1,     1,
       3,     2,     3,     4,     2,     3,     1,     2,     1,     2,
       1,     1,     1,     1,     1,     1,     4,     4,     2,     8,
      10,     2,     1,     3,     1,     2,     1,     1,     1,     1,
       2,     4,     3,     3,     4,     1,     2,     4,     2,     6,
       0,     1,     4,     0,     2,     0,     1,     1,     3,     1,
       3,     1,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     1,     1,     1,     1,
       1,     1,     3,     1,     3,     1,     1,     3,     1,     1,
       1,     2,     1,     0,     0,     1,     1,     3,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     4,     3,     4,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     4,     3,     3,     3,     1,     1,     1,
       1,     3,     3,     3,     2,     0,     1,     0,     1,     0,
       5,     3,     3,     1,     1,     1,     1,     3,     2,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     2,     2,
       4,     3,     4,     2,     0,     5,     3,     3,     1,     3,
       1,     2,     0,     5,     3,     2,     0,     3,     0,     4,
       2,     0,     3,     3,     1,     0,     1,     1,     1,     1,
       3,     1,     1,     1,     3,     1,     1,     3,     3,     2,
       4,     2,     4,     5,     5,     5,     5,     1,     1,     1,
       1,     1,     1,     3,     3,     4,     4,     3,     1,     1,
       1,     1,     3,     1,     4,     3,     1,     1,     1,     1,
       1,     3,     3,     4,     4,     3,     1,     1,     7,     9,
       7,     6,     8,     1,     2,     4,     4,     1,     1,     4,
       1,     0,     1,     2,     1,     1,     1,     3,     3,     3,
       0,     1,     1,     3,     3,     2,     3,     6,     0,     1,
       4,     2,     0,     5,     3,     3,     1,     6,     4,     4,
       2,     2,     0,     5,     3,     3,     1,     2,     0,     5,
       3,     3,     1,     2,     2,     1,     2,     1,     4,     3,
       3,     6,     3,     1,     1,     1,     4,     4,     4,     4,
       4,     4,     2,     2,     4,     2,     2,     1,     3,     3,
       3,     0,     2,     5,     6,     6,     7,     1,     2,     1,
       2,     1,     4,     1,     4,     3,     0,     1,     3,     2,
       3,     1,     1,     0,     0,     2,     2,     2,     4,     2,
       5,     3,     1,     1,     0,     3,     4,     5,     3,     1,
       2,     0,     4,     1,     3,     2,     2,     2,     1,     1,
       1,     1,     3,     4,     6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   436,     0,   831,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   922,     0,
     910,   713,     0,   719,   720,   721,    22,   778,   898,   164,
     165,   722,     0,   145,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   198,     0,     0,     0,     0,     0,     0,
     403,   404,   405,   408,   407,   406,     0,     0,     0,     0,
     225,     0,     0,     0,   726,   728,   729,   723,   724,     0,
       0,     0,   730,   725,     0,   697,    24,    25,    26,    28,
      27,     0,   727,     0,     0,     0,     0,   731,   409,    29,
      30,    32,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,   530,     0,   163,   135,   902,   714,     0,     0,
       4,   124,   126,   129,   777,     0,   696,     0,     6,   197,
       7,     9,     8,    10,     0,     0,   401,   447,     0,     0,
       0,     0,     0,     0,     0,   445,   886,   887,   516,   515,
     430,   519,     0,     0,   429,   858,   698,   705,     0,   780,
     514,   400,   861,   862,   873,   446,     0,     0,   449,   448,
     859,   860,   857,   893,   897,     0,   504,   779,    11,   408,
     407,   406,     0,     0,    28,     0,   124,   197,     0,   966,
     446,   965,     0,   963,   962,   518,     0,   437,   442,     0,
       0,   487,   488,   489,   490,   513,   511,   510,   509,   508,
     507,   506,   505,   898,   722,   700,     0,     0,   986,   879,
     698,     0,   699,   469,     0,   467,     0,   926,     0,   787,
     428,   709,     0,   986,   708,   703,     0,   718,   699,   905,
     906,   912,   904,   710,     0,     0,   712,   512,     0,     0,
       0,     0,   433,     0,   143,   435,     0,     0,   149,   151,
       0,     0,   153,     0,    82,    81,    76,    75,    67,    68,
      59,    79,    90,     0,    62,     0,    74,    66,    72,    92,
      85,    84,    57,    80,    99,   100,    58,    95,    55,    96,
      56,    97,    54,   101,    89,    93,    98,    86,    87,    61,
      88,    91,    53,    83,    69,   102,    77,    70,    60,    52,
      51,    50,    49,    48,    47,    71,   103,   105,    64,    45,
      46,    73,  1019,  1020,    65,  1025,    44,    63,    94,     0,
       0,   124,   104,   977,  1018,     0,  1021,     0,     0,     0,
     155,     0,     0,     0,     0,   188,     0,     0,     0,     0,
       0,     0,   107,   112,   314,     0,     0,   313,     0,   229,
       0,   226,   319,     0,     0,     0,     0,     0,   983,   213,
     223,   918,   922,     0,   947,     0,   733,     0,     0,     0,
     945,     0,    16,     0,   128,   205,   217,   224,   603,   546,
       0,   971,   528,   532,   534,   835,   436,   447,     0,     0,
     445,   446,   448,     0,     0,   715,     0,   716,     0,     0,
       0,   187,     0,     0,   131,   305,     0,    21,   196,     0,
     222,   209,   221,   406,   409,   197,   402,   178,   179,   180,
     181,   182,   184,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   910,
       0,   177,   901,   901,   185,   932,     0,     0,     0,     0,
       0,     0,     0,     0,   399,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   468,   466,
     836,   837,     0,   901,     0,   849,   305,   305,   901,     0,
     903,   894,   918,     0,   197,     0,     0,   157,     0,   833,
     828,   787,     0,   447,   445,     0,   930,     0,   551,   786,
     921,   447,   445,   446,   131,     0,   305,   427,     0,   851,
     711,     0,   135,   265,     0,   527,     0,   160,     0,     0,
     434,     0,     0,     0,     0,     0,   152,   176,   154,  1019,
    1020,  1016,  1017,     0,  1011,     0,     0,     0,     0,    78,
      43,    65,    42,   978,     0,   183,   156,   186,     0,     0,
       0,     0,     0,     0,     0,   561,     0,   568,   570,   571,
     572,   573,   574,   575,   566,   588,   589,   135,     0,   173,
     175,     0,     0,   109,   116,     0,     0,   111,   120,   113,
       0,    18,     0,     0,   315,     0,   158,   228,   227,     0,
       0,   159,   967,     0,     0,   447,   445,   446,   449,   448,
       0,  1004,   235,     0,   919,     0,     0,   161,     0,     0,
     732,   946,   778,     0,     0,   944,   783,   943,   127,     5,
      13,    14,     0,   233,     0,     0,   539,     0,     0,     0,
     787,     0,     0,   706,   701,   540,     0,     0,     0,     0,
     835,   135,     0,   789,   834,  1029,   426,   439,   501,   867,
     885,   140,   134,   136,   137,   138,   139,   400,     0,   517,
     781,   782,   125,   787,     0,   987,     0,     0,     0,   789,
     306,     0,   522,   199,   231,     0,   472,   474,   473,     0,
       0,   470,   471,   475,   477,   476,   492,   491,   494,   493,
     495,   497,   499,   498,   496,   486,   485,   479,   480,   478,
     481,   482,   484,   500,   483,   900,     0,     0,   936,     0,
     787,   970,     0,   969,   986,   864,   893,   215,   207,   219,
       0,   971,   211,   197,     0,   440,   443,   451,   562,   465,
     464,   463,   462,   461,   460,   459,   458,   457,   456,   455,
     454,   839,     0,   838,   841,   863,   845,   986,   842,     0,
       0,     0,     0,     0,     0,     0,     0,   964,   438,   826,
     830,   786,   832,     0,   702,     0,   925,     0,   924,     0,
     702,   909,   908,     0,     0,   838,   841,   907,   842,   431,
     267,   269,   135,   537,   536,   432,     0,   135,   249,   144,
     435,     0,     0,     0,     0,     0,   261,   261,   150,     0,
       0,     0,     0,  1009,   787,     0,   993,     0,     0,     0,
       0,     0,   785,     0,   697,     0,     0,   129,   735,   696,
     740,     0,   734,   133,   739,   986,  1022,     0,     0,   578,
       0,     0,   584,   581,   582,   590,     0,   569,   564,     0,
     567,     0,     0,     0,   117,    19,     0,     0,   121,    20,
       0,     0,     0,   106,   114,     0,   312,   320,   317,     0,
       0,   956,   961,   958,   957,   960,   959,    12,  1002,  1003,
       0,     0,     0,     0,   918,   915,     0,   550,   955,   954,
     953,     0,   949,     0,   950,   952,     0,     5,     0,     0,
       0,   597,   598,   606,   605,     0,   445,     0,   786,   545,
     549,     0,     0,   972,     0,   529,     0,     0,   994,   835,
     291,  1028,     0,     0,   850,     0,   899,   786,   989,   985,
     307,   308,   695,   788,   304,     0,   835,     0,     0,   233,
     524,   201,   503,     0,   554,   555,     0,   552,   786,   931,
       0,     0,   305,   235,     0,   233,     0,     0,   231,     0,
     910,   452,     0,     0,   847,   848,   865,   866,   895,   896,
       0,     0,     0,   814,   794,   795,   796,   803,     0,     0,
       0,   807,   805,   806,   820,   787,     0,   828,   929,   928,
       0,     0,   852,   717,     0,   271,     0,     0,   141,     0,
       0,     0,     0,     0,     0,     0,   241,   242,   253,     0,
     135,   251,   170,   261,     0,   261,     0,     0,  1023,     0,
       0,     0,   786,  1010,  1012,   992,   787,   991,     0,   787,
     761,   762,   759,   760,   793,     0,   787,   785,     0,   548,
       0,     0,   938,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1015,   563,     0,     0,     0,   586,   587,   585,     0,
       0,   565,     0,   189,     0,   192,   174,     0,   108,   118,
       0,   110,   122,   115,   316,     0,   968,   162,  1004,   984,
     999,   234,   236,   326,     0,     0,   916,     0,   948,     0,
      17,     0,   971,   232,   326,     0,     0,   702,   542,     0,
     707,   973,     0,   994,   535,     0,     0,  1029,     0,   296,
     294,   841,   853,   986,   841,   854,   988,     0,     0,   309,
     132,     0,   835,   230,     0,   835,     0,   502,   935,   934,
       0,   305,     0,     0,     0,     0,     0,     0,   233,   203,
     718,   840,   305,     0,   799,   800,   801,   802,   808,   809,
     818,     0,   787,     0,   814,     0,   798,   822,   786,   825,
     827,   829,     0,   923,   840,     0,     0,     0,     0,   268,
     538,   146,     0,   435,   241,   243,   918,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   255,     0,     0,  1005,
       0,  1008,   786,     0,     0,     0,   737,   786,   784,     0,
     775,     0,   787,     0,   741,   776,   774,   942,     0,   787,
     744,   746,   745,     0,     0,   742,   743,   747,   749,   748,
     764,   763,   766,   765,   767,   769,   771,   770,   768,   757,
     756,   751,   752,   750,   753,   754,   755,   758,  1014,   576,
       0,   577,   583,   591,   592,     0,   135,   119,   123,   318,
       0,     0,     0,  1001,     0,   400,   920,   918,   441,   444,
     450,     0,    15,     0,   400,   609,     0,     0,   611,   604,
     607,     0,   602,     0,   975,     0,   995,   531,     0,   297,
       0,     0,   292,     0,   311,   310,   994,     0,   326,     0,
     835,     0,   305,     0,   891,   326,   971,   326,   974,     0,
       0,     0,   453,     0,     0,   811,   786,   813,   804,     0,
     797,     0,     0,   787,   819,   927,     0,   135,     0,   264,
     250,     0,     0,     0,   240,   166,   254,     0,     0,   257,
       0,   262,   263,   135,   256,  1024,  1006,     0,   990,     0,
    1027,   792,   791,   736,     0,   786,   547,   738,     0,   553,
     786,   937,   773,     0,     0,     0,     0,   998,   996,   997,
     237,     0,     0,     0,   407,   398,     0,     0,     0,   214,
     325,   327,     0,   397,     0,     0,     0,   971,   400,     0,
     951,   322,   218,   600,     0,     0,   541,   533,     0,   300,
     290,     0,   293,   299,   305,   521,   994,   400,   994,     0,
     933,     0,   890,   400,     0,   400,   976,   326,   835,   888,
     817,   816,   810,     0,   812,   786,   821,   135,   270,   142,
     147,   168,   244,     0,   252,   258,   135,   260,  1007,     0,
       0,   544,     0,   941,   940,   772,     0,   135,   193,  1000,
       0,     0,     0,   979,     0,     0,     0,   238,     0,   971,
       0,   363,   359,   365,   697,    28,     0,   353,     0,   358,
     362,   375,     0,   373,   378,     0,   377,     0,   376,     0,
     197,   329,     0,   331,     0,   332,   333,     0,     0,   917,
       0,   601,   599,   610,   608,   301,     0,     0,   288,   298,
       0,     0,     0,     0,   210,   521,   994,   892,   216,   322,
     220,   400,     0,     0,   824,     0,   266,     0,     0,   135,
     247,   167,   259,  1026,   790,     0,     0,     0,     0,     0,
       0,   425,     0,   980,     0,   343,   347,   422,   423,   357,
       0,     0,     0,   338,   661,   660,   657,   659,   658,   678,
     680,   679,   649,   620,   621,   639,   655,   654,   616,   626,
     627,   629,   628,   648,   632,   630,   631,   633,   634,   635,
     636,   637,   638,   640,   641,   642,   643,   644,   645,   647,
     646,   617,   618,   619,   622,   623,   625,   663,   664,   673,
     672,   671,   670,   669,   668,   656,   675,   665,   666,   667,
     650,   651,   652,   653,   676,   677,   681,   683,   682,   684,
     685,   662,   687,   686,   689,   691,   690,   624,   694,   692,
     693,   688,   674,   615,   370,   612,     0,   339,   391,   392,
     390,   383,     0,   384,   340,   417,     0,     0,     0,     0,
     421,     0,   197,   206,   321,     0,     0,     0,   289,   303,
     889,     0,   135,   393,   135,   200,     0,     0,     0,   212,
     994,   815,     0,   135,   245,   148,   169,     0,   543,   939,
     579,   191,   341,   342,   420,   239,     0,     0,   787,     0,
     366,   354,     0,     0,     0,   372,   374,     0,     0,   379,
     386,   387,   385,     0,     0,   328,   981,     0,     0,     0,
     424,     0,   323,     0,   302,     0,   595,   789,     0,     0,
     135,   202,   208,     0,   823,     0,     0,     0,   171,   344,
     124,     0,   345,   346,     0,     0,   360,   786,   368,   364,
     369,   613,   614,     0,   355,   388,   389,   381,   382,   380,
     418,   415,  1004,   334,   330,   419,     0,   324,   596,   788,
       0,   523,   394,     0,   204,     0,   248,   580,     0,   195,
       0,   400,     0,   367,   371,     0,     0,   835,   336,     0,
     593,   520,   525,   246,     0,     0,   172,   351,     0,   399,
     361,   416,   982,     0,   789,   411,   835,   594,     0,   194,
       0,     0,   350,   994,   835,   275,   412,   413,   414,  1029,
     410,     0,     0,     0,   349,     0,   411,     0,   994,     0,
     348,   395,   135,   335,  1029,     0,   280,   278,     0,   135,
       0,     0,   281,     0,     0,   276,   337,     0,   396,     0,
     284,   274,     0,   277,   283,   190,   285,     0,     0,   272,
     282,     0,   273,   287,   286
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   907,   639,   186,  1473,   734,
     351,   592,   596,   352,   593,   597,   122,   123,   124,   125,
     126,   127,   403,   672,   673,   539,   253,  1537,   545,  1453,
    1538,  1779,   863,   346,   587,  1738,  1096,  1276,  1796,   419,
     187,   674,   947,  1156,  1331,   131,   642,   964,   675,   694,
     968,   622,   963,   676,   643,   965,   421,   369,   386,   134,
     949,   910,   893,  1111,  1476,  1208,  1016,  1685,  1541,   809,
    1022,   544,   818,  1024,  1363,   801,  1005,  1008,  1197,  1803,
    1804,   662,   663,   688,   689,   356,   357,   363,  1510,  1664,
    1665,  1285,  1400,  1499,  1658,  1787,  1806,  1696,  1742,  1743,
    1744,  1486,  1487,  1488,  1489,  1698,  1699,  1705,  1754,  1492,
    1493,  1497,  1651,  1652,  1653,  1675,  1833,  1401,  1402,   188,
     136,  1819,  1820,  1656,  1404,  1405,  1406,  1407,   137,   246,
     540,   541,   138,   139,   140,   141,   142,   143,   144,   145,
    1522,   146,   946,  1155,   147,   250,   659,   395,   660,   661,
     535,   649,   650,  1232,   651,  1233,   148,   149,   150,   840,
     151,   152,   343,   153,   344,   575,   576,   577,   578,   579,
     580,   581,   582,   583,   853,   854,  1088,   584,   585,   586,
     860,  1727,   154,   644,  1512,   645,  1125,   915,  1302,  1299,
    1644,  1645,   155,   156,   157,   236,   158,   237,   247,   406,
     527,   159,  1044,   844,   160,  1045,   938,   930,  1046,   992,
    1178,   993,  1180,  1181,  1182,   995,  1342,  1343,   996,   780,
     511,   199,   200,   677,   665,   492,  1141,  1142,   766,   767,
     934,   162,   239,   163,   164,   190,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   726,   175,   243,   244,   625,
     228,   229,   729,   730,  1238,  1239,   379,   380,   901,   176,
     613,   177,   658,   178,   335,  1666,  1717,   370,   414,   683,
     684,  1038,  1136,  1283,   890,   891,   823,   824,   825,   336,
     337,   846,  1475,   932
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1534
static const yytype_int16 yypact[] =
{
   -1534,   123, -1534, -1534,  5493, 13773, 13773,   -38, 13773, 13773,
   13773, 11496, 13773, -1534, 13773, 13773, 13773, 13773, 13773, 13773,
   13773, 13773, 13773, 13773, 13773, 13773, 17689, 17689, 11703, 13773,
   17815,   185,   193, -1534, -1534, -1534, -1534, -1534,   392, -1534,
   -1534,   171, 13773, -1534,   193,   195,   216,   264, -1534,   193,
   11910, 14915, 12117, -1534, 14863, 10461,   263, 13773, 17754,    61,
   -1534, -1534, -1534,    58,    60,     5,   316,   327,   330,   345,
   -1534, 14915,   349,   351, -1534, -1534, -1534, -1534, -1534, 13773,
     461,  3715, -1534, -1534, 14915, -1534, -1534, -1534, -1534, 14915,
   -1534, 14915, -1534,   398,   360, 14915, 14915, -1534,   155, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, 12324, -1534, -1534,   367,   537,   569,   569,
   -1534,   137,   435,   497, -1534,   393, -1534,    91, -1534,   565,
   -1534, -1534, -1534, -1534, 18407,   636, -1534, -1534,   414,   430,
     439,   441,   443,   445,  4674, -1534, -1534, -1534, -1534,    94,
   -1534,   585,   589,   459, -1534,    28,   482, -1534,   517,    -9,
   -1534,   751,   132, -1534, -1534,  3233,   107,   498,   170, -1534,
     144,    65,   516,    83, -1534,    84, -1534,   665, -1534, -1534,
   -1534,   584,   550,   592, -1534, 13773, -1534,   565,   636, 18753,
    4906, 18753, 13773, 18753, 18753, 15252,   567, 16692, 15252,   717,
   14915,   722,   722,   520,   722,   722,   722,   722,   722,   722,
     722,   722,   722, -1534, -1534, -1534,    54, 13773,   621, -1534,
   -1534,   644,   604,   247,   606,   247, 17689, 16973,   605,   805,
   -1534,   584, 13773,   621,   664, -1534,   676,   616, -1534,   146,
   -1534, -1534, -1534,   247,   107, 12531, -1534, -1534, 13773,  9012,
     822,    93, 18753, 10047, -1534, 13773, 13773, 14915, -1534, -1534,
    4731,   638, -1534,  4804, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, 16473, -1534, 16473, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534,    97,    79,   592, -1534, -1534, -1534, -1534,   643,
   16259,    85, -1534, -1534,   687,   831, -1534,   696, 15645,   768,
   -1534,   650,  4849,   651,   779, -1534,    55,  5023, 18420, 18486,
   14915,    98, -1534,    99, -1534, 17164,   104, -1534,   728, -1534,
     729, -1534,   842,   105, 17689, 13773, 13773,   656,   686, -1534,
   -1534, 17295, 11703,   106,   465,   389, -1534, 13980, 17689,   527,
   -1534, 14915, -1534,   201,   435, -1534, -1534, -1534, -1534, 17941,
     846,   763, -1534, -1534, -1534,    64, 13773,   661,   663, 18753,
     672,  2200,   673,  5700, 13773,   262,   659,   629,   262,   442,
     472, -1534, 14915, 16473,   680, 10668, 14863, -1534, -1534, 14525,
   -1534, -1534, -1534, -1534, -1534,   565, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, 13773, 13773, 13773, 12738, 13773, 13773, 13773,
   13773, 13773, 13773, 13773, 13773, 13773, 13773, 13773, 13773, 13773,
   13773, 13773, 13773, 13773, 13773, 13773, 13773, 13773, 13773, 17815,
   13773, -1534, 13773, 13773, -1534, 13773, 14187, 14915, 14915, 14915,
   14915, 14915, 18407,   769,   706, 10254, 13773, 13773, 13773, 13773,
   13773, 13773, 13773, 13773, 13773, 13773, 13773, 13773, -1534, -1534,
   -1534, -1534,  2450, 13773, 13773, -1534, 10668, 10668, 13773, 13773,
     367,   150, 17295,   681,   565, 12945,  4976, -1534, 13773, -1534,
     682,   875,   737,   688,   689, 14330,   247, 13152, -1534, 13359,
   -1534,   700,   703,  2396, -1534,   134, 10668, -1534,  3499, -1534,
   -1534, 16099, -1534, -1534, 10875, -1534, 13773, -1534,   807,  9219,
     898,   707, 18708,   894,    82,    50, -1534, -1534, -1534,   738,
   -1534, -1534, -1534, 16473,  2965,   711,   908, 16904, 14915, -1534,
   -1534, -1534, -1534, -1534,   733, -1534, -1534, -1534,   841, 13773,
     845,   847, 13773, 13773, 13773, -1534,   779, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534,   740, -1534, -1534, -1534,   726, -1534,
   -1534, 14915,   720,   925,   304, 14915,   734,   926,   329,   387,
   18539, -1534, 14915, 13773,   247,    61, -1534, -1534, -1534, 16904,
     859, -1534,   247,    89,    95,   743,   744,  2694,   188,   749,
     739,   477,   827,   756,   247,   100,   759, -1534, 18366, 14915,
   -1534, -1534,   899,  2452,    38, -1534, -1534, -1534,   435, -1534,
   -1534, -1534,   931,   838,   800,   147,   821, 13773,   367,   843,
     968,   777,   828, -1534,   150, -1534, 16473, 16473,   966,   822,
      64, -1534,   783,   975, -1534, 16473,    22, -1534,   473,   143,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534,   776,  2639, -1534,
   -1534, -1534, -1534,   978,   815, -1534, 17689, 13773,   792,   985,
   18753,   982, -1534, -1534,   869, 14720, 12102, 15642, 15252, 13773,
   13965, 19025,  4500,  3899, 11061, 12923, 13130, 13130, 13130, 13130,
    1392,  1392,  1392,  1392,  1392,  1184,  1184,   785,   785,   785,
     520,   520,   520, -1534,   722, 18753,   794,   795, 17103,   799,
     995,   -27, 13773,   -12,   621,   222,   150, -1534, -1534, -1534,
     991,   763, -1534,   565, 17427, -1534, -1534, 15252, -1534, 15252,
   15252, 15252, 15252, 15252, 15252, 15252, 15252, 15252, 15252, 15252,
   15252, -1534, 13773,   209,   156, -1534, -1534,   621,   224,   804,
    2754,   812,   813,   809,  3284,   121,   806, -1534, 18753, 17034,
   -1534, 14915, -1534,    22,    52, 17689, 18753, 17689, 18128,    22,
     247,   159,   866,   817, 13773, -1534,   162, -1534, -1534, -1534,
    8805,   447, -1534, -1534, 18753, 18753,   193, -1534, -1534, -1534,
   13773,   921, 16753, 16904, 14915,  9426,   819,   825, -1534,    68,
     936,   892,   877, -1534,  1022,   835, 16332, 16473, 16904, 16904,
   16904, 16904, 16904,   844,   887,   848, 16904,   411, -1534,   890,
   -1534,   839, -1534, 18841, -1534,   243, -1534, 13773,   863, 18753,
     865,  1031, 11481,  1045, -1534, 18753, 16187, -1534,   740,   976,
   -1534,  5907, 17880,   852,   446, -1534, 18420, 14915,   463, -1534,
   18486, 14915, 14915, -1534, -1534,  3390, -1534, 18841,  1046, 17689,
     860, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
      72, 14915, 17880,   858, 17295, 17558,  1049, -1534, -1534, -1534,
   -1534,   857, -1534, 13773, -1534, -1534,  5079, -1534, 16473, 17880,
     864, -1534, -1534, -1534, -1534,  1052,   868, 13773, 17941, -1534,
   -1534, 14187,   874, -1534, 16473, -1534,   883,  6114,  1041,   135,
   -1534, -1534,    96,  2450, -1534,  3499, -1534, 16473, -1534, -1534,
     247, 18753, -1534, 11082, -1534, 16904,   108,   882, 17880,   838,
   -1534, -1534, 18954, 13773, -1534, -1534, 13773, -1534, 13773, -1534,
    3716,   886, 10668,   827,  1053,   838, 16473,  1061,   869, 14915,
   17815,   247,  4001,   891, -1534, -1534,   151,   893, -1534, -1534,
    1073, 17738, 17738, 17034, -1534, -1534, -1534,  1043,   900,    73,
     905, -1534, -1534, -1534, -1534,  1081,   897,   682,   247,   247,
   13566,  3499, -1534, -1534,  4184,   547,   193, 10047, -1534,  6321,
     904,  6528,   906, 16753, 17689,   907,   962,   247, 18841,  1095,
   -1534, -1534, -1534, -1534,   577, -1534,   295, 16473, -1534,   979,
   16473, 14915,  2965, -1534, -1534, -1534,  1101, -1534,   912,   978,
     639,   639,  1050,  1050, 18230,   913,  1106, 16904, 15931, 17941,
   16144, 15788, 16904, 16904, 16904, 16904, 16623, 16904, 16904, 16904,
   16904, 16904, 16904, 16904, 16904, 16904, 16904, 16904, 16904, 16904,
   16904, 16904, 16904, 16904, 16904, 16904, 16904, 16904, 16904, 16904,
   14915, -1534, 18753, 13773, 13773, 13773, -1534, -1534, -1534, 13773,
   13773, -1534,   779, -1534,  1039, -1534, -1534, 14915, -1534, -1534,
   14915, -1534, -1534, -1534, -1534, 16904,   247, -1534,   477, -1534,
     560,  1112, -1534, -1534,   128,   922,   247, 11289, -1534,  2326,
   -1534,  5286,   763,  1112, -1534,   402,   -11, -1534, 18753,   994,
     927, -1534,   928,  1041, -1534, 16473,   822, 16473,    67,  1114,
    1054,   163, -1534,   621,   166, -1534, -1534, 17689, 13773, 18753,
   18841,   932,   108, -1534,   929,   108,   935, 18954, 18753, 18173,
     941, 10668,   945,   933, 16473,   950,   946, 16473,   838, -1534,
     616,   335, 10668, 13773, -1534, -1534, -1534, -1534, -1534, -1534,
    1023,   947,  1149,  1074, 17034,  1014, -1534, 17941, 17034, -1534,
   -1534, -1534, 17689, 18753, -1534,   193,  1133,  1090, 10047, -1534,
   -1534, -1534,   963, 13773,   962,   247, 17295, 16753,   965, 16904,
    6735,   708,   971, 13773,    56,   311, -1534,   997, 16473, -1534,
    1042, -1534, 16400,  1148,   980, 16904, -1534, 16904, -1534,   981,
   -1534,  1051,  1175,   984, -1534, -1534, -1534, 18275,   983,  1177,
   14184, 18882, 18918, 16904, 18798, 19060, 17494, 10648, 11682, 13337,
   13544, 13544, 13544, 13544,  1729,  1729,  1729,  1729,  1729,   735,
     735,   639,   639,   639,  1050,  1050,  1050,  1050, -1534, 18753,
   13758, 18753, -1534, 18753, -1534,   987, -1534, -1534, -1534, 18841,
   14915, 16473, 16473, -1534, 17880,    88, -1534, 17295, -1534, -1534,
   15252,   986, -1534,   988,  1167, -1534,   142, 13773, -1534, -1534,
   -1534, 13773, -1534, 13773, -1534,   822, -1534, -1534,   180,  1178,
    1113, 13773, -1534,   996,   247, 18753,  1041,   993, -1534,  1000,
     108, 13773, 10668,  1002, -1534, -1534,   763, -1534, -1534,  1003,
     992,  1004, -1534,  1005, 17034, -1534, 17034, -1534, -1534,  1006,
   -1534,  1077,  1009,  1202, -1534,   247,  1182, -1534,  1012, -1534,
   -1534,  1016,  1017,   129, -1534, -1534, 18841,  1013,  1018, -1534,
    4602, -1534, -1534, -1534, -1534, -1534, -1534, 16473, -1534, 16473,
   -1534, 18841, 18332, -1534, 16904, 17941, -1534, -1534, 16904, -1534,
   16904, -1534, 18990, 16904, 13773,  1020,  6942,   560, -1534, -1534,
   -1534,   601, 15058, 17880,  1117, -1534,  2829,  1066, 15307, -1534,
   -1534, -1534,   769, 16213,   109,   110,  1019,   763,   706,   130,
   -1534, -1534, -1534,  1071,  4328,  4557, 18753, -1534,    71,  1215,
    1150, 13773, -1534, 18753, 10668,  1126,  1041,  1641,  1041,  1057,
   18753,  1058, -1534,  1909,  1037,  1928, -1534, -1534,   108, -1534,
   -1534,  1110, -1534, 17034, -1534, 17941, -1534, -1534,  8805, -1534,
   -1534, -1534, -1534,  9633, -1534, -1534, -1534,  8805, -1534,  1059,
   16904, 18841,  1111, 18841, 18377, 18990, 12516, -1534, -1534, -1534,
   17880, 17880, 14915, -1534,  1232, 15931,    70, -1534, 15058,   763,
   18352, -1534,  1075, -1534,   111,  1060,   113, -1534, 15451, -1534,
   -1534, -1534,   115, -1534, -1534, 17087, -1534,  1062, -1534,  1174,
     565, -1534, 15253, -1534, 15253, -1534, -1534,  1241,   769, -1534,
   14473, -1534, -1534, -1534, -1534,  1247,  1186, 13773, -1534, 18753,
    1070,  1067,  1069,   535, -1534,  1126,  1041, -1534, -1534, -1534,
   -1534,  1977,  1072, 17034, -1534,  1142,  8805,  9840,  9633, -1534,
   -1534, -1534,  8805, -1534, 18841, 16904, 16904, 13773,  7149,  1079,
    1088, -1534, 16904, -1534, 17880, -1534, -1534, -1534, -1534, -1534,
   16473,   627,  2829, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534,   141, -1534,  1066, -1534, -1534, -1534,
   -1534, -1534,   103,   670, -1534,  1262,   117, 15645,  1174,  1263,
   -1534, 16473,   565, -1534, -1534,  1089,  1265, 13773, -1534, 18753,
   -1534,   114, -1534, -1534, -1534, -1534,  1091,   535, 14668, -1534,
    1041, -1534, 17034, -1534, -1534, -1534, -1534,  7356, 18841, 18841,
   11895, -1534, -1534, -1534, 18841, -1534,  3014,    75,  1286,  1093,
   -1534, -1534, 16904, 15451, 15451,  1244, -1534, 17087, 17087,   684,
   -1534, -1534, -1534, 16904,  1221, -1534,  1130,  1103,   118, 16904,
   -1534, 14915, -1534, 16904, 18753,  1224, -1534,  1296,  7563,  7770,
   -1534, -1534, -1534,   535, -1534,  7977,  1105,  1228,  1198, -1534,
    1212,  1160, -1534, -1534,  1216, 16473, -1534,   627, -1534, -1534,
   18841, -1534, -1534,  1152, -1534,  1280, -1534, -1534, -1534, -1534,
   18841,  1304,   477, -1534, -1534, 18841,  1119, 18841, -1534,   340,
    1120, -1534, -1534,  8184, -1534,  1118, -1534, -1534,  1122,  1158,
   14915,   706,  1157, -1534, -1534, 16904,   112,   124, -1534,  1251,
   -1534, -1534, -1534, -1534, 17880,   852, -1534,  1166, 14915,   510,
   -1534, 18841, -1534,  1132,  1325,   625,   124, -1534,  1255, -1534,
   17880,  1134, -1534,  1041,   133, -1534, -1534, -1534, -1534, 16473,
   -1534,  1138,  1139,   119, -1534,   542,   625,   302,  1041,  1140,
   -1534, -1534, -1534, -1534, 16473,   258,  1328,  1267,   542, -1534,
    8391,   339,  1331,  1268, 13773, -1534, -1534,  8598, -1534,   261,
    1336,  1271, 13773, -1534, 18753, -1534,  1340,  1275, 13773, -1534,
   18753, 13773, -1534, 18753, 18753
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1534, -1534, -1534,  -579, -1534, -1534, -1534,   168,     2,   -33,
   -1534, -1534, -1534,   757,   492,   489,   207,  1377,  3035, -1534,
    2454, -1534,   152, -1534,    26, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534, -1534,  -434, -1534, -1534,  -154,
     191,    18, -1534, -1534, -1534, -1534, -1534, -1534,    31, -1534,
   -1534, -1534, -1534,    36, -1534, -1534,   895,   901,   896,   -95,
     394,  -890,   401,   460,  -440,   169,  -930, -1534,  -166, -1534,
   -1534, -1534, -1534,  -741,    14, -1534, -1534, -1534, -1534,  -430,
   -1534,  -619, -1534,  -439, -1534, -1534,   772, -1534,  -149, -1534,
   -1534, -1051, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534, -1534,  -181, -1534,   -98, -1534, -1534, -1534, -1534, -1534,
    -262, -1534,   -13,  -949, -1534, -1533,  -451, -1534,  -159,    59,
    -110,  -437, -1534,  -270, -1534, -1534, -1534,    -4,   -42,    -6,
      11,  -742,   -74, -1534, -1534,   -21, -1534, -1534,    -5,   -69,
    -134, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
    -610,  -864, -1534, -1534, -1534, -1534, -1534,  1456, -1534, -1534,
   -1534, -1534,   917, -1534, -1534,   301, -1534,   818, -1534, -1534,
   -1534, -1534, -1534, -1534, -1534,   307, -1534,   823, -1534, -1534,
     539, -1534,   273, -1534, -1534, -1534, -1534, -1534, -1534, -1534,
   -1534,  -928, -1534,  2167,     6, -1534,   873,  -404, -1534, -1534,
     232,  3244,  3451, -1534, -1534,   353,  -167,  -681, -1534, -1534,
     423,   223,  -683,   225, -1534, -1534, -1534, -1534, -1534,   413,
   -1534, -1534, -1534,   101,  -900,  -118,  -427,  -425, -1534,   478,
    -115, -1534, -1534,    39,    42,   131, -1534, -1534,  1098,   -52,
   -1534,  -343,    87,   196, -1534,   230, -1534, -1534, -1534,  -438,
    1040, -1534, -1534, -1534, -1534, -1534,   654,   424, -1534, -1534,
   -1534,  -341,  -659, -1534,   998,  -905, -1534,   -64,  -191,   -36,
     587, -1534, -1036,    30,  -347,   312,   390, -1534, -1534, -1534,
   -1534,   343,   333, -1109
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1014
static const yytype_int16 yytable[] =
{
     189,   191,   473,   193,   194,   195,   197,   198,   944,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   333,   130,   227,   230,   426,   249,   387,  1308,  1137,
     128,   390,   391,   503,   341,   132,   235,   252,   254,   398,
     133,   926,   524,   258,   400,   260,   653,   263,   655,   925,
     342,   495,   347,   251,  1129,   723,   332,   771,   772,  1154,
     906,   402,   520,   135,   775,   764,   472,   765,  1012,   240,
     422,   397,   241,  1294,   252,  1165,  1026,  1027,   426,  1554,
     816,  1108,   967,  1204,  1745,   362,  1361,   793,   -43,   493,
     373,   814,  1391,   -43,   -42,  -868,   994,  1305,   879,   -42,
     416,   796,   536,   797,   536,   161,   -78,   600,   399,   895,
      13,   -78,  1707,   605,   610,   536,   400,   242,  1502,  1504,
    -356,  1108,  1562,     3,  1646,   528,  1714,  1714,  1554,   529,
     895,  1138,  -870,   402,    13,   165,  1708,   895,   895,   895,
     512,   354,  -699,   397,  1731,  -105,  1300,  1309,   514,  1725,
     498,  1515,   490,   491,    13,  1702,  -700,   223,   225,  1185,
    -104,  -556,   192,   522,   213,   402,   588,   411,   490,   491,
      13,  1703,   121,  -105,   493,   513,  1139,   358,   360,    13,
     399,    13,   490,   491,   359,  1231,   361,   506,  -104,  1704,
     521,  -880,  1301,  1413,  1726,   129,  1392,   602,   912,  -871,
    1774,  1393,   376,    60,    61,    62,   179,  1394,   423,  1395,
    -875,  -869,   399,  -911,   213,  1418,  -706,   498,  -874,   261,
     474,  1186,   331,  -872,   589,   494,  -914,   399,  -878,  -913,
    -855,  -868,  1310,  -856,  -701,   392,  1516,  -558,  -559,   368,
     531,  1028,   905,   531,   401,  1109,  1396,  1397,  1746,  1398,
     252,   542,  -882,   817,   412,  -558,   629,   385,  1362,   368,
    1419,  1140,  -295,   368,   368,   353,   355,  1427,  -870,   553,
     424,   695,  1555,  1556,  1433,   533,  1435,  1354,  1330,   538,
    1425,   -43,  1211,   815,  1215,  1802,   499,   -42,   383,   500,
     880,   384,  1399,   417,  -879,   537,   881,   616,   388,   -78,
     601,   896,   368,   913,  1709,   563,   606,   611,   627,  -295,
     494,  1503,  1505,  -356,   619,  1563,   401,  1647,   914,  1715,
    1764,  1830,   980,  1341,   615,  -279,   425,  1151,  1121,  1286,
    1452,  1509,  -877,  -788,  -788,  -871,  -788,  1835,  1842,   116,
     332,  1856,   374,  -881,   782,  1420,  -875,  -869,   401,  -911,
     776,  -884,   393,   499,  -874,   693,   412,   516,   394,  -872,
     252,   399,  -914,   523,   426,  -913,  -855,   227,   510,  -856,
     497,   501,   633,  -707,  1849,  1789,   614,   490,   491,   504,
     165,  -843,  1836,   333,   165,   245,  1531,   338,   497,   885,
    1523,   197,  1525,   248,  1080,   255,  -846,  1213,  1214,   678,
    -526,   745,   867,   640,   641,   387,   740,   741,   422,  -843,
     690,   377,   378,  1213,  1214,   413,   256,   121,   332,  1850,
    1790,   121,   962,  1843,  -846,   543,  1857,   871,   696,   697,
     698,   700,   701,   702,   703,   704,   705,   706,   707,   708,
     709,   710,   711,   712,   713,   714,   715,   716,   717,   718,
     719,   720,   721,   722,   746,   724,  1114,   725,   725,   412,
     728,  1351,   135,  1293,   257,   235,   345,  1837,   733,   374,
     747,   749,   750,   751,   752,   753,   754,   755,   756,   757,
     758,   759,   760,   919,   412,   872,   604,  1474,   725,   770,
    1677,   690,   690,   725,   774,   612,   664,   617,   240,  1216,
     747,   241,   624,   778,  1851,  1344,   562,  -844,  1144,   634,
    1145,  1462,   786,   803,   788,  1364,   364,   332,   473,  1006,
    1007,   690,   374,  1162,   888,   889,  1307,   365,   635,   804,
     366,   805,   628,  1317,   165,  -844,  1319,   630,   377,   378,
    -560,   374,   412,   961,  1097,   367,   242,   375,   933,   371,
     935,   372,   374,   735,   388,   594,   598,   599,   635,  1295,
     389,  1100,  -986,   959,   849,   808,  1170,   852,   855,   856,
     404,   121,  1296,  1557,   459,   653,   973,   655,   916,   768,
     682,  1535,   472,   413,   331,   654,   460,   368,   638,   969,
     412,   377,   378,   415,   129,   490,   491,  1659,   875,  1660,
     418,   412,   735,   490,   491,  1297,   551,   374,   552,   376,
     377,   378,   792,   635,  -986,   798,   427,   374,   412,  1195,
    1196,   377,   378,   405,  -702,    60,    61,    62,   179,   180,
     423,   951,   428,   624,   562,   368,   738,   368,   368,   368,
     368,   429,   399,   430,  1733,   431,   524,   432,  -986,   374,
     680,  1440,   462,  1441,  1081,   408,   463,  1033,  1281,  1282,
     763,   464,   736,   556,   743,   933,   935,  1434,   466,   413,
     165,  1001,   935,  -882,  1002,   636,   377,   378,  1212,  1213,
    1214,   681,   941,   562,   800,   465,   377,   378,   736,  1076,
    1077,  1078,   424,   727,   952,  1417,   795,  -986,   496,  1710,
    -986,  1429,    33,    34,    35,  1079,   653,   121,   655,   374,
    1827,   736,  1812,  1757,   214,   635,  -876,  1711,   377,   378,
    1712,   736,  1323,   769,   736,  1841,   845,   960,   773,  1470,
    1471,  1758,  -557,  1333,  1759,  -700,   474,  1673,  1674,   861,
    1816,  1817,  1818,    53,  1831,  1832,   685,   381,  1507,   338,
     502,    60,    61,    62,   179,   180,   423,   972,  1755,  1756,
    1534,   664,   509,    74,    75,    76,    77,    78,  1353,   507,
     874,   407,   409,   410,   216,  1751,  1752,  1825,   377,   378,
      82,    83,  1073,  1074,  1075,  1076,  1077,  1078,   460,  1004,
    1036,  1039,  1838,   413,    92,   515,   900,   902,   864,   631,
    1010,  1079,   868,   637,  -880,   252,   497,   353,    97,  1358,
    1213,  1214,   518,   927,   519,  -698,   526,   940,   424,  1532,
    1558,    60,    61,    62,   179,   180,   423,   525,  1189,   631,
     534,   637,   631,   637,   637,   456,   457,   458, -1013,   459,
     547,  1021,  1082,   554,   653,   557,   655,   558,   564,  1409,
    1681,   460,   565,   567,   607,   608,   609,   620,   621,   135,
     656,   657,   666,   368,   667,   679,    60,    61,    62,    63,
      64,   423,  1224,   668,   670,   971,    53,    70,   467,  1228,
    -130,   692,   779,  1431,   781,   630,   819,  1805,   424,   783,
     784,    60,    61,    62,    63,    64,   423,   806,  1119,   221,
     221,   789,    70,   467,   790,  1169,  1805,   536,   813,   810,
     553,   826,  1128,   468,  1826,   469,   998,   827,   999,   847,
     135,   848,   865,   733,   130,   850,   862,   851,   470,   859,
     471,   165,   128,   424,   866,   870,   869,   132,  1149,   878,
     469,   887,   133,  1017,   882,   883,   165,   991,  1157,   997,
     886,  1158,  1313,  1159,  1009,   892,   894,   690,   424,  1011,
     897,   908,   568,   569,   570,   135,   903,   909,   121,   571,
     572,   911,  -722,   573,   574,   917,   235,   918,   920,   921,
     924,   928,  1019,   121,   929,  1520,   135,   937,   939,   922,
     923,   129,   165,   942,   943,  1193,   945,   948,   931,  1734,
    1198,   954,   955,   957,   958,   966,   950,   161,  1130,   240,
    1106,   974,   241,   976,   977,  1337,   978,  -704,  1003,  1013,
     768,  1023,   798,  1029,  1030,   624,  1116,  1025,  1031,   121,
     664,  1032,   653,  1199,   655,  1099,  1034,   165,  1048,  1102,
    1103,  1051,  1052,  1288,  1047,  1085,  1770,   664,  1049,  1083,
    1230,  1084,   129,  1236,  1089,  1095,  1092,   242,   165,  1110,
    1105,  1113,  1107,  1117,  1118,  1376,  1126,  1124,   135,  1127,
     135,  1135,  1381,   594,   121,  1167,  1131,   598,  1269,  1270,
    1271,  1133,  1152,  1164,   852,  1273,  1161,  1173,   798,   562,
    1188,  1172,  1183,  -883,  1207,   121,  1289,   129,  1190,   221,
    1184,   763,   653,   795,   655,  1187,  1201,  1206,  1203,  1209,
    1222,  1218,  1290,  1223,   654,  1227,  1079,   736,   129,  1275,
    1226,  1284,  1287,  1815,   224,   224,  1303,   962,  1311,   736,
    1304,   736,  1318,  1316,  1312,  1320,  1325,   368,   165,   130,
     165,  1322,   165,  1315,  1017,  1205,  1324,   128,  1328,  1177,
    1177,   991,   132,  1327,  1335,  1334,   690,   133,  1336,   685,
     685,   987,  1340,  1347,  1348,  1350,  1355,   690,  1290,   795,
    1365,  1391,  1210,  1359,  1367,   121,  1446,   121,  1369,   121,
     135,  1370,  1373,  1374,  1375,  1377,  1380,  1379,  1385,  1346,
    1410,  1411,  1421,  1422,  1426,  1437,  1424,   736,   252,  1220,
     129,  1428,   129,  1432,  1438,  1436,  1439,  1442,  1360,  1443,
    1444,  1445,  1447,    13,  1449,  1454,   562,  1450,  1451,   562,
    1455,  1506,   161,  1467,  1349,  1478,  1491,  1511,   221,  1517,
    1518,   453,   454,   455,   456,   457,   458,   221,   459,  1521,
    1529,  1122,  1533,  1545,   221,   654,  1552,  1560,   845,  1508,
     460,   221,   165,   664,  1655,  1661,   664,  1132,  1526,  1527,
    1543,  1667,   652,  1561,  1654,  1277,  1668,  1671,  1278,   135,
    1146,  1670,  1672,  1680,  1682,  1392,  1713,  1719,  1314,  1723,
    1393,  1692,    60,    61,    62,   179,  1394,   423,  1395,   121,
    1693,  1722,  1414,   426,  1730,  1747,  1415,  1749,  1416,  1166,
    1753,  1761,  1762,  1763,  1768,  1769,  1423,  1776,  1777,  1778,
    -352,  1780,   129,  1708,  1781,  1784,  1430,   690,  1785,  1788,
    1793,  1791,  1794,  1345,   224,  1396,  1397,  1795,  1398,   165,
    1800,  1807,  1810,  1813,  1814,  1822,  1824,   624,  1017,  1828,
    1829,   165,  1844,  1839,  1403,  1852,  1657,  1845,  1853,   424,
    1858,  1859,   991,  1403,  1861,  1862,   991,   873,  1098,  1101,
    1217,  1809,  1168,  1219,  1163,   739,   121,   742,   737,  1123,
    1823,  1412,  1686,  1352,  1456,   221,  1821,   876,   121,  1466,
    1678,  1701,  1559,   654,  1706,  1498,  1408,  1846,  1718,  1834,
    1479,  1676,   748,  1274,   857,  1408,  1272,  1091,  1298,   858,
    1229,   129,  1332,   218,   218,  1179,  1338,   233,  1551,  1339,
    1191,  1143,   626,  1037,   691,  1786,  1519,  1469,   624,   690,
    1280,   664,  1221,  1268,     0,     0,     0,     0,  1386,     0,
       0,   233, -1014, -1014, -1014, -1014, -1014,   451,   452,   453,
     454,   455,   456,   457,   458,   135,   459,     0,  1387,     0,
       0,     0,     0,   224,     0,     0,     0,     0,   460,     0,
       0,     0,   224,     0,   618,     0,     0,   474,  1306,   224,
     931,     0,     0,     0,     0,     0,   224,  1553,     0,  1540,
       0,     0,   219,   219,     0,     0,  1403,     0,     0,     0,
       0,     0,  1403,     0,  1403,     0,     0,  1326,     0,  1448,
    1329,     0,   991,     0,   991,     0,     0,   135,  1721,     0,
       0,     0,  1669,     0,     0,  1457,   135,   165,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1408,     0,
       0,  1748,     0,     0,  1408,     0,  1408,     0,     0,   664,
       0,     0,  1690,     0,     0,     0,     0,     0,     0,     0,
       0,  1366,     0,     0,   121,  1146,     0,     0,     0,   221,
     331,     0,     0,  1684,  1540,     0,  1496,     0,     0,     0,
       0,   654,     0,     0,     0,     0,     0,   129,     0,   165,
       0,     0,     0,     0,   165,     0,     0,     0,   165,     0,
    1403,     0,     0,  1500,     0,   135,     0,     0,     0,  1536,
     224,   135,     0,   218,     0,     0,     0,   135,  1542,     0,
       0,   991,     0,     0,  1388,  1389,   121,   221,     0,  1548,
       0,   121,  1798,     0,  1716,   121,     0,     0,     0,     0,
       0,     0,  1408,     0,     0,     0,     0,     0,     0,   129,
     368,   654,     0,   562,     0,  1391,   331,     0,   129,     0,
     233,     0,   233,     0,     0,     0,  1643,  1766,   221,   332,
     221,     0,  1724,  1650,     0,     0,     0,   165,   165,   165,
     331,     0,   331,   165,     0,     0,     0,     0,   331,   165,
       0,     0,   219,     0,     0,   221,     0,    13,     0,   426,
       0,  1687,     0,     0,     0,     0,     0,     0,     0,  1662,
    1458,   991,  1459,     0,   121,   121,   121,   233,     0,     0,
     121,     0,     0,     0,     0,     0,   121,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   129,     0,     0,
       0,     0,   218,   129,     0,     0,  1501,     0,     0,   129,
       0,   218,     0,     0,     0,     0,   135,     0,   218,  1392,
       0,     0,   221,     0,  1393,   218,    60,    61,    62,   179,
    1394,   423,  1395,     0,     0,     0,   233,   221,   221, -1014,
   -1014, -1014, -1014, -1014,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,     0,     0,   224,     0,     0,   135,   135,     0,
     233,   652,     0,   233,   135,  1079,     0,     0,     0,  1396,
    1397,     0,  1398,     0,     0,     0,     0,     0,     0,     0,
       0,   219,     0,     0,     0,     0,     0,     0,   165,     0,
     219,     0,     0,   424,  1728,   562,  1729,   219,     0,     0,
       0,     0,   135,     0,   219,  1735,   233,     0,     0,  1854,
    1799,     0,   224,     0,     0,  1524,   331,  1860,     0,     0,
     991,     0,     0,  1863,     0,   121,  1864,     0,     0,   165,
     165,     0,     0,     0,  1740,     0,   165,     0,     0,     0,
       0,  1643,  1643,     0,     0,  1650,  1650,     0,   129,   218,
       0,     0,  1773,   224,     0,   224,   221,   221,   664,   368,
       0,     0,     0,  1697,     0,     0,   121,   121,     0,   135,
       0,     0,     0,   121,   165,     0,   135,   664,     0,     0,
     224,     0,     0,  1391,     0,   664,     0,     0,     0,   129,
     129,     0,   652,     0,     0,     0,   129,     0,     0,     0,
     233,   233,  1391,     0,   837,     0,     0,     0,     0,     0,
       0,   121,     0,     0,     0,     0,     0,     0,  1797,     0,
       0,     0,     0,     0,     0,    13,     0,     0,   219,     0,
       0,     0,     0,     0,   129,     0,  1811,     0,     0,     0,
       0,   165,     0,     0,    13,     0,     0,   224,   165,     0,
       0,  1391,     0,     0,  1840,     0,   837,     0,     0,     0,
       0,  1847,   224,   224,  1720,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   121,     0,
       0,     0,     0,     0,     0,   121,     0,  1392,     0,     0,
     221,     0,  1393,    13,    60,    61,    62,   179,  1394,   423,
    1395,   129,     0,   233,   233,     0,  1392,     0,   129,     0,
       0,  1393,   233,    60,    61,    62,   179,  1394,   423,  1395,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     652,     0,     0,   218,     0,   221,     0,  1396,  1397,     0,
    1398,     0,     0,     0,     0,     0,     0,     0,  1782,   221,
     221,     0,     0,     0,     0,  1392,  1396,  1397,     0,  1398,
    1393,   424,    60,    61,    62,   179,  1394,   423,  1395,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     424,   224,   224,  1528,     0,     0,     0,     0,     0,     0,
       0,   218,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1530,     0,     0,  1396,  1397,     0,  1398,     0,
       0,     0,   219,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   931,     0,     0,     0,     0,     0,     0,   424,
     221,     0,   218,     0,   218,     0,     0,   931,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1679,     0,     0,     0,     0,     0,     0,     0,   218,
     837,     0,     0,   220,   220,     0,     0,   234,     0,     0,
     219,     0,     0,   233,   233,   837,   837,   837,   837,   837,
       0,     0,     0,   837,   505,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   233,
       0,   219,     0,   219,     0,   224,     0,     0,   652,     0,
       0,     0,     0,     0,     0,     0,   218,     0,     0,     0,
       0,     0,     0,     0,   488,   489,     0,     0,   219,   233,
       0,   218,   218,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   233,   233,     0,     0,     0,
     224,     0,     0,     0,     0,   233,     0,     0,     0,     0,
       0,   233,     0,     0,   224,   224,     0,     0,     0,     0,
       0,     0,     0,     0,   233,     0,     0,     0,   652,     0,
       0,     0,   837,     0,     0,   233,     0,     0,     0,     0,
     490,   491,     0,     0,     0,   219,   433,   434,   435,     0,
       0,     0,     0,   233,     0,     0,     0,   233,     0,     0,
     219,   219,     0,     0,     0,   436,     0,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,     0,
     459,     0,     0,     0,     0,   224,     0,     0,     0,     0,
     218,   218,   460,   220,     0,     0,     0,     0,     0,     0,
       0,   669,     0,     0,   233,     0,     0,   233,     0,   233,
     505,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,     0,   837,     0,   233,     0,     0,   837,
     837,   837,   837,   837,   837,   837,   837,   837,   837,   837,
     837,   837,   837,   837,   837,   837,   837,   837,   837,   837,
     837,   837,   837,   837,   837,   837,   837,     0,     0,     0,
     488,   489,   433,   434,   435,     0,     0,     0,     0,   219,
     219,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   436,   837,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,     0,   459,     0,   334,     0,
       0,     0,   233,     0,   233,     0,     0,     0,   460,     0,
       0,     0,   220,     0,   218,     0,   490,   491,    36,     0,
     213,   220,     0,  1291,     0,     0,     0,     0,   220,     0,
       0,   233,     0,     0,   233,   220,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,   220,     0,     0,     0,
       0,     0,     0,     0,   233,     0,     0,     0,     0,   218,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   218,   218,     0,   837,     0,     0,     0,
       0,     0,     0,     0,     0,   233,     0,   791,     0,   233,
       0,     0,   837,   219,   837,     0,   761,     0,    86,    87,
       0,    88,   184,    90,     0,     0,     0,     0,     0,     0,
     837,     0,     0,     0,     0,     0,   234,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,   219,   433,
     434,   435,     0,   762,     0,   116,   904,     0,   233,   233,
       0,   233,   219,   219,   218,     0,     0,     0,   436,   220,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,     0,   459,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   460,     0,     0,   505,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,     0,     0,     0,   841,     0,     0,   334,     0,   334,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,   233,     0,   233,     0,     0,     0,
       0,   837,   233,     0,     0,   837,     0,   837,   488,   489,
     837,     0,     0,     0,   433,   434,   435,     0,     0,   233,
     233,     0,     0,   233,     0,     0,   841,     0,     0,     0,
     233,     0,     0,   436,   334,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     460,     0,   233,     0,   490,   491,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   837,     0,     0,
       0,     0,     0,   936,     0,     0,     0,   233,   233,     0,
       0,     0,     0,   220,     0,   233,     0,   233,  1480,     0,
       0,     0,     0,     0,     0,     0,     0,   334,     0,     0,
     334,     0,     0,     0,     0,     0,     0,     0,     0,   233,
       0,   233,     0,     0,     0,     0,     0,   233,     0,     0,
       0,     0,     0,     0,     0,   884,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
       0,   220,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   837,   837,     0,     0,     0,     0,    48,   837,
       0,   233,     0,     0,     0,     0,     0,   233,     0,   233,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1481,     0,   220,     0,   220,     0,     0,     0,   975,     0,
       0,     0,     0,  1482,  1483,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   220,
     841,   183,     0,     0,    84,  1484,     0,    86,    87,     0,
      88,  1485,    90,     0,   820,   841,   841,   841,   841,   841,
       0,     0,     0,   841,     0,     0,     0,   334,   822,     0,
       0,   839,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,  1094,
       0,     0,     0,     0,     0,     0,     0,     0,   233,     0,
       0,     0,     0,    36,     0,     0,   220,     0,     0,     0,
       0,     0,   821,     0,     0,   233,     0,     0,     0,  1112,
       0,   220,   220,   839,    48,     0,     0,     0,     0,     0,
       0,     0,     0,   233,     0,     0,  1112,     0,     0,   837,
       0,     0,     0,     0,     0,   220,     0,     0,     0,     0,
     837,     0,    36,     0,     0,     0,   837,     0,     0,     0,
     837,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     334,   334,   841,    48,     0,  1153,     0,   183,     0,   334,
      84,     0,   233,    86,    87,     0,    88,   184,    90,     0,
       0,     0,     0,     0,     0,     0,     0,   234,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   837,     0,     0,     0,   183,     0,     0,    84,
       0,   233,    86,    87,     0,    88,   184,    90,     0,     0,
     220,   220,     0,     0,     0,     0,     0,   233,     0,     0,
       0,     0,     0,     0,     0,     0,   233,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   233,     0,     0,   841,     0,   220,     0,  1739,   841,
     841,   841,   841,   841,   841,   841,   841,   841,   841,   841,
     841,   841,   841,   841,   841,   841,   841,   841,   841,   841,
     841,   841,   841,   841,   841,   841,   841,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
       0,     0,     0,     0,     0,     0,     0,   839,     0,     0,
     222,   222,   841,     0,   238,     0,     0,     0,     0,     0,
     334,   334,   839,   839,   839,   839,   839,     0,     0,     0,
     839,     0,     0,     0,   433,   434,   435,   488,   489,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   436,   220,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     460,     0,     0,     0,   220,     0,     0,     0,     0,   220,
       0,     0,   334,   490,   491,     0,     0,     0,     0,     0,
       0,     0,     0,   220,   220,     0,   841,     0,   334,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   334,   841,     0,   841,     0,     0,     0,     0,   839,
     433,   434,   435,     0,     0,     0,     0,     0,     0,     0,
     841,     0,     0,     0,     0,     0,     0,     0,     0,   436,
     334,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,     0,   459,     0,     0,     0,     0,     0,
       0,  1390,     0,     0,   220,     0,   460,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   334,     0,     0,   334,     0,   822,     0,   979,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   839,     0,     0,     0,     0,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   839,   839,
     839,   839,   839,   839,     0,     0,     0,     0,     0,     0,
       0,   841,   220,     0,     0,   841,     0,   841,     0,     0,
     841,     0,     0,     0,     0,     0,     0,     0,     0,   839,
    1477,     0,     0,  1490,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   334,
       0,   334,   838,     0,  1104,     0,     0,     0,    48,   222,
       0,     0,     0,     0,     0,     0,     0,     0,   222,     0,
       0,     0,   220,     0,     0,   222,     0,     0,   334,     0,
       0,   334,   222,     0,     0,     0,     0,   841,     0,     0,
       0,     0,     0,   238,     0,     0,     0,  1549,  1550,     0,
       0,     0,     0,     0,   838,     0,     0,  1490,     0,     0,
       0,     0,     0,     0,     0,   761,     0,    86,    87,     0,
      88,   184,    90,   839,     0,     0,     0,     0,     0,     0,
       0,     0,   334,     0,     0,     0,   334,     0,     0,   839,
       0,   839,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   839,     0,     0,
       0,     0,   794,   238,   116,     0,     0,     0,     0,     0,
       0,     0,   841,   841,     0,     0,     0,     0,     0,   841,
       0,  1695,     0,     0,     0,     0,   433,   434,   435,  1490,
       0,     0,     0,     0,     0,   334,   334,     0,     0,     0,
       0,     0,     0,     0,     0,   436,   222,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,     0,
     459,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   460,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,   842,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,   334,     0,   334,     0,     0,     0,     0,   839,     0,
       0,     0,   839,     0,   839,     0,     0,   839,     0,     0,
       0,     0,     0,     0,     0,     0,   334,     0,   838,     0,
       0,     0,     0,   842,     0,     0,     0,   334,     0,     0,
       0,     0,     0,   838,   838,   838,   838,   838,     0,   841,
     381,   838,     0,    86,    87,     0,    88,   184,    90,     0,
     841,     0,     0,     0,     0,     0,   841,     0,     0,     0,
     841,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   839,     0,     0,     0,   382,     0,
    1160,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,   334,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   841,   459,     0,     0,   334,     0,   334,     0,
       0,  1808,     0,     0,   334,   460,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1477,     0,     0,
     838,     0,     0,     0,     0,     0,     0,     0,   222,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   839,
     839,     0,     0,     0,     0,     0,   839,     0,   843,     0,
       0,   433,   434,   435,   334,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   222,
     436,   222,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,     0,   459,   222,   842,     0,     0,
     877,     0,     0,     0,     0,     0,     0,   460,     0,     0,
       0,     0,   842,   842,   842,   842,   842,     0,     0,     0,
     842,     0,   838,     0,     0,     0,     0,   838,   838,   838,
     838,   838,   838,   838,   838,   838,   838,   838,   838,   838,
     838,   838,   838,   838,   838,   838,   838,   838,   838,   838,
     838,   838,   838,   838,   838,   334,     0,     0,     0,     0,
       0,     0,     0,   222,     0,     0,     0,     0,     0,     0,
       0,     0,   334,     0,     0,     0,     0,     0,   222,   222,
     838,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1741,     0,     0,     0,     0,     0,   839,     0,     0,     0,
       0,     0,   238,     0,     0,     0,     0,   839,     0,     0,
       0,     0,     0,   839,     0,     0,     0,   839,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   842,
       0,     0,     0,     0,   433,   434,   435,     0,     0,   334,
       0,     0,     0,     0,     0,  1171,     0,     0,     0,     0,
       0,     0,     0,   436,   238,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,   839,
       0,     0,     0,     0,   838,     0,     0,     0,     0,     0,
     460,     0,     0,     0,     0,     0,     0,   222,   222,     0,
     838,     0,   838,     0,  1018,     0,     0,     0,     0,     0,
       0,     0,     0,   334,     0,     0,     0,     0,   838,  1040,
    1041,  1042,  1043,     0,     0,     0,     0,  1050,   334,     0,
       0,   842,     0,   238,     0,     0,   842,   842,   842,   842,
     842,   842,   842,   842,   842,   842,   842,   842,   842,   842,
     842,   842,   842,   842,   842,   842,   842,   842,   842,   842,
     842,   842,   842,   842,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   433,   434,
     435,     0,     0,     0,     0,     0,     0,     0,     0,   842,
       0,     0,     0,     0,     0,     0,     0,   436,     0,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,     0,   459,     0,     0,     0,     0,     0,  1194,     0,
       0,   222,     0,     0,   460,     0,  1150,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   838,
       0,     0,     0,   838,     0,   838,     0,     0,   838,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   238,     0,     0,     0,     0,   222,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,   222,     0,   842,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   842,
       0,   842,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   842,     0,     0,
       0,     0,     0,     0,     0,   838,     0,     0,     0,     0,
       0,     0,     0,  1237,  1240,  1241,  1242,  1244,  1245,  1246,
    1247,  1248,  1249,  1250,  1251,  1252,  1253,  1254,  1255,  1256,
    1257,  1258,  1259,  1260,  1261,  1262,  1263,  1264,  1265,  1266,
    1267,   222,  1513,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,     0,   459,     0,  1279,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   460,   433,   434,   435,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     838,   838,     0,     0,     0,     0,   436,   838,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
       0,   459,   433,   434,   435,     0,     0,     0,   842,   238,
       0,     0,   842,   460,   842,     0,     0,   842,     0,     0,
       0,   436,  1361,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,     0,   459,     0,     0,     0,
    1356,     0,     0,     0,     0,     0,     0,     0,   460,     0,
       0,     0,     0,     0,     0,     0,  1371,     0,  1372,     0,
       0,     0,     0,     0,   433,   434,   435,     0,     0,   238,
       0,     0,     0,     0,  1382,     0,     0,     0,     0,     0,
       0,     0,     0,   436,   842,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,     0,
       0,     0,     0,     0,     0,     0,     0,   838,     0,     0,
     460,   433,   434,   435,     0,     0,     0,     0,   838,     0,
       0,     0,     0,     0,   838,     0,     0,     0,   838,     0,
     436,  1514,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,     0,   459,     0,     0,     0,   842,
     842,     0,     0,     0,     0,     0,   842,   460,     0,     0,
       0,     0,     0,     0,  1362,  1700,     0,     0,     0,     0,
       0,     0,     0,     0,   433,   434,   435,     0,     0,     0,
     838,     0,     0,     0,     0,  1461,     0,     0,     0,  1463,
       0,  1464,     0,   436,  1465,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,   433,
     434,   435,     0,     0,     0,     0,     0,     0,     0,     0,
     460,     0,     0,     0,     0,     0,   461,     0,   436,     0,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,     0,   459,     0,     0,     0,     0,     0,     0,
       0,  1544,     0,     0,     0,   460,     0,     0,     0,     0,
     505,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   546,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   842,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   842,     0,     0,
       0,     0,     0,   842,     0,     0,     0,   842,     0,     0,
     488,   489,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   433,   434,   435,     0,
       0,  1783,     0,     0,     0,     0,  1688,  1689,     0,     0,
       0,     0,     0,  1694,     0,   436,   548,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   842,
     459,     0,     0,   433,   434,   435,   490,   491,     0,     0,
       0,     0,   460,     0,     0,     0,     0,     0,     0,     0,
       0,   566,   436,     0,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,     0,   459,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,   460,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,  1750,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,  1760,     0,    41,    42,    43,    44,
    1765,    45,     0,    46,  1767,    47,     0,   777,    48,    49,
       0,     0,     0,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,    69,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,   590,    80,     0,     0,     0,
       0,    81,    82,    83,    84,    85,  1801,    86,    87,     0,
      88,    89,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,    95,     0,    96,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1120,   116,   117,     0,   118,   119,     5,
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
      50,    51,    52,    53,    54,    55,    56,     0,    57,    58,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,    69,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,    81,    82,
      83,    84,    85,     0,    86,    87,     0,    88,    89,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,    95,     0,    96,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1292,   116,   117,     0,   118,   119,     5,     6,     7,     8,
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
      95,     0,    96,     0,    97,    98,     0,    99,   100,   101,
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
       0,     0,   183,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   184,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,   671,   116,   117,     0,   118,   119,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   183,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   184,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1093,   116,   117,     0,   118,   119,     5,     6,     7,
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
       0,    80,     0,     0,     0,     0,   183,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   184,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1134,   116,
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
       0,     0,     0,   183,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   184,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1200,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,  1202,
      45,     0,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     183,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     184,    90,    91,     0,     0,    92,     0,     0,    93,     0,
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
       0,    47,  1357,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   183,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   184,    90,    91,
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
       0,     0,     0,     0,   183,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   184,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,  1468,   116,   117,     0,
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
       0,   183,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   184,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1691,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,  1736,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,    63,    64,    65,     0,    66,    67,
      68,     0,    70,    71,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   183,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   184,    90,
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
      80,     0,     0,     0,     0,   183,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   184,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1771,   116,   117,
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
       0,     0,   183,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   184,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1772,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
    1775,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   183,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   184,
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
       0,    80,     0,     0,     0,     0,   183,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   184,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1792,   116,
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
       0,     0,     0,   183,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   184,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1848,   116,   117,     0,   118,
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
     183,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     184,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,  1855,   116,   117,     0,   118,   119,     5,     6,
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
       0,     0,    80,     0,     0,     0,     0,   183,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   184,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   532,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
     179,   180,    65,     0,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   183,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   184,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,   807,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,   179,   180,    65,
       0,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   183,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   184,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,  1020,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,    49,     0,     0,     0,
      50,    51,    52,    53,     0,    55,    56,     0,    57,     0,
      59,    60,    61,    62,   179,   180,    65,     0,    66,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
      79,     0,     0,    80,     0,     0,     0,     0,   183,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   184,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,  1539,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,   179,   180,    65,     0,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   183,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   184,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
    1683,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,     0,    55,
      56,     0,    57,     0,    59,    60,    61,    62,   179,   180,
      65,     0,    66,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,   183,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   184,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,   179,   180,    65,     0,    66,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   183,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   184,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   396,    12,     0,     0,     0,     0,     0,     0,   744,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   179,   180,   181,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   183,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   184,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,   339,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,     0,     0,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   179,
     180,   181,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   182,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   183,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   184,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,   339,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   185,     0,   340,     0,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,     0,     0,   686,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1079,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   179,   180,   181,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   182,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     183,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     184,    90,     0,   687,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   185,     0,
       0,     0,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   179,   180,   181,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   183,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   184,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   185,     0,     0,   802,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,     0,   459,     0,  1147,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   460,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     179,   180,   181,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   183,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   184,    90,     0,  1148,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   185,     0,     0,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   396,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   179,   180,   181,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   183,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   184,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   433,   434,   435,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     436,     0,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,     0,   459,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   460,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
     196,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   179,   180,   181,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   183,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   184,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,  1086,  1087,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   185,     0,     0,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,     0,     0,     0,   226,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1079,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   179,   180,   181,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   183,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   184,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   185,     0,   433,   434,   435,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   436,     0,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,     0,   459,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   460,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   179,   180,
     181,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   183,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   184,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,  1737,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     185,     0,   259,   434,   435,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   436,     0,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,     0,   459,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   460,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   179,   180,   181,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     182,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   183,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   184,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   185,     0,   262,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   396,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   179,   180,   181,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   183,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   184,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   433,   434,   435,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   436,     0,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,     0,
     459,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   460,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   179,
     180,   181,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   182,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   183,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   184,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,  1547,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   185,   530,     0,     0,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   699,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   179,   180,   181,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   182,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     183,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     184,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   185,     0,
       0,     0,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,     0,   459,     0,     0,
     744,     0,     0,     0,     0,     0,     0,     0,     0,   460,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   179,   180,   181,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   182,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   183,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   184,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   185,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10, -1014, -1014, -1014, -1014,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,     0,   459,     0,     0,   785,     0,     0,
       0,     0,     0,     0,     0,     0,   460,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     179,   180,   181,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   183,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   184,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   185,     0,     0,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
       0,     0,     0,     0,   787,     0,     0,     0,     0,     0,
       0,     0,     0,  1079,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   179,   180,   181,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   182,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   183,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   184,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   185,
       0,     0,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
   -1014, -1014, -1014, -1014,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,     0,     0,     0,
       0,  1192,     0,     0,     0,     0,     0,     0,     0,     0,
    1079,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   179,   180,   181,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   183,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   184,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   185,     0,   433,   434,
     435,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   436,     0,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,     0,   459,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   460,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   179,   180,   181,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   182,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   183,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   184,    90,     0,     0,     0,
      92,     0,     0,    93,     0,  1384,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   185,     0,   433,   434,   435,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   436,   953,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,     0,   459,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   460,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,   632,
      38,     0,     0,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,     0,     0,     0,     0,    60,    61,    62,   179,   180,
     181,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   183,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   184,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     185,     0,     0,     0,     0,   116,   117,     0,   118,   119,
     264,   265,     0,   266,   267,  1054,  1055,   268,   269,   270,
     271,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1056,   272,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,     0,     0,     0,
       0,   274,     0,     0,     0,     0,     0,     0,     0,     0,
    1079,     0,     0,     0,     0,   276,   277,   278,   279,   280,
     281,   282,     0,     0,     0,    36,     0,   213,     0,     0,
       0,     0,     0,     0,     0,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,    48,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
       0,     0,     0,   731,   318,   319,   320,     0,     0,     0,
     321,   559,   560,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   264,   265,     0,   266,   267,     0,   561,
     268,   269,   270,   271,     0,    86,    87,     0,    88,   184,
      90,   326,     0,   327,     0,     0,   328,   272,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   274,     0,     0,     0,     0,     0,
     732,     0,   116,     0,     0,     0,     0,     0,   276,   277,
     278,   279,   280,   281,   282,     0,     0,     0,    36,     0,
     213,     0,     0,     0,     0,     0,     0,     0,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,    48,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,     0,     0,     0,   317,   318,   319,   320,
       0,     0,     0,   321,   559,   560,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   264,   265,     0,   266,
     267,     0,   561,   268,   269,   270,   271,     0,    86,    87,
       0,    88,   184,    90,   326,     0,   327,     0,     0,   328,
     272,     0,   273,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   274,     0,   275,
       0,     0,     0,   732,     0,   116,     0,     0,     0,     0,
       0,   276,   277,   278,   279,   280,   281,   282,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,    48,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,     0,     0,     0,     0,
     318,   319,   320,    36,     0,     0,   321,   322,   323,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,   324,     0,     0,    84,   325,
       0,    86,    87,     0,    88,   184,    90,   326,     0,   327,
       0,     0,   328,     0,     0,     0,     0,     0,     0,   329,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,   264,   265,   330,   266,   267,     0,  1663,   268,   269,
     270,   271,     0,    86,    87,     0,    88,   184,    90,     0,
       0,     0,     0,     0,     0,   272,     0,   273,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   274,     0,   275,   692,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   276,   277,   278,   279,
     280,   281,   282,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,    48,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,     0,     0,     0,     0,   318,   319,   320,    36,     0,
       0,   321,   322,   323,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
     324,     0,     0,    84,   325,     0,    86,    87,     0,    88,
     184,    90,   326,     0,   327,     0,     0,   328,     0,     0,
       0,     0,     0,     0,   329,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   264,   265,   330,   266,
     267,     0,  1732,   268,   269,   270,   271,     0,    86,    87,
       0,    88,   184,    90,     0,     0,     0,     0,     0,     0,
     272,     0,   273,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   274,     0,   275,
     950,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   276,   277,   278,   279,   280,   281,   282,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,    48,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,     0,     0,     0,   317,
     318,   319,   320,    36,     0,     0,   321,   322,   323,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,   324,     0,     0,    84,   325,
       0,    86,    87,     0,    88,   184,    90,   326,     0,   327,
       0,     0,   328,     0,     0,     0,     0,     0,     0,   329,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,   264,   265,   330,   266,   267,     0,     0,   268,   269,
     270,   271,     0,    86,    87,     0,    88,   184,    90,     0,
       0,     0,     0,     0,     0,   272,     0,   273,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   274,     0,   275,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   276,   277,   278,   279,
     280,   281,   282,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,    48,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,     0,     0,     0,     0,   318,   319,   320,     0,     0,
       0,   321,   322,   323,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     324,     0,     0,    84,   325,     0,    86,    87,     0,    88,
     184,    90,   326,     0,   327,     0,     0,   328,     0,     0,
       0,     0,     0,     0,   329,  1472,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   264,   265,   330,   266,
     267,     0,     0,   268,   269,   270,   271,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     272,   436,   273,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,     0,   459,   274,     0,   275,
       0,     0,     0,     0,     0,     0,     0,     0,   460,     0,
       0,   276,   277,   278,   279,   280,   281,   282,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,    48,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,     0,     0,     0,     0,
     318,   319,   320,     0,     0,    36,   321,   322,   323,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   324,    48,     0,    84,   325,
       0,    86,    87,     0,    88,   184,    90,   326,     0,   327,
       0,     0,   328,     0,     0,     0,     0,     0,     0,   329,
       0,  1494,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,   330,  1564,  1565,  1566,  1567,  1568,     0,
       0,  1569,  1570,  1571,  1572,    86,    87,     0,    88,   184,
      90,     0,     0,     0,     0,     0,     0,     0,  1573,  1574,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,  1575,     0,  1495,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1576,
    1577,  1578,  1579,  1580,  1581,  1582,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1583,
    1584,  1585,  1586,  1587,  1588,  1589,  1590,  1591,  1592,  1593,
      48,  1594,  1595,  1596,  1597,  1598,  1599,  1600,  1601,  1602,
    1603,  1604,  1605,  1606,  1607,  1608,  1609,  1610,  1611,  1612,
    1613,  1614,  1615,  1616,  1617,  1618,  1619,  1620,  1621,  1622,
    1623,     0,     0,     0,  1624,  1625,     0,  1626,  1627,  1628,
    1629,  1630,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1631,  1632,  1633,     0,     0,     0,    86,
      87,     0,    88,   184,    90,  1634,     0,  1635,  1636,     0,
    1637,     0,     0,     0,     0,     0,     0,  1638,  1639,     0,
    1640,     0,  1641,  1642,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   264,   265,
       0,   266,   267,     0,   435,   268,   269,   270,   271,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   436,   272,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,     0,   459,     0,     0,   274,
       0,     0,     0,     0,     0,     0,     0,     0,   460,     0,
       0,     0,     0,   276,   277,   278,   279,   280,   281,   282,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,    48,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,     0,     0,
       0,   317,   318,   319,   320,     0,     0,     0,   321,   559,
     560,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   264,   265,     0,   266,   267,     0,   561,   268,   269,
     270,   271,     0,    86,    87,     0,    88,   184,    90,   326,
       0,   327,     0,     0,   328,   272,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   274,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   276,   277,   278,   279,
     280,   281,   282,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,    48,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,     0,     0,     0,  1235,   318,   319,   320,     0,     0,
       0,   321,   559,   560,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   264,   265,     0,   266,   267,     0,
     561,   268,   269,   270,   271,     0,    86,    87,     0,    88,
     184,    90,   326,     0,   327,     0,     0,   328,   272,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   274,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   276,
     277,   278,   279,   280,   281,   282,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
      48,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,     0,     0,     0,     0,   318,   319,
     320,     0,     0,     0,   321,   559,   560,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   561,     0,     0,     0,     0,     0,    86,
      87,     0,    88,   184,    90,   326,     0,   327,     0,     0,
     328,     0,     0,     0,     0,     0,     0,     0,     0,   433,
     434,   435,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   436,     0,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,     0,   459,  1053,  1054,  1055,     0,     0,     0,
       0,     0,     0,     0,     0,   460,     0,     0,     0,     0,
       0,     0,     0,  1056,     0,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,   433,   434,   435,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1079,     0,     0,     0,     0,     0,   436,     0,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
       0,   459,   273,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   460,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   275,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   273,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
     799,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,   275,     0,     0,     0,     0,
    -399,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   179,   180,   423,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,  1234,     0,   549,   550,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,   273,     0,     0,     0,   183,   555,     0,    84,   325,
       0,    86,    87,     0,    88,   184,    90,     0,     0,     0,
       0,     0,     0,  1090,     0,     0,     0,     0,   275,   329,
       0,     0,     0,   549,   550,   424,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      36,   183,     0,   330,    84,   325,     0,    86,    87,     0,
      88,   184,    90,     0,     0,     0,     0,     0,     0,   273,
       0,    48,     0,     0,     0,   329,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   275,     0,     0,   330,
       0,     0,     0,     0,     0,     0,   549,   550,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,   183,     0,     0,    84,   325,     0,
      86,    87,     0,    88,   184,    90,     0,  1035,     0,    48,
       0,     0,   273,     0,     0,     0,     0,     0,   329,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   275,
       0,     0,   330,     0,   549,   550,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,   183,     0,     0,    84,   325,     0,    86,    87,
       0,    88,   184,    90,     0,  1368,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,   329,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
     330,     0,     0,     0,     0,     0,     0,   549,   550,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   183,     0,     0,    84,   325,
       0,    86,    87,     0,    88,   184,    90,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   329,
       0,     0,     0,  1243,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     828,   829,     0,   330,     0,     0,   830,     0,   831,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     832,     0,     0,     0,     0,     0,     0,     0,    33,    34,
      35,    36,   433,   434,   435,     0,     0,     0,     0,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   436,    48,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,     0,   459,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   833,   460,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     216,     0,     0,     0,     0,   183,    82,    83,    84,   834,
       0,    86,    87,     0,    88,   184,    90,     0,  1014,     0,
      92,     0,     0,     0,     0,     0,     0,     0,     0,   835,
       0,     0,     0,     0,    97,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      28,     0,     0,   836,   508,     0,     0,     0,    33,    34,
      35,    36,     0,   213,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   215,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1015,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     216,     0,     0,     0,     0,   183,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   184,    90,     0,     0,     0,
      92,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,   828,   829,   217,     0,     0,     0,   830,   116,   831,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   832,     0,     0,     0,     0,     0,     0,     0,    33,
      34,    35,    36,   433,   434,   435,     0,     0,     0,     0,
       0,   214,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   436,    48,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,     0,   459,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   833,   460,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   216,     0,     0,     0,     0,   183,    82,    83,    84,
     834,     0,    86,    87,     0,    88,   184,    90,     0,     0,
       0,    92,     0,     0,     0,     0,     0,     0,     0,     0,
     835,   981,   982,     0,     0,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   983,     0,     0,   836,   517,     0,     0,     0,   984,
     985,   986,    36,   433,   434,   435,     0,     0,     0,     0,
       0,   987,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   436,    48,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,     0,   459,     0,     0,
       0,     0,     0,     0,     0,    36,     0,     0,   988,   460,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   989,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,    86,    87,     0,    88,   184,    90,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     990,     0,     0,     0,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    28,     0,     0,     0,   956,     0,     0,     0,    33,
      34,    35,    36,  1648,   213,    86,    87,  1649,    88,   184,
      90,   214,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   215,     0,     0,  1495,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   216,     0,     0,     0,     0,   183,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   184,    90,     0,     0,
       0,    92,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,    28,     0,   217,     0,     0,   603,     0,   116,
      33,    34,    35,    36,     0,   213,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   215,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   623,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   216,     0,     0,     0,     0,   183,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   184,    90,     0,
       0,     0,    92,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,    28,   217,   970,     0,     0,     0,
     116,     0,    33,    34,    35,    36,     0,   213,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,   215,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1079,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   216,     0,     0,     0,     0,   183,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   184,
      90,     0,     0,     0,    92,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,    28,     0,   217,     0,     0,
       0,     0,   116,    33,    34,    35,    36,     0,   213,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1115,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   216,     0,     0,     0,     0,
     183,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     184,    90,     0,     0,     0,    92,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,    28,     0,   217,     0,
       0,     0,     0,   116,    33,    34,    35,    36,     0,   213,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   215,
       0,     0,     0,  1174,  1175,  1176,    36,     0,     0,     0,
       0,     0,     0,    73,     0,    74,    75,    76,    77,    78,
       0,     0,    36,     0,     0,     0,   216,    48,     0,     0,
       0,   183,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   184,    90,    48,     0,     0,    92,     0,     0,     0,
       0,   348,   349,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,   217,
      33,    34,    35,    36,   116,   213,    86,    87,     0,    88,
     184,    90,   214,     0,     0,     0,     0,     0,     0,   350,
       0,     0,    86,    87,    48,    88,   184,    90,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   231,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    74,    75,    76,    77,    78,     0,     0,    36,     0,
       0,     0,   216,     0,     0,     0,     0,   183,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   184,    90,    48,
       0,     0,    92,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   232,    33,    34,    35,    36,
     116,   213,     0,     0,     0,     0,     0,     0,   646,     0,
       0,     0,   183,     0,     0,    84,    85,     0,    86,    87,
      48,    88,   184,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   216,     0,
       0,     0,     0,   183,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   184,    90,     0,     0,     0,    92,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    97,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   433,   434,
     435,   647,     0,     0,     0,     0,   648,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   436,     0,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,     0,   459,   433,   434,   435,     0,     0,     0,     0,
       0,     0,     0,     0,   460,     0,     0,     0,     0,     0,
       0,     0,   436,     0,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,     0,   459,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   460,
    1053,  1054,  1055,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1056,
    1000,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,     0,     0,  1053,  1054,  1055,     0,     0,
       0,     0,     0,     0,     0,     0,  1079,     0,     0,     0,
       0,     0,     0,     0,  1056,  1321,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1079,  1053,  1054,  1055,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1056,  1225,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,     0,     0,  1053,  1054,  1055,
       0,     0,     0,     0,     0,     0,     0,     0,  1079,     0,
       0,     0,     0,     0,     0,     0,  1056,  1378,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1079,    36,     0,   898,   899,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1460,    48,     0,     0,     0,     0,
       0,     0,     0,  1481,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,  1482,  1483,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,   183,     0,    48,    84,    85,  1546,
      86,    87,     0,    88,  1485,    90,     0,     0,     0,    48,
       0,     0,     0,     0,    86,    87,     0,    88,   184,    90,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   420,    36,    86,    87,     0,    88,   184,
      90,     0,     0,     0,     0,   591,     0,     0,    86,    87,
       0,    88,   184,    90,     0,    48,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,   595,     0,     0,    86,    87,     0,    88,   184,    90,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   350,     0,     0,    86,    87,     0,
      88,   184,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   433,   434,
     435,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   811,   436,     0,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,     0,   459,   433,   434,   435,     0,     0,     0,     0,
       0,     0,     0,     0,   460,     0,     0,     0,     0,     0,
       0,     0,   436,     0,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   812,   459,  1053,  1054,
    1055,     0,     0,     0,     0,     0,     0,     0,     0,   460,
       0,     0,     0,     0,     0,     0,     0,  1056,  1383,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,  1053,  1054,  1055,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1079,     0,     0,     0,     0,     0,
    1056,     0,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1055,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1079,     0,     0,
       0,  1056,     0,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1056,  1079,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
    1078,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1079,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     460,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1079,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,     0,   459,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   460,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1079
};

static const yytype_int16 yycheck[] =
{
       5,     6,   161,     8,     9,    10,    11,    12,   689,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    54,     4,    28,    29,   135,    32,    91,  1137,   929,
       4,    95,    96,   187,    55,     4,    30,    42,    44,   113,
       4,   660,   233,    49,   113,    50,   389,    52,   389,   659,
      55,   166,    57,    42,   918,   459,    54,   496,   497,   949,
     639,   113,   229,     4,   502,   492,   161,   492,   810,    30,
     134,   113,    30,  1124,    79,   965,   817,     9,   188,     9,
      30,     9,   741,  1013,     9,    80,    30,   526,     9,    67,
      79,     9,     4,    14,     9,    67,   779,  1133,     9,    14,
       9,   528,     9,   528,     9,     4,     9,     9,   113,     9,
      46,    14,     9,     9,     9,     9,   185,    30,     9,     9,
       9,     9,     9,     0,     9,   243,     9,     9,     9,   244,
       9,    35,    67,   185,    46,     4,    33,     9,     9,     9,
      86,    80,   151,   185,  1677,   172,   157,    80,   217,    35,
      67,    80,   130,   131,    46,    14,   151,    26,    27,    86,
     172,    67,   200,   232,    80,   217,   111,    30,   130,   131,
      46,    30,     4,   200,    67,   217,    80,   119,   118,    46,
     185,    46,   130,   131,   126,  1049,   126,   192,   200,    48,
     232,   200,   203,    51,    80,     4,   108,    98,    51,    67,
    1733,   113,   148,   115,   116,   117,   118,   119,   120,   121,
      67,    67,   217,    67,    80,    35,   151,    67,    67,    51,
     161,   148,    54,    67,   169,   203,    67,   232,   200,    67,
      67,   203,   165,    67,   151,    80,   165,    67,    67,    71,
     245,   173,   204,   248,   113,   173,   158,   159,   173,   161,
     255,   256,   200,   203,   155,    67,   374,    89,   202,    91,
      80,   165,   198,    95,    96,    58,   205,  1318,   203,   172,
     182,   425,   202,   203,  1325,   249,  1327,  1207,  1168,   253,
    1316,   202,  1023,   201,  1025,   173,   203,   202,    81,   205,
     201,    84,   204,   202,   200,   202,   201,   366,   156,   202,
     202,   201,   134,   156,   201,   338,   202,   202,   202,   201,
     203,   202,   202,   202,   366,   202,   185,   202,   171,   202,
     202,   202,   201,  1187,   366,   201,   135,   946,   907,   201,
     201,   201,   200,   198,   201,   203,   201,    35,    80,   205,
     338,    80,    80,   200,   511,   165,   203,   203,   217,   203,
     504,   200,   197,   203,   203,   419,   155,   226,   203,   203,
     365,   366,   203,   232,   474,   203,   203,   372,   200,   203,
     200,   175,   377,   151,    35,    35,   365,   130,   131,   188,
     249,   172,    80,   416,   253,   200,  1437,    54,   200,   201,
    1426,   396,  1428,   200,   151,   200,   172,   102,   103,   404,
       8,   475,    98,   202,   203,   469,   470,   471,   472,   200,
     415,   149,   150,   102,   103,   172,   200,   249,   416,    80,
      80,   253,   200,   165,   200,   257,   165,    98,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   475,   460,   894,   462,   463,   155,
     465,  1203,   403,  1122,   200,   459,   203,   165,   466,    80,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   650,   155,    98,   355,  1392,   493,   494,
    1526,   496,   497,   498,   499,   364,   395,   366,   459,   204,
     505,   459,   371,   508,   165,  1188,   338,   172,   935,   378,
     935,  1375,   517,   534,   519,   204,   200,   515,   677,    72,
      73,   526,    80,   962,    47,    48,  1136,   200,    86,   534,
     200,   536,    67,  1152,   403,   200,  1155,   148,   149,   150,
      67,    80,   155,   734,    98,   200,   459,    86,   666,   200,
     668,   200,    80,   466,   156,   348,   349,   350,    86,   157,
     200,    98,   151,   730,   569,   539,   970,   572,   573,   574,
     203,   403,   170,  1478,    54,   918,   767,   918,   647,   492,
     412,  1445,   677,   172,   416,   389,    66,   419,   381,   743,
     155,   149,   150,   200,   403,   130,   131,  1502,   603,  1504,
      35,   155,   515,   130,   131,   203,   273,    80,   275,   148,
     149,   150,   525,    86,   203,   528,   202,    80,   155,    72,
      73,   149,   150,    86,   151,   115,   116,   117,   118,   119,
     120,   695,   202,   502,   466,   467,   468,   469,   470,   471,
     472,   202,   647,   202,  1680,   202,   837,   202,   151,    80,
     208,  1334,    67,  1336,   845,    86,    67,   824,    98,    99,
     492,   202,   466,   330,   473,   783,   784,  1326,   151,   172,
     539,   789,   790,   200,   789,   148,   149,   150,   101,   102,
     103,   209,   687,   515,   532,   203,   149,   150,   492,    50,
      51,    52,   182,   463,   699,  1305,   528,   200,   200,    29,
     203,  1320,    75,    76,    77,    66,  1049,   539,  1049,    80,
    1819,   515,   202,    29,    87,    86,   200,    47,   149,   150,
      50,   525,  1161,   493,   528,  1834,   558,   732,   498,   128,
     129,    47,    67,  1172,    50,   151,   677,   202,   203,   587,
     115,   116,   117,   107,   202,   203,   413,   155,  1407,   416,
     200,   115,   116,   117,   118,   119,   120,   762,  1707,  1708,
    1443,   660,    45,   136,   137,   138,   139,   140,  1206,   202,
     602,   117,   118,   119,   147,  1703,  1704,  1813,   149,   150,
     153,   154,    47,    48,    49,    50,    51,    52,    66,   794,
     826,   827,  1828,   172,   167,   151,   628,   629,   591,   375,
     806,    66,   595,   379,   200,   810,   200,   600,   181,   101,
     102,   103,   207,   661,     9,   151,   200,   686,   182,  1438,
    1479,   115,   116,   117,   118,   119,   120,   151,   995,   405,
       8,   407,   408,   409,   410,    50,    51,    52,   151,    54,
     202,   815,   847,   200,  1187,    14,  1187,   151,    80,  1287,
    1533,    66,   202,   202,   126,   126,    14,   201,   172,   800,
      14,    98,   201,   695,   201,   206,   115,   116,   117,   118,
     119,   120,  1039,   201,   201,   744,   107,   126,   127,  1046,
     200,   200,   200,  1322,     9,   148,   553,  1787,   182,   201,
     201,   115,   116,   117,   118,   119,   120,    90,   903,    26,
      27,   201,   126,   127,   201,   969,  1806,     9,    14,   202,
     172,   200,   917,   162,  1814,   164,   785,     9,   787,   186,
     861,    80,   202,   921,   906,    80,   200,    80,   177,   189,
     179,   800,   906,   182,     9,     9,   202,   906,   943,    80,
     164,   202,   906,   812,   201,   201,   815,   779,   953,   781,
     201,   956,  1143,   958,   802,   128,   200,   962,   182,   807,
     201,    30,   183,   184,   185,   906,    67,   129,   800,   190,
     191,   171,   151,   194,   195,   132,   970,     9,   201,   151,
      14,   198,   814,   815,     9,  1424,   927,     9,   173,   656,
     657,   800,   861,   201,     9,  1000,    14,   128,   665,  1682,
    1006,   207,   207,   204,     9,    14,   200,   906,   921,   970,
     879,   207,   970,   201,   201,  1182,   207,   151,   201,    98,
     933,   202,   935,    87,   132,   894,   895,   202,   151,   861,
     929,     9,  1375,  1007,  1375,   867,   201,   906,   151,   871,
     872,   151,   203,  1117,   200,    14,  1727,   946,   200,   186,
    1048,   186,   861,  1051,     9,   203,    80,   970,   927,   891,
      14,   203,   202,    14,   207,  1232,    14,   203,  1009,   201,
    1011,    30,  1239,   866,   906,    14,   202,   870,  1083,  1084,
    1085,   198,   200,    30,  1089,  1090,   200,    14,  1001,   921,
       9,   200,    49,   200,   132,   927,  1117,   906,   201,   226,
     200,   933,  1445,   935,  1445,   200,   202,   200,   202,    14,
       9,   132,  1117,   201,   918,     9,    66,   921,   927,    80,
     207,     9,   200,  1804,    26,    27,   132,   200,    14,   933,
     202,   935,   203,   201,    80,   200,   203,   969,  1007,  1121,
    1009,   200,  1011,  1148,  1013,  1014,   201,  1121,   202,   981,
     982,   983,  1121,   203,   207,   132,  1161,  1121,     9,   826,
     827,    87,   148,    30,    74,   202,   201,  1172,  1173,  1001,
     173,     4,  1020,   202,   132,  1007,  1343,  1009,    30,  1011,
    1121,   201,   201,   132,     9,   201,     9,   204,   201,  1195,
     204,   203,    14,    80,   201,   203,   200,  1001,  1203,  1031,
    1009,   201,  1011,   201,   200,   202,   201,   201,  1213,   132,
     201,     9,    30,    46,   202,   202,  1048,   201,   201,  1051,
     202,   202,  1121,   203,  1198,   108,   160,   156,   355,    14,
      80,    47,    48,    49,    50,    51,    52,   364,    54,   113,
     203,   908,   132,   132,   371,  1049,    14,   172,  1080,  1408,
      66,   378,  1121,  1152,    80,    14,  1155,   924,   201,   201,
     201,    14,   389,   203,   202,  1097,    80,   200,  1100,  1210,
     937,   201,   203,   201,   132,   108,    14,    14,  1147,    14,
     113,   202,   115,   116,   117,   118,   119,   120,   121,  1121,
     202,   202,  1297,  1403,   203,     9,  1301,   204,  1303,   966,
      56,    80,   172,   200,    80,     9,  1311,   202,    80,   111,
      98,   151,  1121,    33,    98,   163,  1321,  1322,    14,   200,
     202,   201,   200,  1192,   226,   158,   159,   169,   161,  1198,
     173,    80,   166,   201,     9,    80,   202,  1206,  1207,   201,
     201,  1210,    14,   203,  1285,    14,  1500,    80,    80,   182,
      14,    80,  1184,  1294,    14,    80,  1188,   600,   866,   870,
    1027,  1795,   968,  1030,   963,   469,  1198,   472,   467,   909,
    1810,   204,  1538,  1204,  1360,   502,  1806,   605,  1210,  1384,
    1529,  1562,  1480,  1187,  1646,  1398,  1285,  1838,  1658,  1826,
    1394,  1525,   475,  1092,   576,  1294,  1089,   858,  1125,   576,
    1047,  1210,  1170,    26,    27,   982,  1183,    30,  1472,  1184,
     997,   933,   372,   826,   416,  1762,  1421,  1387,  1287,  1424,
    1108,  1320,  1032,  1080,    -1,    -1,    -1,    -1,  1276,    -1,
      -1,    54,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,  1386,    54,    -1,  1280,    -1,
      -1,    -1,    -1,   355,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,   364,    -1,   366,    -1,    -1,  1408,  1135,   371,
    1137,    -1,    -1,    -1,    -1,    -1,   378,  1475,    -1,  1453,
      -1,    -1,    26,    27,    -1,    -1,  1427,    -1,    -1,    -1,
      -1,    -1,  1433,    -1,  1435,    -1,    -1,  1164,    -1,  1347,
    1167,    -1,  1334,    -1,  1336,    -1,    -1,  1448,  1662,    -1,
      -1,    -1,  1517,    -1,    -1,  1363,  1457,  1386,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1427,    -1,
      -1,  1698,    -1,    -1,  1433,    -1,  1435,    -1,    -1,  1438,
      -1,    -1,  1547,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1218,    -1,    -1,  1386,  1222,    -1,    -1,    -1,   686,
    1392,    -1,    -1,  1537,  1538,    -1,  1398,    -1,    -1,    -1,
      -1,  1375,    -1,    -1,    -1,    -1,    -1,  1386,    -1,  1448,
      -1,    -1,    -1,    -1,  1453,    -1,    -1,    -1,  1457,    -1,
    1531,    -1,    -1,  1402,    -1,  1536,    -1,    -1,    -1,  1447,
     502,  1542,    -1,   226,    -1,    -1,    -1,  1548,  1456,    -1,
      -1,  1443,    -1,    -1,  1281,  1282,  1448,   744,    -1,  1467,
      -1,  1453,  1781,    -1,  1657,  1457,    -1,    -1,    -1,    -1,
      -1,    -1,  1531,    -1,    -1,    -1,    -1,    -1,    -1,  1448,
    1472,  1445,    -1,  1475,    -1,     4,  1478,    -1,  1457,    -1,
     273,    -1,   275,    -1,    -1,    -1,  1488,  1721,   785,  1657,
     787,    -1,  1667,  1495,    -1,    -1,    -1,  1536,  1537,  1538,
    1502,    -1,  1504,  1542,    -1,    -1,    -1,    -1,  1510,  1548,
      -1,    -1,   226,    -1,    -1,   812,    -1,    46,    -1,  1799,
      -1,  1539,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1508,
    1367,  1533,  1369,    -1,  1536,  1537,  1538,   330,    -1,    -1,
    1542,    -1,    -1,    -1,    -1,    -1,  1548,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1536,    -1,    -1,
      -1,    -1,   355,  1542,    -1,    -1,  1403,    -1,    -1,  1548,
      -1,   364,    -1,    -1,    -1,    -1,  1687,    -1,   371,   108,
      -1,    -1,   879,    -1,   113,   378,   115,   116,   117,   118,
     119,   120,   121,    -1,    -1,    -1,   389,   894,   895,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,   686,    -1,    -1,  1728,  1729,    -1,
     413,   918,    -1,   416,  1735,    66,    -1,    -1,    -1,   158,
     159,    -1,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   355,    -1,    -1,    -1,    -1,    -1,    -1,  1687,    -1,
     364,    -1,    -1,   182,  1672,  1657,  1674,   371,    -1,    -1,
      -1,    -1,  1773,    -1,   378,  1683,   459,    -1,    -1,  1844,
    1781,    -1,   744,    -1,    -1,   204,  1678,  1852,    -1,    -1,
    1682,    -1,    -1,  1858,    -1,  1687,  1861,    -1,    -1,  1728,
    1729,    -1,    -1,    -1,  1696,    -1,  1735,    -1,    -1,    -1,
      -1,  1703,  1704,    -1,    -1,  1707,  1708,    -1,  1687,   502,
      -1,    -1,  1730,   785,    -1,   787,  1013,  1014,  1787,  1721,
      -1,    -1,    -1,  1560,    -1,    -1,  1728,  1729,    -1,  1840,
      -1,    -1,    -1,  1735,  1773,    -1,  1847,  1806,    -1,    -1,
     812,    -1,    -1,     4,    -1,  1814,    -1,    -1,    -1,  1728,
    1729,    -1,  1049,    -1,    -1,    -1,  1735,    -1,    -1,    -1,
     553,   554,     4,    -1,   557,    -1,    -1,    -1,    -1,    -1,
      -1,  1773,    -1,    -1,    -1,    -1,    -1,    -1,  1780,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    -1,    -1,   502,    -1,
      -1,    -1,    -1,    -1,  1773,    -1,  1798,    -1,    -1,    -1,
      -1,  1840,    -1,    -1,    46,    -1,    -1,   879,  1847,    -1,
      -1,     4,    -1,    -1,  1832,    -1,   609,    -1,    -1,    -1,
      -1,  1839,   894,   895,  1661,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1840,    -1,
      -1,    -1,    -1,    -1,    -1,  1847,    -1,   108,    -1,    -1,
    1147,    -1,   113,    46,   115,   116,   117,   118,   119,   120,
     121,  1840,    -1,   656,   657,    -1,   108,    -1,  1847,    -1,
      -1,   113,   665,   115,   116,   117,   118,   119,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1187,    -1,    -1,   686,    -1,  1192,    -1,   158,   159,    -1,
     161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1745,  1206,
    1207,    -1,    -1,    -1,    -1,   108,   158,   159,    -1,   161,
     113,   182,   115,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,  1013,  1014,   204,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   744,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   204,    -1,    -1,   158,   159,    -1,   161,    -1,
      -1,    -1,   686,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1819,    -1,    -1,    -1,    -1,    -1,    -1,   182,
    1287,    -1,   785,    -1,   787,    -1,    -1,  1834,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   812,
     813,    -1,    -1,    26,    27,    -1,    -1,    30,    -1,    -1,
     744,    -1,    -1,   826,   827,   828,   829,   830,   831,   832,
      -1,    -1,    -1,   836,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   862,
      -1,   785,    -1,   787,    -1,  1147,    -1,    -1,  1375,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   879,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    65,    -1,    -1,   812,   892,
      -1,   894,   895,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   908,   909,    -1,    -1,    -1,
    1192,    -1,    -1,    -1,    -1,   918,    -1,    -1,    -1,    -1,
      -1,   924,    -1,    -1,  1206,  1207,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   937,    -1,    -1,    -1,  1445,    -1,
      -1,    -1,   945,    -1,    -1,   948,    -1,    -1,    -1,    -1,
     130,   131,    -1,    -1,    -1,   879,    10,    11,    12,    -1,
      -1,    -1,    -1,   966,    -1,    -1,    -1,   970,    -1,    -1,
     894,   895,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,  1287,    -1,    -1,    -1,    -1,
    1013,  1014,    66,   226,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   201,    -1,    -1,  1027,    -1,    -1,  1030,    -1,  1032,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,  1047,    -1,  1049,    -1,    -1,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,    -1,    -1,    -1,
      64,    65,    10,    11,    12,    -1,    -1,    -1,    -1,  1013,
    1014,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,  1105,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    54,    -1,
      -1,    -1,  1135,    -1,  1137,    -1,    -1,    -1,    66,    -1,
      -1,    -1,   355,    -1,  1147,    -1,   130,   131,    78,    -1,
      80,   364,    -1,   207,    -1,    -1,    -1,    -1,   371,    -1,
      -1,  1164,    -1,    -1,  1167,   378,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,   389,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1187,    -1,    -1,    -1,    -1,  1192,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1206,  1207,    -1,  1209,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1218,    -1,   201,    -1,  1222,
      -1,    -1,  1225,  1147,  1227,    -1,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
    1243,    -1,    -1,    -1,    -1,    -1,   459,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,  1192,    10,
      11,    12,    -1,   203,    -1,   205,   204,    -1,  1281,  1282,
      -1,  1284,  1206,  1207,  1287,    -1,    -1,    -1,    29,   502,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,   557,    -1,    -1,   273,    -1,   275,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1287,  1367,    -1,  1369,    -1,    -1,    -1,
      -1,  1374,  1375,    -1,    -1,  1378,    -1,  1380,    64,    65,
    1383,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1392,
    1393,    -1,    -1,  1396,    -1,    -1,   609,    -1,    -1,    -1,
    1403,    -1,    -1,    29,   330,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,  1445,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1460,    -1,    -1,
      -1,    -1,    -1,   204,    -1,    -1,    -1,  1470,  1471,    -1,
      -1,    -1,    -1,   686,    -1,  1478,    -1,  1480,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   413,    -1,    -1,
     416,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1502,
      -1,  1504,    -1,    -1,    -1,    -1,    -1,  1510,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,   744,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1545,  1546,    -1,    -1,    -1,    -1,    99,  1552,
      -1,  1554,    -1,    -1,    -1,    -1,    -1,  1560,    -1,  1562,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,    -1,   785,    -1,   787,    -1,    -1,    -1,   204,    -1,
      -1,    -1,    -1,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   812,
     813,   152,    -1,    -1,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    29,   828,   829,   830,   831,   832,
      -1,    -1,    -1,   836,    -1,    -1,    -1,   553,   554,    -1,
      -1,   557,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    -1,   862,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1661,    -1,
      -1,    -1,    -1,    78,    -1,    -1,   879,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,  1678,    -1,    -1,    -1,   892,
      -1,   894,   895,   609,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1696,    -1,    -1,   909,    -1,    -1,  1702,
      -1,    -1,    -1,    -1,    -1,   918,    -1,    -1,    -1,    -1,
    1713,    -1,    78,    -1,    -1,    -1,  1719,    -1,    -1,    -1,
    1723,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     656,   657,   945,    99,    -1,   948,    -1,   152,    -1,   665,
     155,    -1,  1745,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   970,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,  1785,    -1,    -1,    -1,   152,    -1,    -1,   155,
      -1,  1794,   158,   159,    -1,   161,   162,   163,    -1,    -1,
    1013,  1014,    -1,    -1,    -1,    -1,    -1,  1810,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1819,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,  1834,    -1,    -1,  1047,    -1,  1049,    -1,   204,  1052,
    1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,  1077,  1078,  1079,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   813,    -1,    -1,
      26,    27,  1105,    -1,    30,    -1,    -1,    -1,    -1,    -1,
     826,   827,   828,   829,   830,   831,   832,    -1,    -1,    -1,
     836,    -1,    -1,    -1,    10,    11,    12,    64,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,  1147,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,  1187,    -1,    -1,    -1,    -1,  1192,
      -1,    -1,   908,   130,   131,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1206,  1207,    -1,  1209,    -1,   924,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   937,  1225,    -1,  1227,    -1,    -1,    -1,    -1,   945,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1243,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     966,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,  1284,    -1,    -1,  1287,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     226,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1027,    -1,    -1,  1030,    -1,  1032,    -1,   204,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1047,    -1,    -1,    -1,    -1,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1374,  1375,    -1,    -1,  1378,    -1,  1380,    -1,    -1,
    1383,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1105,
    1393,    -1,    -1,  1396,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1135,
      -1,  1137,   557,    -1,   204,    -1,    -1,    -1,    99,   355,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   364,    -1,
      -1,    -1,  1445,    -1,    -1,   371,    -1,    -1,  1164,    -1,
      -1,  1167,   378,    -1,    -1,    -1,    -1,  1460,    -1,    -1,
      -1,    -1,    -1,   389,    -1,    -1,    -1,  1470,  1471,    -1,
      -1,    -1,    -1,    -1,   609,    -1,    -1,  1480,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,   158,   159,    -1,
     161,   162,   163,  1209,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1218,    -1,    -1,    -1,  1222,    -1,    -1,  1225,
      -1,  1227,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,  1243,    -1,    -1,
      -1,    -1,   203,   459,   205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1545,  1546,    -1,    -1,    -1,    -1,    -1,  1552,
      -1,  1554,    -1,    -1,    -1,    -1,    10,    11,    12,  1562,
      -1,    -1,    -1,    -1,    -1,  1281,  1282,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   502,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   557,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,  1367,    -1,  1369,    -1,    -1,    -1,    -1,  1374,    -1,
      -1,    -1,  1378,    -1,  1380,    -1,    -1,  1383,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1392,    -1,   813,    -1,
      -1,    -1,    -1,   609,    -1,    -1,    -1,  1403,    -1,    -1,
      -1,    -1,    -1,   828,   829,   830,   831,   832,    -1,  1702,
     155,   836,    -1,   158,   159,    -1,   161,   162,   163,    -1,
    1713,    -1,    -1,    -1,    -1,    -1,  1719,    -1,    -1,    -1,
    1723,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,  1460,    -1,    -1,    -1,   203,    -1,
     204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     686,    -1,  1478,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,  1785,    54,    -1,    -1,  1502,    -1,  1504,    -1,
      -1,  1794,    -1,    -1,  1510,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1810,    -1,    -1,
     945,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   744,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1545,
    1546,    -1,    -1,    -1,    -1,    -1,  1552,    -1,   557,    -1,
      -1,    10,    11,    12,  1560,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   785,
      29,   787,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,   812,   813,    -1,    -1,
     609,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,   828,   829,   830,   831,   832,    -1,    -1,    -1,
     836,    -1,  1047,    -1,    -1,    -1,    -1,  1052,  1053,  1054,
    1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1661,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   879,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1678,    -1,    -1,    -1,    -1,    -1,   894,   895,
    1105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1696,    -1,    -1,    -1,    -1,    -1,  1702,    -1,    -1,    -1,
      -1,    -1,   918,    -1,    -1,    -1,    -1,  1713,    -1,    -1,
      -1,    -1,    -1,  1719,    -1,    -1,    -1,  1723,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   945,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1745,
      -1,    -1,    -1,    -1,    -1,   204,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   970,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,  1785,
      -1,    -1,    -1,    -1,  1209,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,  1013,  1014,    -1,
    1225,    -1,  1227,    -1,   813,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1819,    -1,    -1,    -1,    -1,  1243,   828,
     829,   830,   831,    -1,    -1,    -1,    -1,   836,  1834,    -1,
      -1,  1047,    -1,  1049,    -1,    -1,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,  1078,  1079,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1105,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,   204,    -1,
      -1,  1147,    -1,    -1,    66,    -1,   945,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1374,
      -1,    -1,    -1,  1378,    -1,  1380,    -1,    -1,  1383,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1187,    -1,    -1,    -1,    -1,  1192,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1206,  1207,    -1,  1209,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1225,
      -1,  1227,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1243,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1460,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1052,  1053,  1054,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,
    1079,  1287,   204,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    -1,  1105,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1545,  1546,    -1,    -1,    -1,    -1,    29,  1552,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    10,    11,    12,    -1,    -1,    -1,  1374,  1375,
      -1,    -1,  1378,    66,  1380,    -1,    -1,  1383,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
    1209,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1225,    -1,  1227,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1445,
      -1,    -1,    -1,    -1,  1243,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,  1460,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1702,    -1,    -1,
      66,    10,    11,    12,    -1,    -1,    -1,    -1,  1713,    -1,
      -1,    -1,    -1,    -1,  1719,    -1,    -1,    -1,  1723,    -1,
      29,   204,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,  1545,
    1546,    -1,    -1,    -1,    -1,    -1,  1552,    66,    -1,    -1,
      -1,    -1,    -1,    -1,   202,  1561,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
    1785,    -1,    -1,    -1,    -1,  1374,    -1,    -1,    -1,  1378,
      -1,  1380,    -1,    29,  1383,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,   202,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1460,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,   202,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1702,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1713,    -1,    -1,
      -1,    -1,    -1,  1719,    -1,    -1,    -1,  1723,    -1,    -1,
      64,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,  1747,    -1,    -1,    -1,    -1,  1545,  1546,    -1,    -1,
      -1,    -1,    -1,  1552,    -1,    29,   202,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,  1785,
      54,    -1,    -1,    10,    11,    12,   130,   131,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   202,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    66,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,    -1,
      -1,    -1,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      71,    -1,    -1,  1702,    75,    76,    77,    78,    79,    80,
      -1,    82,    83,    -1,  1713,    -1,    87,    88,    89,    90,
    1719,    92,    -1,    94,  1723,    96,    -1,   201,    99,   100,
      -1,    -1,    -1,   104,   105,   106,   107,   108,   109,   110,
      -1,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,   122,   123,   124,   125,   126,   127,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,   144,    -1,   202,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,  1785,   158,   159,    -1,
     161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,   170,
      -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,    -1,
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
     104,   105,   106,   107,   108,   109,   110,    -1,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,   122,   123,
     124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
     144,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
     164,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,
      -1,    -1,   176,   177,    -1,   179,    -1,   181,   182,    -1,
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
     107,   108,   109,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,   122,   123,   124,   125,   126,
     127,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,   164,    -1,    -1,
     167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,
     177,    -1,   179,    -1,   181,   182,    -1,   184,   185,   186,
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
      -1,    96,    97,    -1,    99,   100,    -1,    -1,    -1,   104,
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
      94,    95,    96,    -1,    -1,    99,   100,    -1,    -1,    -1,
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
      93,    94,    -1,    96,    -1,    -1,    99,   100,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,
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
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   200,    -1,   202,   203,
      -1,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
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
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
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
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,
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
     176,    -1,    -1,    -1,    -1,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,
     206,    -1,   208,   209,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   200,    -1,   202,    -1,    -1,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    47,    48,    -1,    -1,    -1,
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
     162,   163,    -1,   165,    -1,   167,    -1,    -1,   170,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   205,   206,    -1,   208,   209,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,    -1,   203,    -1,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    35,    -1,    -1,
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
     158,   159,    -1,   161,   162,   163,    -1,   165,    -1,   167,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
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
      -1,    10,    11,    12,   205,   206,    -1,   208,   209,     3,
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
     104,    -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   115,   116,   117,   118,   119,   120,    -1,    -1,   123,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,    -1,
      -1,    -1,   176,   192,   193,    -1,    -1,   181,   182,    -1,
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
     197,    -1,    -1,   200,    -1,    10,    11,    12,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
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
     170,    -1,    -1,    -1,   189,    -1,   176,    -1,    -1,    -1,
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,   202,    11,    12,   205,   206,    -1,   208,   209,
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
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,   202,
      -1,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   200,   201,    -1,    -1,    -1,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,
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
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   205,   206,    -1,   208,   209,     3,     4,
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
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    47,
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
      -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,
     208,   209,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    47,    48,    -1,    -1,
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
     194,   195,   196,   197,    -1,    -1,   200,    -1,    10,    11,
      12,   205,   206,    -1,   208,   209,     3,     4,     5,     6,
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
     167,    -1,    -1,   170,    -1,   187,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,    10,    11,    12,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    29,    30,    31,    32,    33,    34,
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
      -1,   181,   182,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,   206,    -1,   208,   209,
       3,     4,    -1,     6,     7,    11,    12,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    27,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,   126,   127,   128,   129,    -1,    -1,    -1,
     133,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,   152,
      10,    11,    12,    13,    -1,   158,   159,    -1,   161,   162,
     163,   164,    -1,   166,    -1,    -1,   169,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    54,    -1,    -1,    -1,    -1,    -1,
     203,    -1,   205,    -1,    -1,    -1,    -1,    -1,    68,    69,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,   126,   127,   128,   129,
      -1,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,   152,    10,    11,    12,    13,    -1,   158,   159,
      -1,   161,   162,   163,   164,    -1,   166,    -1,    -1,   169,
      27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    54,    -1,    56,
      -1,    -1,    -1,   203,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    68,    69,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,
     127,   128,   129,    78,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,   152,    -1,    -1,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,   164,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,     3,     4,   200,     6,     7,    -1,   204,    10,    11,
      12,    13,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    54,    -1,    56,   200,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,    -1,   127,   128,   129,    78,    -1,
      -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
     152,    -1,    -1,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,   164,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,     3,     4,   200,     6,
       7,    -1,   204,    10,    11,    12,    13,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    54,    -1,    56,
     200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    69,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,   126,
     127,   128,   129,    78,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,   152,    -1,    -1,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,   164,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,     3,     4,   200,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,    -1,   127,   128,   129,    -1,    -1,
      -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     152,    -1,    -1,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,   164,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,    -1,   176,   177,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,     3,     4,   200,     6,
       7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    29,    29,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    68,    69,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,
     127,   128,   129,    -1,    -1,    78,   133,   134,   135,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    99,    -1,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,   164,    -1,   166,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,   124,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,    -1,   200,     3,     4,     5,     6,     7,    -1,
      -1,    10,    11,    12,    13,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    54,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      69,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
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
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    68,    69,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
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
     122,    -1,    -1,    -1,   126,   127,   128,   129,    -1,    -1,
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
     119,   120,   121,   122,    -1,    -1,    -1,    -1,   127,   128,
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
      46,    47,    48,    49,    50,    51,    52,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    29,    -1,    -1,    -1,   152,   107,    -1,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
      -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,    56,   176,
      -1,    -1,    -1,   134,   135,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      78,   152,    -1,   200,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    99,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    56,    -1,    -1,   200,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,   165,    -1,    99,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    56,
      -1,    -1,   200,    -1,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    78,   152,    -1,    -1,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,   165,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    30,    -1,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      47,    48,    -1,   200,    -1,    -1,    53,    -1,    55,    -1,
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
      -1,   158,   159,    -1,   161,   162,   163,    -1,    35,    -1,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      67,    -1,    -1,   200,   132,    -1,    -1,    -1,    75,    76,
      77,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    47,    48,   200,    -1,    -1,    -1,    53,   205,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    99,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    66,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    47,    48,    -1,    -1,   181,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    67,    -1,    -1,   200,   132,    -1,    -1,    -1,    75,
      76,    77,    78,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    99,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,   134,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    67,    -1,    -1,    -1,   132,    -1,    -1,    -1,    75,
      76,    77,    78,   156,    80,   158,   159,   160,   161,   162,
     163,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   120,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    67,    -1,   200,    -1,    -1,   203,    -1,   205,
      75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    67,   200,    69,    -1,    -1,    -1,
     205,    -1,    75,    76,    77,    78,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    67,    -1,   200,    -1,    -1,
      -1,    -1,   205,    75,    76,    77,    78,    -1,    80,    -1,
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
     192,   193,   194,   195,   196,    -1,    67,    -1,   200,    -1,
      -1,    -1,    -1,   205,    75,    76,    77,    78,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
      -1,    -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    78,    -1,    -1,    -1,   147,    99,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    99,    -1,    -1,   167,    -1,    -1,    -1,
      -1,   107,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,    -1,   200,
      75,    76,    77,    78,   205,    80,   158,   159,    -1,   161,
     162,   163,    87,    -1,    -1,    -1,    -1,    -1,    -1,   155,
      -1,    -1,   158,   159,    99,   161,   162,   163,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   120,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   136,   137,   138,   139,   140,    -1,    -1,    78,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    99,
      -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,   200,    75,    76,    77,    78,
     205,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,   152,    -1,    -1,   155,   156,    -1,   158,   159,
      99,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   120,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    10,    11,
      12,   200,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     132,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   132,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   132,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   132,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    78,    -1,    80,    81,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    -1,    -1,   152,    -1,    99,   155,   156,   132,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   156,    78,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,    -1,   155,    -1,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    99,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,   155,    -1,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   155,    -1,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    28,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    98,    54,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    66,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66
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
     450,   451,   452,   453,   454,   456,   469,   471,   473,   118,
     119,   120,   133,   152,   162,   200,   217,   250,   329,   348,
     445,   348,   200,   348,   348,   348,   104,   348,   348,   431,
     432,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,    80,    87,   120,   147,   200,   227,   367,
     403,   406,   411,   445,   448,   445,    35,   348,   460,   461,
     348,   120,   200,   227,   403,   404,   405,   407,   411,   442,
     443,   444,   452,   457,   458,   200,   339,   408,   200,   339,
     355,   340,   348,   236,   339,   200,   200,   200,   339,   202,
     348,   217,   202,   348,     3,     4,     6,     7,    10,    11,
      12,    13,    27,    29,    54,    56,    68,    69,    70,    71,
      72,    73,    74,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   126,   127,   128,
     129,   133,   134,   135,   152,   156,   164,   166,   169,   176,
     200,   217,   218,   219,   230,   474,   489,   490,   492,   183,
     202,   345,   348,   372,   374,   203,   243,   348,   107,   108,
     155,   220,   223,   226,    80,   205,   295,   296,   119,   126,
     118,   126,    80,   297,   200,   200,   200,   200,   217,   267,
     477,   200,   200,   340,    80,    86,   148,   149,   150,   466,
     467,   155,   203,   226,   226,   217,   268,   477,   156,   200,
     477,   477,    80,   197,   203,   357,    27,   338,   342,   348,
     349,   445,   449,   232,   203,    86,   409,   466,    86,   466,
     466,    30,   155,   172,   478,   200,     9,   202,    35,   249,
     156,   266,   477,   120,   182,   250,   330,   202,   202,   202,
     202,   202,   202,    10,    11,    12,    29,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    54,
      66,   202,    67,    67,   202,   203,   151,   127,   162,   164,
     177,   179,   269,   328,   329,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    64,    65,
     130,   131,   435,    67,   203,   440,   200,   200,    67,   203,
     205,   453,   200,   249,   250,    14,   348,   202,   132,    45,
     217,   430,    86,   338,   349,   151,   445,   132,   207,     9,
     416,   338,   349,   445,   478,   151,   200,   410,   435,   440,
     201,   348,    30,   234,     8,   360,     9,   202,   234,   235,
     340,   341,   348,   217,   281,   238,   202,   202,   202,   134,
     135,   492,   492,   172,   200,   107,   492,    14,   151,   134,
     135,   152,   217,   219,    80,   202,   202,   202,   183,   184,
     185,   190,   191,   194,   195,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   387,   388,   389,   244,   111,   169,
     202,   155,   221,   224,   226,   155,   222,   225,   226,   226,
       9,   202,    98,   203,   445,     9,   202,   126,   126,    14,
       9,   202,   445,   470,   340,   338,   349,   445,   448,   449,
     201,   172,   261,   133,   445,   459,   460,   202,    67,   435,
     148,   467,    79,   348,   445,    86,   148,   467,   226,   216,
     202,   203,   256,   264,   393,   395,    87,   200,   205,   361,
     362,   364,   406,   451,   453,   471,    14,    98,   472,   356,
     358,   359,   291,   292,   433,   434,   201,   201,   201,   201,
     201,   204,   233,   234,   251,   258,   263,   433,   348,   206,
     208,   209,   217,   479,   480,   492,    35,   165,   293,   294,
     348,   474,   200,   477,   259,   249,   348,   348,   348,    30,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   407,   348,   348,   455,   455,   348,   462,
     463,   126,   203,   218,   219,   452,   453,   267,   217,   268,
     477,   477,   266,   250,    35,   342,   345,   348,   372,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   156,   203,   217,   436,   437,   438,   439,   452,   455,
     348,   293,   293,   455,   348,   459,   249,   201,   348,   200,
     429,     9,   416,   201,   201,    35,   348,    35,   348,   201,
     201,   201,   452,   293,   203,   217,   436,   437,   452,   201,
     232,   285,   203,   345,   348,   348,    90,    30,   234,   279,
     202,    28,    98,    14,     9,   201,    30,   203,   282,   492,
      29,    87,   230,   486,   487,   488,   200,     9,    47,    48,
      53,    55,    67,   134,   156,   176,   200,   227,   228,   230,
     369,   403,   411,   412,   413,   217,   491,   186,    80,   348,
      80,    80,   348,   384,   385,   348,   348,   377,   387,   189,
     390,   232,   200,   242,   226,   202,     9,    98,   226,   202,
       9,    98,    98,   223,   217,   348,   296,   412,    80,     9,
     201,   201,   201,   201,   201,   201,   201,   202,    47,    48,
     484,   485,   128,   272,   200,     9,   201,   201,    80,    81,
     217,   468,   217,    67,   204,   204,   213,   215,    30,   129,
     271,   171,    51,   156,   171,   397,   349,   132,     9,   416,
     201,   151,   492,   492,    14,   360,   291,   232,   198,     9,
     417,   492,   493,   435,   440,   435,   204,     9,   416,   173,
     445,   348,   201,     9,   417,    14,   352,   252,   128,   270,
     200,   477,   348,    30,   207,   207,   132,   204,     9,   416,
     348,   478,   200,   262,   257,   265,    14,   472,   260,   249,
      69,   445,   348,   478,   207,   204,   201,   201,   207,   204,
     201,    47,    48,    67,    75,    76,    77,    87,   134,   147,
     176,   217,   419,   421,   422,   425,   428,   217,   445,   445,
     132,   435,   440,   201,   348,   286,    72,    73,   287,   232,
     339,   232,   341,    98,    35,   133,   276,   445,   412,   217,
      30,   234,   280,   202,   283,   202,   283,     9,   173,    87,
     132,   151,     9,   416,   201,   165,   479,   480,   481,   479,
     412,   412,   412,   412,   412,   415,   418,   200,   151,   200,
     412,   151,   203,    10,    11,    12,    29,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    66,
     151,   478,   348,   186,   186,    14,   192,   193,   386,     9,
     196,   390,    80,   204,   403,   203,   246,    98,   224,   217,
      98,   225,   217,   217,   204,    14,   445,   202,     9,   173,
     217,   273,   403,   203,   459,   133,   445,    14,   207,   348,
     204,   213,   492,   273,   203,   396,    14,   201,   348,   361,
     452,   202,   492,   198,   204,    30,   482,   434,    35,    80,
     165,   436,   437,   439,   436,   437,   492,    35,   165,   348,
     412,   291,   200,   403,   271,   353,   253,   348,   348,   348,
     204,   200,   293,   272,    30,   271,   492,    14,   270,   477,
     407,   204,   200,    14,    75,    76,    77,   217,   420,   420,
     422,   423,   424,    49,   200,    86,   148,   200,     9,   416,
     201,   429,    35,   348,   204,    72,    73,   288,   339,   234,
     204,   202,    91,   202,   276,   445,   200,   132,   275,    14,
     232,   283,   101,   102,   103,   283,   204,   492,   132,   492,
     217,   486,     9,   201,   416,   132,   207,     9,   416,   415,
     218,   361,   363,   365,   201,   126,   218,   412,   464,   465,
     412,   412,   412,    30,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   491,   348,
     348,   348,   385,   348,   375,    80,   247,   217,   217,   412,
     485,    98,    99,   483,     9,   301,   201,   200,   342,   345,
     348,   207,   204,   472,   301,   157,   170,   203,   392,   399,
     157,   203,   398,   132,   202,   482,   492,   360,   493,    80,
     165,    14,    80,   478,   445,   348,   201,   291,   203,   291,
     200,   132,   200,   293,   201,   203,   492,   203,   202,   492,
     271,   254,   410,   293,   132,   207,     9,   416,   421,   423,
     148,   361,   426,   427,   422,   445,   339,    30,    74,   234,
     202,   341,   275,   459,   276,   201,   412,    97,   101,   202,
     348,    30,   202,   284,   204,   173,   492,   132,   165,    30,
     201,   412,   412,   201,   132,     9,   416,   201,   132,   204,
       9,   416,   412,    30,   187,   201,   232,   217,   492,   492,
     403,     4,   108,   113,   119,   121,   158,   159,   161,   204,
     302,   327,   328,   329,   334,   335,   336,   337,   433,   459,
     204,   203,   204,    51,   348,   348,   348,   360,    35,    80,
     165,    14,    80,   348,   200,   482,   201,   301,   201,   291,
     348,   293,   201,   301,   472,   301,   202,   203,   200,   201,
     422,   422,   201,   132,   201,     9,   416,    30,   232,   202,
     201,   201,   201,   239,   202,   202,   284,   232,   492,   492,
     132,   412,   361,   412,   412,   412,   348,   203,   204,   483,
     128,   129,   177,   218,   475,   492,   274,   403,   108,   337,
      29,   121,   134,   135,   156,   162,   311,   312,   313,   314,
     403,   160,   319,   320,   124,   200,   217,   321,   322,   303,
     250,   492,     9,   202,     9,   202,   202,   472,   328,   201,
     298,   156,   394,   204,   204,    80,   165,    14,    80,   348,
     293,   113,   350,   482,   204,   482,   201,   201,   204,   203,
     204,   301,   291,   132,   422,   361,   232,   237,   240,    30,
     234,   278,   232,   201,   412,   132,   132,   188,   232,   403,
     403,   477,    14,   218,     9,   202,   203,   475,   472,   314,
     172,   203,     9,   202,     3,     4,     5,     6,     7,    10,
      11,    12,    13,    27,    28,    54,    68,    69,    70,    71,
      72,    73,    74,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   133,   134,   136,   137,   138,   139,
     140,   152,   153,   154,   164,   166,   167,   169,   176,   177,
     179,   181,   182,   217,   400,   401,     9,   202,   156,   160,
     217,   322,   323,   324,   202,    80,   333,   249,   304,   475,
     475,    14,   250,   204,   299,   300,   475,    14,    80,   348,
     201,   200,   203,   202,   203,   325,   350,   482,   298,   204,
     201,   422,   132,    30,   234,   277,   278,   232,   412,   412,
     348,   204,   202,   202,   412,   403,   307,   492,   315,   316,
     411,   312,    14,    30,    48,   317,   320,     9,    33,   201,
      29,    47,    50,    14,     9,   202,   219,   476,   333,    14,
     492,   249,   202,    14,   348,    35,    80,   391,   232,   232,
     203,   325,   204,   482,   422,   232,    95,   189,   245,   204,
     217,   230,   308,   309,   310,     9,   173,     9,   416,   204,
     412,   401,   401,    56,   318,   323,   323,    29,    47,    50,
     412,    80,   172,   200,   202,   412,   477,   412,    80,     9,
     417,   204,   204,   232,   325,    93,   202,    80,   111,   241,
     151,    98,   492,   411,   163,    14,   484,   305,   200,    35,
      80,   201,   204,   202,   200,   169,   248,   217,   328,   329,
     173,   412,   173,   289,   290,   434,   306,    80,   403,   246,
     166,   217,   202,   201,     9,   417,   115,   116,   117,   331,
     332,   289,    80,   274,   202,   482,   434,   493,   201,   201,
     202,   202,   203,   326,   331,    35,    80,   165,   482,   203,
     232,   493,    80,   165,    14,    80,   326,   232,   204,    35,
      80,   165,    14,    80,   348,   204,    80,   165,    14,    80,
     348,    14,    80,   348,   348
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

  case 106:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 886 "hphp.y"
    { ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 895 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 990 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1042 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1054 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1077 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { (yyval).reset();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1082 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1090 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1091 "hphp.y"
    { (yyval).reset();;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1095 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { (yyval).reset();;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1100 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1106 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1112 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1119 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1125 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1132 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1150 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1158 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1164 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1167 "hphp.y"
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

  case 211:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
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

  case 213:

/* Line 1455 of yacc.c  */
#line 1199 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1210 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1220 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval).reset();;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval).reset();;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyval).reset();;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1274 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { (yyval).reset();;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { (yyval).reset();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { (yyval).reset();;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { (yyval).reset();;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { (yyval).reset();;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { (yyval).reset();;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { (yyval).reset();;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { (yyval).reset();;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1363 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1375 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1383 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { (yyval).reset();;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1479 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { (yyval).reset();;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { (yyval).reset();;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { (yyval).reset();;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval).reset();;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval).reset();;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval).reset();;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval).reset();;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval).reset();;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval).reset();;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval).reset();;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 526:

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

  case 527:

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

  case 528:

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

  case 529:

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

  case 530:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 531:

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

  case 532:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 533:

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

  case 534:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 535:

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

  case 536:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 598:

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

  case 599:

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

  case 600:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval).reset();;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval).reset();;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 609:

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

  case 610:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { (yyval).reset();;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval).reset();;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { (yyval).reset();;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval).reset();;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { (yyval).reset();;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval).reset();;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval).reset();;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { (yyval).reset();;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { (yyval).reset();;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { (yyval).reset();;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2726 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { (yyval).reset();;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 853:

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

  case 854:

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

  case 855:

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

  case 856:

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

  case 857:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 863:

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

  case 864:

/* Line 1455 of yacc.c  */
#line 2874 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 872:

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

  case 873:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2905 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyval).reset();;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { (yyval)++;;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 907:

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

  case 908:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 913:

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

  case 914:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyval).reset();;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3079 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3093 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3096 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3112 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 949:

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

  case 950:

/* Line 1455 of yacc.c  */
#line 3124 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3136 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3142 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3150 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3156 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3172 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3177 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3181 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3186 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3198 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3204 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3208 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3218 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3225 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3226 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3230 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3233 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3247 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3251 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 995:

/* Line 1455 of yacc.c  */
#line 3252 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3262 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3264 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3268 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3271 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3275 "hphp.y"
    {;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3276 "hphp.y"
    {;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3277 "hphp.y"
    {;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3283 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1006:

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

  case 1007:

/* Line 1455 of yacc.c  */
#line 3299 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3304 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3305 "hphp.y"
    { ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3310 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3311 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3317 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3327 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3331 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3338 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3341 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3345 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3348 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3354 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3358 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3361 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3364 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3370 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1027:

/* Line 1455 of yacc.c  */
#line 3376 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1028:

/* Line 1455 of yacc.c  */
#line 3384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1029:

/* Line 1455 of yacc.c  */
#line 3385 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14571 "hphp.5.tab.cpp"
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
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}

