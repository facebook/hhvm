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
#define YYLAST   16676

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
     261,   266,   268,   272,   274,   278,   281,   283,   286,   289,
     295,   300,   303,   304,   306,   308,   310,   312,   316,   322,
     331,   332,   337,   338,   345,   346,   357,   358,   363,   366,
     370,   373,   377,   380,   384,   388,   392,   396,   400,   404,
     410,   412,   414,   416,   417,   427,   428,   439,   445,   446,
     460,   461,   467,   471,   475,   478,   481,   484,   487,   490,
     493,   497,   500,   503,   504,   509,   519,   520,   521,   526,
     529,   530,   532,   533,   535,   536,   546,   547,   558,   559,
     571,   572,   582,   583,   594,   595,   604,   605,   615,   616,
     624,   625,   634,   635,   644,   645,   653,   654,   663,   665,
     667,   669,   671,   673,   676,   680,   684,   687,   690,   691,
     694,   695,   698,   699,   701,   705,   707,   711,   714,   715,
     717,   720,   725,   727,   732,   734,   739,   741,   746,   748,
     753,   757,   763,   767,   772,   777,   783,   789,   794,   795,
     797,   799,   804,   805,   811,   812,   815,   816,   820,   821,
     829,   838,   845,   848,   854,   861,   866,   867,   872,   878,
     886,   893,   900,   908,   918,   927,   934,   942,   948,   951,
     956,   962,   966,   967,   971,   976,   983,   989,   995,  1002,
    1011,  1019,  1022,  1023,  1025,  1028,  1031,  1035,  1040,  1045,
    1049,  1051,  1053,  1056,  1061,  1065,  1071,  1073,  1077,  1080,
    1081,  1084,  1088,  1091,  1092,  1093,  1098,  1099,  1105,  1108,
    1111,  1114,  1115,  1126,  1127,  1139,  1143,  1147,  1151,  1156,
    1161,  1165,  1171,  1174,  1177,  1178,  1185,  1191,  1196,  1200,
    1202,  1204,  1208,  1213,  1215,  1218,  1220,  1222,  1227,  1234,
    1236,  1238,  1243,  1245,  1247,  1251,  1254,  1257,  1258,  1261,
    1262,  1264,  1268,  1270,  1272,  1274,  1276,  1280,  1285,  1290,
    1295,  1297,  1299,  1302,  1305,  1308,  1312,  1316,  1318,  1320,
    1322,  1324,  1328,  1330,  1334,  1336,  1338,  1340,  1341,  1343,
    1346,  1348,  1350,  1352,  1354,  1356,  1358,  1360,  1362,  1363,
    1365,  1367,  1369,  1373,  1379,  1381,  1385,  1391,  1396,  1400,
    1404,  1408,  1413,  1417,  1421,  1425,  1428,  1431,  1433,  1435,
    1439,  1443,  1445,  1447,  1448,  1450,  1453,  1458,  1462,  1466,
    1473,  1476,  1480,  1487,  1489,  1491,  1493,  1495,  1497,  1504,
    1508,  1513,  1520,  1524,  1528,  1532,  1536,  1540,  1544,  1548,
    1552,  1556,  1560,  1564,  1568,  1571,  1574,  1577,  1580,  1584,
    1588,  1592,  1596,  1600,  1604,  1608,  1612,  1616,  1620,  1624,
    1628,  1632,  1636,  1640,  1644,  1648,  1651,  1654,  1657,  1660,
    1664,  1668,  1672,  1676,  1680,  1684,  1688,  1692,  1696,  1700,
    1704,  1710,  1715,  1717,  1720,  1723,  1726,  1729,  1732,  1735,
    1738,  1741,  1744,  1746,  1748,  1750,  1754,  1757,  1759,  1765,
    1766,  1767,  1779,  1780,  1793,  1794,  1799,  1800,  1808,  1809,
    1815,  1816,  1820,  1821,  1828,  1831,  1834,  1839,  1841,  1843,
    1849,  1853,  1859,  1863,  1866,  1867,  1870,  1871,  1876,  1881,
    1885,  1890,  1895,  1900,  1905,  1907,  1909,  1911,  1913,  1917,
    1921,  1926,  1928,  1931,  1936,  1939,  1946,  1947,  1949,  1954,
    1955,  1958,  1959,  1961,  1963,  1967,  1969,  1973,  1975,  1977,
    1981,  1985,  1987,  1989,  1991,  1993,  1995,  1997,  1999,  2001,
    2003,  2005,  2007,  2009,  2011,  2013,  2015,  2017,  2019,  2021,
    2023,  2025,  2027,  2029,  2031,  2033,  2035,  2037,  2039,  2041,
    2043,  2045,  2047,  2049,  2051,  2053,  2055,  2057,  2059,  2061,
    2063,  2065,  2067,  2069,  2071,  2073,  2075,  2077,  2079,  2081,
    2083,  2085,  2087,  2089,  2091,  2093,  2095,  2097,  2099,  2101,
    2103,  2105,  2107,  2109,  2111,  2113,  2115,  2117,  2119,  2121,
    2123,  2125,  2127,  2129,  2131,  2133,  2135,  2137,  2139,  2141,
    2143,  2145,  2150,  2152,  2154,  2156,  2158,  2160,  2162,  2166,
    2168,  2172,  2174,  2176,  2180,  2182,  2184,  2186,  2189,  2191,
    2192,  2193,  2195,  2197,  2201,  2202,  2204,  2206,  2208,  2210,
    2212,  2214,  2216,  2218,  2220,  2222,  2224,  2226,  2228,  2232,
    2235,  2237,  2239,  2244,  2248,  2253,  2255,  2257,  2261,  2265,
    2269,  2273,  2277,  2281,  2285,  2289,  2293,  2297,  2301,  2305,
    2309,  2313,  2317,  2321,  2325,  2329,  2332,  2335,  2338,  2341,
    2345,  2349,  2353,  2357,  2361,  2365,  2369,  2373,  2377,  2383,
    2388,  2392,  2396,  2400,  2402,  2404,  2406,  2408,  2412,  2416,
    2420,  2423,  2424,  2426,  2427,  2429,  2430,  2436,  2440,  2444,
    2446,  2448,  2450,  2452,  2456,  2459,  2461,  2463,  2465,  2467,
    2469,  2473,  2475,  2477,  2479,  2482,  2485,  2490,  2494,  2499,
    2502,  2503,  2509,  2513,  2517,  2519,  2523,  2525,  2528,  2529,
    2535,  2539,  2542,  2543,  2547,  2548,  2553,  2556,  2557,  2561,
    2565,  2567,  2568,  2570,  2572,  2574,  2576,  2580,  2582,  2584,
    2586,  2590,  2592,  2594,  2598,  2602,  2605,  2610,  2613,  2618,
    2624,  2630,  2636,  2642,  2644,  2646,  2648,  2650,  2652,  2654,
    2658,  2662,  2667,  2672,  2676,  2678,  2680,  2682,  2684,  2688,
    2690,  2695,  2699,  2703,  2705,  2707,  2709,  2711,  2713,  2717,
    2721,  2726,  2731,  2735,  2737,  2739,  2747,  2757,  2765,  2772,
    2781,  2783,  2788,  2793,  2795,  2797,  2802,  2805,  2807,  2808,
    2810,  2812,  2814,  2818,  2822,  2826,  2827,  2829,  2831,  2835,
    2839,  2842,  2846,  2853,  2854,  2856,  2861,  2864,  2865,  2871,
    2875,  2879,  2881,  2888,  2893,  2898,  2901,  2904,  2905,  2911,
    2915,  2919,  2921,  2924,  2925,  2931,  2935,  2939,  2941,  2944,
    2947,  2949,  2952,  2954,  2959,  2963,  2967,  2974,  2978,  2980,
    2982,  2984,  2989,  2994,  2999,  3004,  3009,  3014,  3017,  3020,
    3025,  3028,  3031,  3033,  3037,  3041,  3045,  3046,  3049,  3055,
    3062,  3069,  3077,  3079,  3082,  3084,  3087,  3089,  3094,  3096,
    3101,  3105,  3106,  3108,  3112,  3115,  3119,  3121,  3123,  3124,
    3125,  3128,  3131,  3134,  3139,  3142,  3148,  3152,  3154,  3156,
    3157,  3161,  3166,  3172,  3176,  3178,  3181,  3182,  3187,  3189,
    3193,  3196,  3199,  3202,  3204,  3206,  3208,  3210,  3214,  3219,
    3226,  3228,  3237,  3244,  3246
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     197,     0,    -1,    -1,   198,   199,    -1,   199,   200,    -1,
      -1,   220,    -1,   237,    -1,   244,    -1,   241,    -1,   251,
      -1,   441,    -1,   125,   186,   187,   188,    -1,   152,   212,
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
      -1,   213,    -1,   213,   446,    -1,   213,   446,    -1,   217,
       9,   442,    14,   381,    -1,   108,   442,    14,   381,    -1,
     218,   219,    -1,    -1,   220,    -1,   237,    -1,   244,    -1,
     251,    -1,   189,   218,   190,    -1,    71,   327,   220,   273,
     275,    -1,    71,   327,    30,   218,   274,   276,    74,   188,
      -1,    -1,    90,   327,   221,   267,    -1,    -1,    89,   222,
     220,    90,   327,   188,    -1,    -1,    92,   186,   329,   188,
     329,   188,   329,   187,   223,   265,    -1,    -1,   100,   327,
     224,   270,    -1,   104,   188,    -1,   104,   336,   188,    -1,
     106,   188,    -1,   106,   336,   188,    -1,   109,   188,    -1,
     109,   336,   188,    -1,    27,   104,   188,    -1,   114,   283,
     188,    -1,   120,   285,   188,    -1,    88,   328,   188,    -1,
     144,   328,   188,    -1,   122,   186,   438,   187,   188,    -1,
     188,    -1,    82,    -1,    83,    -1,    -1,    94,   186,   336,
      98,   264,   263,   187,   225,   266,    -1,    -1,    94,   186,
     336,    28,    98,   264,   263,   187,   226,   266,    -1,    96,
     186,   269,   187,   268,    -1,    -1,   110,   229,   111,   186,
     372,    80,   187,   189,   218,   190,   231,   227,   234,    -1,
      -1,   110,   229,   169,   228,   232,    -1,   112,   336,   188,
      -1,   105,   203,   188,    -1,   336,   188,    -1,   330,   188,
      -1,   331,   188,    -1,   332,   188,    -1,   333,   188,    -1,
     334,   188,    -1,   109,   333,   188,    -1,   335,   188,    -1,
     203,    30,    -1,    -1,   189,   230,   218,   190,    -1,   231,
     111,   186,   372,    80,   187,   189,   218,   190,    -1,    -1,
      -1,   189,   233,   218,   190,    -1,   169,   232,    -1,    -1,
      35,    -1,    -1,   107,    -1,    -1,   236,   235,   445,   238,
     186,   279,   187,   450,   313,    -1,    -1,   317,   236,   235,
     445,   239,   186,   279,   187,   450,   313,    -1,    -1,   402,
     316,   236,   235,   445,   240,   186,   279,   187,   450,   313,
      -1,    -1,   162,   203,   242,    30,   460,   440,   189,   286,
     190,    -1,    -1,   402,   162,   203,   243,    30,   460,   440,
     189,   286,   190,    -1,    -1,   257,   254,   245,   258,   259,
     189,   289,   190,    -1,    -1,   402,   257,   254,   246,   258,
     259,   189,   289,   190,    -1,    -1,   127,   255,   247,   260,
     189,   289,   190,    -1,    -1,   402,   127,   255,   248,   260,
     189,   289,   190,    -1,    -1,   126,   250,   379,   258,   259,
     189,   289,   190,    -1,    -1,   164,   256,   252,   259,   189,
     289,   190,    -1,    -1,   402,   164,   256,   253,   259,   189,
     289,   190,    -1,   445,    -1,   156,    -1,   445,    -1,   445,
      -1,   126,    -1,   119,   126,    -1,   119,   118,   126,    -1,
     118,   119,   126,    -1,   118,   126,    -1,   128,   372,    -1,
      -1,   129,   261,    -1,    -1,   128,   261,    -1,    -1,   372,
      -1,   261,     9,   372,    -1,   372,    -1,   262,     9,   372,
      -1,   132,   264,    -1,    -1,   414,    -1,    35,   414,    -1,
     133,   186,   427,   187,    -1,   220,    -1,    30,   218,    93,
     188,    -1,   220,    -1,    30,   218,    95,   188,    -1,   220,
      -1,    30,   218,    91,   188,    -1,   220,    -1,    30,   218,
      97,   188,    -1,   203,    14,   381,    -1,   269,     9,   203,
      14,   381,    -1,   189,   271,   190,    -1,   189,   188,   271,
     190,    -1,    30,   271,   101,   188,    -1,    30,   188,   271,
     101,   188,    -1,   271,   102,   336,   272,   218,    -1,   271,
     103,   272,   218,    -1,    -1,    30,    -1,   188,    -1,   273,
      72,   327,   220,    -1,    -1,   274,    72,   327,    30,   218,
      -1,    -1,    73,   220,    -1,    -1,    73,    30,   218,    -1,
      -1,   278,     9,   403,   319,   461,   165,    80,    -1,   278,
       9,   403,   319,   461,    35,   165,    80,    -1,   278,     9,
     403,   319,   461,   165,    -1,   278,   386,    -1,   403,   319,
     461,   165,    80,    -1,   403,   319,   461,    35,   165,    80,
      -1,   403,   319,   461,   165,    -1,    -1,   403,   319,   461,
      80,    -1,   403,   319,   461,    35,    80,    -1,   403,   319,
     461,    35,    80,    14,   336,    -1,   403,   319,   461,    80,
      14,   336,    -1,   278,     9,   403,   319,   461,    80,    -1,
     278,     9,   403,   319,   461,    35,    80,    -1,   278,     9,
     403,   319,   461,    35,    80,    14,   336,    -1,   278,     9,
     403,   319,   461,    80,    14,   336,    -1,   280,     9,   403,
     461,   165,    80,    -1,   280,     9,   403,   461,    35,   165,
      80,    -1,   280,     9,   403,   461,   165,    -1,   280,   386,
      -1,   403,   461,   165,    80,    -1,   403,   461,    35,   165,
      80,    -1,   403,   461,   165,    -1,    -1,   403,   461,    80,
      -1,   403,   461,    35,    80,    -1,   403,   461,    35,    80,
      14,   336,    -1,   403,   461,    80,    14,   336,    -1,   280,
       9,   403,   461,    80,    -1,   280,     9,   403,   461,    35,
      80,    -1,   280,     9,   403,   461,    35,    80,    14,   336,
      -1,   280,     9,   403,   461,    80,    14,   336,    -1,   282,
     386,    -1,    -1,   336,    -1,    35,   414,    -1,   165,   336,
      -1,   282,     9,   336,    -1,   282,     9,   165,   336,    -1,
     282,     9,    35,   414,    -1,   283,     9,   284,    -1,   284,
      -1,    80,    -1,   191,   414,    -1,   191,   189,   336,   190,
      -1,   285,     9,    80,    -1,   285,     9,    80,    14,   381,
      -1,    80,    -1,    80,    14,   381,    -1,   286,   287,    -1,
      -1,   288,   188,    -1,   443,    14,   381,    -1,   289,   290,
      -1,    -1,    -1,   315,   291,   321,   188,    -1,    -1,   317,
     460,   292,   321,   188,    -1,   322,   188,    -1,   323,   188,
      -1,   324,   188,    -1,    -1,   316,   236,   235,   444,   186,
     293,   277,   187,   450,   314,    -1,    -1,   402,   316,   236,
     235,   445,   186,   294,   277,   187,   450,   314,    -1,   158,
     299,   188,    -1,   159,   307,   188,    -1,   161,   309,   188,
      -1,     4,   128,   372,   188,    -1,     4,   129,   372,   188,
      -1,   113,   262,   188,    -1,   113,   262,   189,   295,   190,
      -1,   295,   296,    -1,   295,   297,    -1,    -1,   216,   151,
     203,   166,   262,   188,    -1,   298,    98,   316,   203,   188,
      -1,   298,    98,   317,   188,    -1,   216,   151,   203,    -1,
     203,    -1,   300,    -1,   299,     9,   300,    -1,   301,   369,
     305,   306,    -1,   156,    -1,    29,   302,    -1,   302,    -1,
     134,    -1,   134,   172,   460,   173,    -1,   134,   172,   460,
       9,   460,   173,    -1,   372,    -1,   121,    -1,   162,   189,
     304,   190,    -1,   135,    -1,   380,    -1,   303,     9,   380,
      -1,   303,   385,    -1,    14,   381,    -1,    -1,    56,   163,
      -1,    -1,   308,    -1,   307,     9,   308,    -1,   160,    -1,
     310,    -1,   203,    -1,   124,    -1,   186,   311,   187,    -1,
     186,   311,   187,    50,    -1,   186,   311,   187,    29,    -1,
     186,   311,   187,    47,    -1,   310,    -1,   312,    -1,   312,
      50,    -1,   312,    29,    -1,   312,    47,    -1,   311,     9,
     311,    -1,   311,    33,   311,    -1,   203,    -1,   156,    -1,
     160,    -1,   188,    -1,   189,   218,   190,    -1,   188,    -1,
     189,   218,   190,    -1,   317,    -1,   121,    -1,   317,    -1,
      -1,   318,    -1,   317,   318,    -1,   115,    -1,   116,    -1,
     117,    -1,   120,    -1,   119,    -1,   118,    -1,   182,    -1,
     320,    -1,    -1,   115,    -1,   116,    -1,   117,    -1,   321,
       9,    80,    -1,   321,     9,    80,    14,   381,    -1,    80,
      -1,    80,    14,   381,    -1,   322,     9,   443,    14,   381,
      -1,   108,   443,    14,   381,    -1,   323,     9,   443,    -1,
     119,   108,   443,    -1,   119,   325,   440,    -1,   325,   440,
      14,   460,    -1,   108,   177,   445,    -1,   186,   326,   187,
      -1,    69,   376,   379,    -1,    69,   249,    -1,    68,   336,
      -1,   361,    -1,   356,    -1,   186,   336,   187,    -1,   328,
       9,   336,    -1,   336,    -1,   328,    -1,    -1,    27,    -1,
      27,   336,    -1,    27,   336,   132,   336,    -1,   186,   330,
     187,    -1,   414,    14,   330,    -1,   133,   186,   427,   187,
      14,   330,    -1,    28,   336,    -1,   414,    14,   333,    -1,
     133,   186,   427,   187,    14,   333,    -1,   337,    -1,   414,
      -1,   326,    -1,   418,    -1,   417,    -1,   133,   186,   427,
     187,    14,   336,    -1,   414,    14,   336,    -1,   414,    14,
      35,   414,    -1,   414,    14,    35,    69,   376,   379,    -1,
     414,    26,   336,    -1,   414,    25,   336,    -1,   414,    24,
     336,    -1,   414,    23,   336,    -1,   414,    22,   336,    -1,
     414,    21,   336,    -1,   414,    20,   336,    -1,   414,    19,
     336,    -1,   414,    18,   336,    -1,   414,    17,   336,    -1,
     414,    16,   336,    -1,   414,    15,   336,    -1,   414,    65,
      -1,    65,   414,    -1,   414,    64,    -1,    64,   414,    -1,
     336,    31,   336,    -1,   336,    32,   336,    -1,   336,    10,
     336,    -1,   336,    12,   336,    -1,   336,    11,   336,    -1,
     336,    33,   336,    -1,   336,    35,   336,    -1,   336,    34,
     336,    -1,   336,    49,   336,    -1,   336,    47,   336,    -1,
     336,    48,   336,    -1,   336,    50,   336,    -1,   336,    51,
     336,    -1,   336,    66,   336,    -1,   336,    52,   336,    -1,
     336,    46,   336,    -1,   336,    45,   336,    -1,    47,   336,
      -1,    48,   336,    -1,    53,   336,    -1,    55,   336,    -1,
     336,    37,   336,    -1,   336,    36,   336,    -1,   336,    39,
     336,    -1,   336,    38,   336,    -1,   336,    40,   336,    -1,
     336,    44,   336,    -1,   336,    41,   336,    -1,   336,    43,
     336,    -1,   336,    42,   336,    -1,   336,    54,   376,    -1,
     186,   337,   187,    -1,   336,    29,   336,    30,   336,    -1,
     336,    29,    30,   336,    -1,   437,    -1,    63,   336,    -1,
      62,   336,    -1,    61,   336,    -1,    60,   336,    -1,    59,
     336,    -1,    58,   336,    -1,    57,   336,    -1,    70,   377,
      -1,    56,   336,    -1,   383,    -1,   355,    -1,   354,    -1,
     192,   378,   192,    -1,    13,   336,    -1,   358,    -1,   113,
     186,   360,   386,   187,    -1,    -1,    -1,   236,   235,   186,
     340,   279,   187,   450,   338,   189,   218,   190,    -1,    -1,
     317,   236,   235,   186,   341,   279,   187,   450,   338,   189,
     218,   190,    -1,    -1,   182,    80,   343,   348,    -1,    -1,
     182,   183,   344,   279,   184,   450,   348,    -1,    -1,   182,
     189,   345,   218,   190,    -1,    -1,    80,   346,   348,    -1,
      -1,   183,   347,   279,   184,   450,   348,    -1,     8,   336,
      -1,     8,   333,    -1,     8,   189,   218,   190,    -1,    87,
      -1,   439,    -1,   350,     9,   349,   132,   336,    -1,   349,
     132,   336,    -1,   351,     9,   349,   132,   381,    -1,   349,
     132,   381,    -1,   350,   385,    -1,    -1,   351,   385,    -1,
      -1,   176,   186,   352,   187,    -1,   134,   186,   428,   187,
      -1,    67,   428,   193,    -1,   372,   189,   430,   190,    -1,
     372,   189,   432,   190,    -1,   358,    67,   424,   193,    -1,
     359,    67,   424,   193,    -1,   355,    -1,   439,    -1,   417,
      -1,    87,    -1,   186,   337,   187,    -1,   360,     9,    80,
      -1,   360,     9,    35,    80,    -1,    80,    -1,    35,    80,
      -1,   170,   156,   362,   171,    -1,   364,    51,    -1,   364,
     171,   365,   170,    51,   363,    -1,    -1,   156,    -1,   364,
     366,    14,   367,    -1,    -1,   365,   368,    -1,    -1,   156,
      -1,   157,    -1,   189,   336,   190,    -1,   157,    -1,   189,
     336,   190,    -1,   361,    -1,   370,    -1,   369,    30,   370,
      -1,   369,    48,   370,    -1,   203,    -1,    70,    -1,   107,
      -1,   108,    -1,   109,    -1,    27,    -1,    28,    -1,   110,
      -1,   111,    -1,   169,    -1,   112,    -1,    71,    -1,    72,
      -1,    74,    -1,    73,    -1,    90,    -1,    91,    -1,    89,
      -1,    92,    -1,    93,    -1,    94,    -1,    95,    -1,    96,
      -1,    97,    -1,    54,    -1,    98,    -1,   100,    -1,   101,
      -1,   102,    -1,   103,    -1,   104,    -1,   106,    -1,   105,
      -1,    88,    -1,    13,    -1,   126,    -1,   127,    -1,   128,
      -1,   129,    -1,    69,    -1,    68,    -1,   121,    -1,     5,
      -1,     7,    -1,     6,    -1,     4,    -1,     3,    -1,   152,
      -1,   113,    -1,   114,    -1,   123,    -1,   124,    -1,   125,
      -1,   120,    -1,   119,    -1,   118,    -1,   117,    -1,   116,
      -1,   115,    -1,   182,    -1,   122,    -1,   133,    -1,   134,
      -1,    10,    -1,    12,    -1,    11,    -1,   136,    -1,   138,
      -1,   137,    -1,   139,    -1,   140,    -1,   154,    -1,   153,
      -1,   181,    -1,   164,    -1,   167,    -1,   166,    -1,   177,
      -1,   179,    -1,   176,    -1,   215,   186,   281,   187,    -1,
     216,    -1,   156,    -1,   372,    -1,   380,    -1,   120,    -1,
     422,    -1,   186,   337,   187,    -1,   373,    -1,   374,   151,
     423,    -1,   373,    -1,   420,    -1,   375,   151,   423,    -1,
     372,    -1,   120,    -1,   425,    -1,   186,   187,    -1,   327,
      -1,    -1,    -1,    86,    -1,   434,    -1,   186,   281,   187,
      -1,    -1,    75,    -1,    76,    -1,    77,    -1,    87,    -1,
     139,    -1,   140,    -1,   154,    -1,   136,    -1,   167,    -1,
     137,    -1,   138,    -1,   153,    -1,   181,    -1,   147,    86,
     148,    -1,   147,   148,    -1,   380,    -1,   214,    -1,   134,
     186,   384,   187,    -1,    67,   384,   193,    -1,   176,   186,
     353,   187,    -1,   382,    -1,   357,    -1,   186,   381,   187,
      -1,   381,    31,   381,    -1,   381,    32,   381,    -1,   381,
      10,   381,    -1,   381,    12,   381,    -1,   381,    11,   381,
      -1,   381,    33,   381,    -1,   381,    35,   381,    -1,   381,
      34,   381,    -1,   381,    49,   381,    -1,   381,    47,   381,
      -1,   381,    48,   381,    -1,   381,    50,   381,    -1,   381,
      51,   381,    -1,   381,    52,   381,    -1,   381,    46,   381,
      -1,   381,    45,   381,    -1,   381,    66,   381,    -1,    53,
     381,    -1,    55,   381,    -1,    47,   381,    -1,    48,   381,
      -1,   381,    37,   381,    -1,   381,    36,   381,    -1,   381,
      39,   381,    -1,   381,    38,   381,    -1,   381,    40,   381,
      -1,   381,    44,   381,    -1,   381,    41,   381,    -1,   381,
      43,   381,    -1,   381,    42,   381,    -1,   381,    29,   381,
      30,   381,    -1,   381,    29,    30,   381,    -1,   216,   151,
     204,    -1,   156,   151,   204,    -1,   216,   151,   126,    -1,
     214,    -1,    79,    -1,   439,    -1,   380,    -1,   194,   434,
     194,    -1,   195,   434,   195,    -1,   147,   434,   148,    -1,
     387,   385,    -1,    -1,     9,    -1,    -1,     9,    -1,    -1,
     387,     9,   381,   132,   381,    -1,   387,     9,   381,    -1,
     381,   132,   381,    -1,   381,    -1,    75,    -1,    76,    -1,
      77,    -1,   147,    86,   148,    -1,   147,   148,    -1,    75,
      -1,    76,    -1,    77,    -1,   203,    -1,    87,    -1,    87,
      49,   390,    -1,   388,    -1,   390,    -1,   203,    -1,    47,
     389,    -1,    48,   389,    -1,   134,   186,   392,   187,    -1,
      67,   392,   193,    -1,   176,   186,   395,   187,    -1,   393,
     385,    -1,    -1,   393,     9,   391,   132,   391,    -1,   393,
       9,   391,    -1,   391,   132,   391,    -1,   391,    -1,   394,
       9,   391,    -1,   391,    -1,   396,   385,    -1,    -1,   396,
       9,   349,   132,   391,    -1,   349,   132,   391,    -1,   394,
     385,    -1,    -1,   186,   397,   187,    -1,    -1,   399,     9,
     203,   398,    -1,   203,   398,    -1,    -1,   401,   399,   385,
      -1,    46,   400,    45,    -1,   402,    -1,    -1,   130,    -1,
     131,    -1,   203,    -1,   156,    -1,   189,   336,   190,    -1,
     405,    -1,   423,    -1,   203,    -1,   189,   336,   190,    -1,
     407,    -1,   423,    -1,    67,   424,   193,    -1,   189,   336,
     190,    -1,   415,   409,    -1,   186,   326,   187,   409,    -1,
     426,   409,    -1,   186,   326,   187,   409,    -1,   186,   326,
     187,   404,   406,    -1,   186,   337,   187,   404,   406,    -1,
     186,   326,   187,   404,   405,    -1,   186,   337,   187,   404,
     405,    -1,   421,    -1,   371,    -1,   419,    -1,   420,    -1,
     410,    -1,   412,    -1,   414,   404,   406,    -1,   375,   151,
     423,    -1,   416,   186,   281,   187,    -1,   417,   186,   281,
     187,    -1,   186,   414,   187,    -1,   371,    -1,   419,    -1,
     420,    -1,   410,    -1,   414,   404,   406,    -1,   413,    -1,
     416,   186,   281,   187,    -1,   186,   414,   187,    -1,   375,
     151,   423,    -1,   421,    -1,   410,    -1,   371,    -1,   355,
      -1,   380,    -1,   186,   414,   187,    -1,   186,   337,   187,
      -1,   417,   186,   281,   187,    -1,   416,   186,   281,   187,
      -1,   186,   418,   187,    -1,   339,    -1,   342,    -1,   414,
     404,   408,   446,   186,   281,   187,    -1,   186,   326,   187,
     404,   408,   446,   186,   281,   187,    -1,   375,   151,   205,
     446,   186,   281,   187,    -1,   375,   151,   423,   186,   281,
     187,    -1,   375,   151,   189,   336,   190,   186,   281,   187,
      -1,   422,    -1,   422,    67,   424,   193,    -1,   422,   189,
     336,   190,    -1,   423,    -1,    80,    -1,   191,   189,   336,
     190,    -1,   191,   423,    -1,   336,    -1,    -1,   421,    -1,
     411,    -1,   412,    -1,   425,   404,   406,    -1,   374,   151,
     421,    -1,   186,   414,   187,    -1,    -1,   411,    -1,   413,
      -1,   425,   404,   405,    -1,   186,   414,   187,    -1,   427,
       9,    -1,   427,     9,   414,    -1,   427,     9,   133,   186,
     427,   187,    -1,    -1,   414,    -1,   133,   186,   427,   187,
      -1,   429,   385,    -1,    -1,   429,     9,   336,   132,   336,
      -1,   429,     9,   336,    -1,   336,   132,   336,    -1,   336,
      -1,   429,     9,   336,   132,    35,   414,    -1,   429,     9,
      35,   414,    -1,   336,   132,    35,   414,    -1,    35,   414,
      -1,   431,   385,    -1,    -1,   431,     9,   336,   132,   336,
      -1,   431,     9,   336,    -1,   336,   132,   336,    -1,   336,
      -1,   433,   385,    -1,    -1,   433,     9,   381,   132,   381,
      -1,   433,     9,   381,    -1,   381,   132,   381,    -1,   381,
      -1,   434,   435,    -1,   434,    86,    -1,   435,    -1,    86,
     435,    -1,    80,    -1,    80,    67,   436,   193,    -1,    80,
     404,   203,    -1,   149,   336,   190,    -1,   149,    79,    67,
     336,   193,   190,    -1,   150,   414,   190,    -1,   203,    -1,
      81,    -1,    80,    -1,   123,   186,   328,   187,    -1,   124,
     186,   414,   187,    -1,   124,   186,   337,   187,    -1,   124,
     186,   418,   187,    -1,   124,   186,   417,   187,    -1,   124,
     186,   326,   187,    -1,     7,   336,    -1,     6,   336,    -1,
       5,   186,   336,   187,    -1,     4,   336,    -1,     3,   336,
      -1,   414,    -1,   438,     9,   414,    -1,   375,   151,   204,
      -1,   375,   151,   126,    -1,    -1,    98,   460,    -1,   177,
     445,    14,   460,   188,    -1,   402,   177,   445,    14,   460,
     188,    -1,   179,   445,   440,    14,   460,   188,    -1,   402,
     179,   445,   440,    14,   460,   188,    -1,   205,    -1,   460,
     205,    -1,   204,    -1,   460,   204,    -1,   205,    -1,   205,
     172,   452,   173,    -1,   203,    -1,   203,   172,   452,   173,
      -1,   172,   448,   173,    -1,    -1,   460,    -1,   447,     9,
     460,    -1,   447,   385,    -1,   447,     9,   165,    -1,   448,
      -1,   165,    -1,    -1,    -1,    30,   460,    -1,    98,   460,
      -1,    99,   460,    -1,   452,     9,   453,   203,    -1,   453,
     203,    -1,   452,     9,   453,   203,   451,    -1,   453,   203,
     451,    -1,    47,    -1,    48,    -1,    -1,    87,   132,   460,
      -1,    29,    87,   132,   460,    -1,   216,   151,   203,   132,
     460,    -1,   455,     9,   454,    -1,   454,    -1,   455,   385,
      -1,    -1,   176,   186,   456,   187,    -1,   216,    -1,   203,
     151,   459,    -1,   203,   446,    -1,    29,   460,    -1,    56,
     460,    -1,   216,    -1,   134,    -1,   135,    -1,   457,    -1,
     458,   151,   459,    -1,   134,   172,   460,   173,    -1,   134,
     172,   460,     9,   460,   173,    -1,   156,    -1,   186,   107,
     186,   449,   187,    30,   460,   187,    -1,   186,   460,     9,
     447,   385,   187,    -1,   460,    -1,    -1
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
    1179,  1177,  1188,  1188,  1195,  1194,  1206,  1204,  1217,  1218,
    1222,  1225,  1228,  1229,  1230,  1233,  1234,  1237,  1239,  1242,
    1243,  1246,  1247,  1250,  1251,  1255,  1256,  1261,  1262,  1265,
    1266,  1267,  1271,  1272,  1276,  1277,  1281,  1282,  1286,  1287,
    1292,  1293,  1298,  1299,  1300,  1301,  1304,  1307,  1309,  1312,
    1313,  1317,  1319,  1322,  1325,  1328,  1329,  1332,  1333,  1337,
    1343,  1349,  1356,  1358,  1363,  1368,  1374,  1378,  1382,  1386,
    1391,  1396,  1401,  1406,  1412,  1421,  1426,  1431,  1437,  1439,
    1443,  1447,  1452,  1456,  1459,  1462,  1466,  1470,  1474,  1478,
    1483,  1491,  1493,  1496,  1497,  1498,  1499,  1501,  1503,  1508,
    1509,  1512,  1513,  1514,  1518,  1519,  1521,  1522,  1526,  1528,
    1531,  1535,  1541,  1543,  1546,  1546,  1550,  1549,  1553,  1555,
    1558,  1561,  1559,  1575,  1571,  1585,  1587,  1589,  1591,  1593,
    1595,  1597,  1601,  1602,  1603,  1606,  1612,  1616,  1622,  1625,
    1630,  1632,  1637,  1642,  1646,  1647,  1651,  1652,  1654,  1656,
    1662,  1663,  1665,  1669,  1670,  1675,  1679,  1680,  1684,  1685,
    1689,  1691,  1697,  1702,  1703,  1705,  1709,  1710,  1711,  1712,
    1716,  1717,  1718,  1719,  1720,  1721,  1723,  1728,  1731,  1732,
    1736,  1737,  1741,  1742,  1745,  1746,  1749,  1750,  1753,  1754,
    1758,  1759,  1760,  1761,  1762,  1763,  1764,  1768,  1769,  1772,
    1773,  1774,  1777,  1779,  1781,  1782,  1785,  1787,  1791,  1793,
    1797,  1801,  1805,  1810,  1811,  1813,  1814,  1815,  1816,  1819,
    1823,  1824,  1828,  1829,  1833,  1834,  1835,  1836,  1840,  1844,
    1849,  1853,  1857,  1862,  1863,  1864,  1865,  1866,  1870,  1872,
    1873,  1874,  1877,  1878,  1879,  1880,  1881,  1882,  1883,  1884,
    1885,  1886,  1887,  1888,  1889,  1890,  1891,  1892,  1893,  1894,
    1895,  1896,  1897,  1898,  1899,  1900,  1901,  1902,  1903,  1904,
    1905,  1906,  1907,  1908,  1909,  1910,  1911,  1912,  1913,  1914,
    1915,  1916,  1917,  1918,  1919,  1921,  1922,  1924,  1925,  1927,
    1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,  1936,  1937,
    1938,  1939,  1940,  1941,  1942,  1943,  1944,  1945,  1949,  1953,
    1958,  1957,  1972,  1970,  1988,  1987,  2006,  2005,  2024,  2023,
    2041,  2041,  2056,  2056,  2074,  2075,  2076,  2081,  2083,  2087,
    2091,  2097,  2101,  2107,  2109,  2113,  2115,  2119,  2123,  2124,
    2128,  2135,  2142,  2144,  2149,  2150,  2151,  2152,  2154,  2158,
    2159,  2160,  2161,  2165,  2171,  2180,  2193,  2194,  2197,  2200,
    2203,  2204,  2207,  2211,  2214,  2217,  2224,  2225,  2229,  2230,
    2232,  2236,  2237,  2238,  2239,  2240,  2241,  2242,  2243,  2244,
    2245,  2246,  2247,  2248,  2249,  2250,  2251,  2252,  2253,  2254,
    2255,  2256,  2257,  2258,  2259,  2260,  2261,  2262,  2263,  2264,
    2265,  2266,  2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,
    2275,  2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,
    2285,  2286,  2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,
    2295,  2296,  2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,
    2305,  2306,  2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314,
    2315,  2319,  2324,  2325,  2329,  2330,  2331,  2332,  2334,  2338,
    2339,  2350,  2351,  2353,  2365,  2366,  2367,  2371,  2372,  2373,
    2377,  2378,  2379,  2382,  2384,  2388,  2389,  2390,  2391,  2393,
    2394,  2395,  2396,  2397,  2398,  2399,  2400,  2401,  2402,  2405,
    2410,  2411,  2412,  2414,  2415,  2417,  2418,  2419,  2420,  2422,
    2424,  2426,  2428,  2430,  2431,  2432,  2433,  2434,  2435,  2436,
    2437,  2438,  2439,  2440,  2441,  2442,  2443,  2444,  2445,  2446,
    2448,  2450,  2452,  2454,  2455,  2458,  2459,  2463,  2467,  2469,
    2473,  2476,  2479,  2485,  2486,  2487,  2488,  2489,  2490,  2491,
    2496,  2498,  2502,  2503,  2506,  2507,  2511,  2514,  2516,  2518,
    2522,  2523,  2524,  2525,  2528,  2532,  2533,  2534,  2535,  2539,
    2541,  2548,  2549,  2550,  2551,  2552,  2553,  2555,  2556,  2561,
    2563,  2566,  2569,  2571,  2573,  2576,  2578,  2582,  2584,  2587,
    2590,  2596,  2598,  2601,  2602,  2607,  2610,  2614,  2614,  2619,
    2622,  2623,  2627,  2628,  2632,  2633,  2634,  2638,  2643,  2648,
    2649,  2653,  2658,  2663,  2664,  2668,  2669,  2674,  2676,  2681,
    2692,  2706,  2718,  2733,  2734,  2735,  2736,  2737,  2738,  2739,
    2749,  2758,  2760,  2762,  2766,  2767,  2768,  2769,  2770,  2786,
    2787,  2789,  2791,  2798,  2799,  2800,  2801,  2802,  2803,  2804,
    2805,  2807,  2812,  2816,  2817,  2821,  2824,  2831,  2835,  2844,
    2851,  2859,  2861,  2862,  2866,  2867,  2869,  2874,  2875,  2886,
    2887,  2888,  2889,  2900,  2903,  2906,  2907,  2908,  2909,  2920,
    2924,  2925,  2926,  2928,  2929,  2930,  2934,  2936,  2939,  2941,
    2942,  2943,  2944,  2947,  2949,  2950,  2954,  2956,  2959,  2961,
    2962,  2963,  2967,  2969,  2972,  2975,  2977,  2979,  2983,  2984,
    2986,  2987,  2993,  2994,  2996,  3006,  3008,  3010,  3013,  3014,
    3015,  3019,  3020,  3021,  3022,  3023,  3024,  3025,  3026,  3027,
    3028,  3029,  3033,  3034,  3038,  3040,  3048,  3050,  3054,  3058,
    3063,  3067,  3075,  3076,  3080,  3081,  3087,  3088,  3097,  3098,
    3106,  3109,  3113,  3116,  3121,  3126,  3128,  3129,  3130,  3134,
    3135,  3139,  3140,  3143,  3146,  3148,  3152,  3158,  3159,  3160,
    3164,  3168,  3178,  3186,  3188,  3192,  3194,  3199,  3205,  3208,
    3213,  3221,  3224,  3227,  3228,  3231,  3234,  3235,  3240,  3243,
    3247,  3251,  3257,  3267,  3268
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
     248,   244,   250,   249,   252,   251,   253,   251,   254,   254,
     255,   256,   257,   257,   257,   257,   257,   258,   258,   259,
     259,   260,   260,   261,   261,   262,   262,   263,   263,   264,
     264,   264,   265,   265,   266,   266,   267,   267,   268,   268,
     269,   269,   270,   270,   270,   270,   271,   271,   271,   272,
     272,   273,   273,   274,   274,   275,   275,   276,   276,   277,
     277,   277,   277,   277,   277,   277,   277,   278,   278,   278,
     278,   278,   278,   278,   278,   279,   279,   279,   279,   279,
     279,   279,   279,   280,   280,   280,   280,   280,   280,   280,
     280,   281,   281,   282,   282,   282,   282,   282,   282,   283,
     283,   284,   284,   284,   285,   285,   285,   285,   286,   286,
     287,   288,   289,   289,   291,   290,   292,   290,   290,   290,
     290,   293,   290,   294,   290,   290,   290,   290,   290,   290,
     290,   290,   295,   295,   295,   296,   297,   297,   298,   298,
     299,   299,   300,   300,   301,   301,   302,   302,   302,   302,
     302,   302,   302,   303,   303,   304,   305,   305,   306,   306,
     307,   307,   308,   309,   309,   309,   310,   310,   310,   310,
     311,   311,   311,   311,   311,   311,   311,   312,   312,   312,
     313,   313,   314,   314,   315,   315,   316,   316,   317,   317,
     318,   318,   318,   318,   318,   318,   318,   319,   319,   320,
     320,   320,   321,   321,   321,   321,   322,   322,   323,   323,
     324,   324,   325,   326,   326,   326,   326,   326,   326,   327,
     328,   328,   329,   329,   330,   330,   330,   330,   331,   332,
     333,   334,   335,   336,   336,   336,   336,   336,   337,   337,
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
       4,     1,     3,     1,     3,     2,     1,     2,     2,     5,
       4,     2,     0,     1,     1,     1,     1,     3,     5,     8,
       0,     4,     0,     6,     0,    10,     0,     4,     2,     3,
       2,     3,     2,     3,     3,     3,     3,     3,     3,     5,
       1,     1,     1,     0,     9,     0,    10,     5,     0,    13,
       0,     5,     3,     3,     2,     2,     2,     2,     2,     2,
       3,     2,     2,     0,     4,     9,     0,     0,     4,     2,
       0,     1,     0,     1,     0,     9,     0,    10,     0,    11,
       0,     9,     0,    10,     0,     8,     0,     9,     0,     7,
       0,     8,     0,     8,     0,     7,     0,     8,     1,     1,
       1,     1,     1,     2,     3,     3,     2,     2,     0,     2,
       0,     2,     0,     1,     3,     1,     3,     2,     0,     1,
       2,     4,     1,     4,     1,     4,     1,     4,     1,     4,
       3,     5,     3,     4,     4,     5,     5,     4,     0,     1,
       1,     4,     0,     5,     0,     2,     0,     3,     0,     7,
       8,     6,     2,     5,     6,     4,     0,     4,     5,     7,
       6,     6,     7,     9,     8,     6,     7,     5,     2,     4,
       5,     3,     0,     3,     4,     6,     5,     5,     6,     8,
       7,     2,     0,     1,     2,     2,     3,     4,     4,     3,
       1,     1,     2,     4,     3,     5,     1,     3,     2,     0,
       2,     3,     2,     0,     0,     4,     0,     5,     2,     2,
       2,     0,    10,     0,    11,     3,     3,     3,     4,     4,
       3,     5,     2,     2,     0,     6,     5,     4,     3,     1,
       1,     3,     4,     1,     2,     1,     1,     4,     6,     1,
       1,     4,     1,     1,     3,     2,     2,     0,     2,     0,
       1,     3,     1,     1,     1,     1,     3,     4,     4,     4,
       1,     1,     2,     2,     2,     3,     3,     1,     1,     1,
       1,     3,     1,     3,     1,     1,     1,     0,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     3,     5,     1,     3,     5,     4,     3,     3,
       3,     4,     3,     3,     3,     2,     2,     1,     1,     3,
       3,     1,     1,     0,     1,     2,     4,     3,     3,     6,
       2,     3,     6,     1,     1,     1,     1,     1,     6,     3,
       4,     6,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       5,     4,     1,     2,     2,     2,     2,     2,     2,     2,
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
       0,   424,     0,   787,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   877,     0,
     865,   669,     0,   675,   676,   677,    22,   734,   854,   151,
     152,   678,     0,   132,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
     390,   391,   392,   395,   394,   393,     0,     0,     0,     0,
     212,     0,     0,     0,   682,   684,   685,   679,   680,     0,
       0,     0,   686,   681,     0,   653,    24,    25,    26,    28,
      27,     0,   683,     0,     0,     0,     0,   687,   396,   522,
       0,   150,   122,     0,   670,     0,     0,     4,   111,   113,
     116,   733,     0,   652,     0,     6,   182,     7,     9,     8,
      10,     0,     0,   388,   435,     0,     0,     0,     0,     0,
       0,     0,   433,   843,   844,   504,   503,   418,   507,     0,
     417,   814,   654,   661,     0,   736,   502,   387,   817,   818,
     829,   434,     0,     0,   437,   436,   815,   816,   813,   850,
     853,   492,   735,    11,   395,   394,   393,     0,     0,    28,
       0,   111,   182,     0,   921,   434,   920,     0,   918,   917,
     506,     0,   425,   430,     0,     0,   475,   476,   477,   478,
     501,   499,   498,   497,   496,   495,   494,   493,   854,   678,
     656,     0,     0,   941,   836,   654,     0,   655,   457,     0,
     455,     0,   881,     0,   743,   416,   665,   202,     0,   941,
     415,   664,   659,     0,   674,   655,   860,   861,   867,   859,
     666,     0,     0,   668,   500,     0,     0,     0,     0,   421,
       0,   130,   423,     0,     0,   136,   138,     0,     0,   140,
       0,    69,    68,    63,    62,    54,    55,    46,    66,    77,
       0,    49,     0,    61,    53,    59,    79,    72,    71,    44,
      67,    86,    87,    45,    82,    42,    83,    43,    84,    41,
      88,    76,    80,    85,    73,    74,    48,    75,    78,    40,
      70,    56,    89,    64,    57,    47,    39,    38,    37,    36,
      35,    34,    58,    90,    92,    51,    32,    33,    60,   974,
     975,    52,   980,    31,    50,    81,     0,     0,   111,    91,
     932,   973,     0,   976,     0,     0,   142,     0,     0,   173,
       0,     0,     0,     0,     0,     0,    94,    99,   301,     0,
       0,   300,     0,   216,     0,   213,   306,     0,     0,     0,
       0,     0,   938,   198,   210,   873,   877,     0,   902,     0,
     689,     0,     0,     0,   900,     0,    16,     0,   115,   190,
     204,   211,   559,   534,     0,   926,   514,   516,   518,   791,
     424,   435,     0,     0,   433,   434,   436,     0,     0,   856,
     671,     0,   672,     0,     0,     0,   172,     0,     0,   118,
     292,     0,    21,   181,     0,   209,   194,   208,   393,   396,
     182,   389,   165,   166,   167,   168,   169,   171,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   865,     0,   164,   858,   858,   887,
       0,     0,     0,     0,     0,     0,     0,     0,   386,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   456,   454,   792,   793,     0,   858,     0,   805,
     292,   292,   858,     0,   873,     0,   182,     0,     0,   144,
       0,   789,   784,   743,     0,   435,   433,     0,   885,     0,
     539,   742,   876,   674,   435,   433,   434,   118,     0,   292,
     414,     0,   807,   667,     0,   122,   252,     0,   521,     0,
     147,     0,     0,   422,     0,     0,     0,     0,     0,   139,
     163,   141,   974,   975,   971,   972,     0,   966,     0,     0,
       0,     0,    65,    30,    52,    29,   933,   170,   143,   122,
       0,   160,   162,     0,     0,    96,   103,     0,     0,    98,
     107,   100,     0,    18,     0,     0,   302,     0,   145,   215,
     214,     0,     0,   146,   922,     0,     0,   435,   433,   434,
     437,   436,     0,   959,   222,     0,   874,     0,     0,   148,
       0,     0,   688,   901,   734,     0,     0,   899,   739,   898,
     114,     5,    13,    14,     0,   220,     0,     0,   527,     0,
       0,   743,     0,     0,   662,   657,   528,     0,     0,     0,
       0,   791,   122,     0,   745,   790,   984,   413,   427,   489,
     823,   842,   127,   121,   123,   124,   125,   126,   387,     0,
     505,   737,   738,   112,   743,     0,   942,     0,     0,     0,
     745,   293,     0,   510,   184,   218,     0,   460,   462,   461,
       0,     0,   458,   459,   463,   465,   464,   480,   479,   482,
     481,   483,   485,   487,   486,   484,   474,   473,   467,   468,
     466,   469,   470,   472,   488,   471,   857,     0,     0,   891,
       0,   743,   925,     0,   924,   941,   820,   200,   192,   206,
       0,   926,   196,   182,     0,   428,   431,   439,   453,   452,
     451,   450,   449,   448,   447,   446,   445,   444,   443,   442,
     795,     0,   794,   797,   819,   801,   941,   798,     0,     0,
       0,     0,     0,     0,     0,     0,   919,   426,   782,   786,
     742,   788,     0,   658,     0,   880,     0,   879,   218,     0,
     658,   864,   863,   850,   853,     0,     0,   794,   797,   862,
     798,   419,   254,   256,   122,   525,   524,   420,     0,   122,
     236,   131,   423,     0,     0,     0,     0,     0,   248,   248,
     137,     0,     0,     0,     0,   964,   743,     0,   948,     0,
       0,     0,     0,     0,   741,     0,   653,     0,     0,   116,
     691,   652,   696,     0,   690,   120,   695,   941,   977,     0,
       0,     0,   104,     0,    19,     0,   108,     0,    20,     0,
       0,    93,   101,     0,   299,   307,   304,     0,     0,   911,
     916,   913,   912,   915,   914,    12,   957,   958,     0,     0,
       0,     0,   873,   870,     0,   538,   910,   909,   908,     0,
     904,     0,   905,   907,     0,     5,     0,     0,     0,   553,
     554,   562,   561,     0,   433,     0,   742,   533,   537,     0,
       0,   927,     0,   515,     0,     0,   949,   791,   278,   983,
       0,     0,   806,     0,   855,   742,   944,   940,   294,   295,
     651,   744,   291,     0,   791,     0,     0,   220,   512,   186,
     491,     0,   542,   543,     0,   540,   742,   886,     0,     0,
     292,   222,     0,   220,     0,     0,   218,     0,   865,   440,
       0,     0,   803,   804,   821,   822,   851,   852,     0,     0,
       0,   770,   750,   751,   752,   759,     0,     0,     0,   763,
     761,   762,   776,   743,     0,   784,   884,   883,     0,   220,
       0,   808,   673,     0,   258,     0,     0,   128,     0,     0,
       0,     0,     0,     0,     0,   228,   229,   240,     0,   122,
     238,   157,   248,     0,   248,     0,     0,   978,     0,     0,
       0,   742,   965,   967,   947,   743,   946,     0,   743,   717,
     718,   715,   716,   749,     0,   743,   741,     0,   536,     0,
       0,   893,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     970,   174,     0,   177,   161,     0,    95,   105,     0,    97,
     109,   102,   303,     0,   923,   149,   959,   939,   954,   221,
     223,   313,     0,     0,   871,     0,   903,     0,    17,     0,
     926,   219,   313,     0,     0,   658,   530,     0,   663,   928,
       0,   949,   519,     0,     0,   984,     0,   283,   281,   797,
     809,   941,   797,   810,   943,     0,     0,   296,   119,     0,
     791,   217,     0,   791,     0,   490,   890,   889,     0,   292,
       0,     0,     0,     0,     0,     0,   220,   188,   674,   796,
     292,     0,   755,   756,   757,   758,   764,   765,   774,     0,
     743,     0,   770,     0,   754,   778,   742,   781,   783,   785,
       0,   878,     0,   796,     0,     0,     0,     0,   255,   526,
     133,     0,   423,   228,   230,   873,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   242,     0,     0,   960,     0,
     963,   742,     0,     0,     0,   693,   742,   740,     0,   731,
       0,   743,     0,   697,   732,   730,   897,     0,   743,   700,
     702,   701,     0,     0,   698,   699,   703,   705,   704,   720,
     719,   722,   721,   723,   725,   727,   726,   724,   713,   712,
     707,   708,   706,   709,   710,   711,   714,   969,     0,   122,
     106,   110,   305,     0,     0,     0,   956,     0,   387,   875,
     873,   429,   432,   438,     0,    15,     0,   387,   565,     0,
       0,   567,   560,   563,     0,   558,     0,   930,     0,   950,
     523,     0,   284,     0,     0,   279,     0,   298,   297,   949,
       0,   313,     0,   791,     0,   292,     0,   848,   313,   926,
     313,   929,     0,     0,     0,   441,     0,     0,   767,   742,
     769,   760,     0,   753,     0,     0,   743,   775,   882,   313,
       0,   122,     0,   251,   237,     0,     0,     0,   227,   153,
     241,     0,     0,   244,     0,   249,   250,   122,   243,   979,
     961,     0,   945,     0,   982,   748,   747,   692,     0,   742,
     535,   694,     0,   541,   742,   892,   729,     0,     0,     0,
     953,   951,   952,   224,     0,     0,     0,   394,   385,     0,
       0,     0,   199,   312,   314,     0,   384,     0,     0,     0,
     926,   387,     0,   906,   309,   205,   556,     0,     0,   529,
     517,     0,   287,   277,     0,   280,   286,   292,   509,   949,
     387,   949,     0,   888,     0,   847,   387,     0,   387,   931,
     313,   791,   845,   773,   772,   766,     0,   768,   742,   777,
     387,   122,   257,   129,   134,   155,   231,     0,   239,   245,
     122,   247,   962,     0,     0,   532,     0,   896,   895,   728,
     122,   178,   955,     0,     0,     0,   934,     0,     0,     0,
     225,     0,   926,     0,   350,   346,   352,   653,    28,     0,
     340,     0,   345,   349,   362,     0,   360,   365,     0,   364,
       0,   363,     0,   182,   316,     0,   318,     0,   319,   320,
       0,     0,   872,     0,   557,   555,   566,   564,   288,     0,
       0,   275,   285,     0,     0,     0,     0,   195,   509,   949,
     849,   201,   309,   207,   387,     0,     0,   780,     0,   203,
     253,     0,     0,   122,   234,   154,   246,   981,   746,     0,
       0,     0,     0,     0,   412,     0,   935,     0,   330,   334,
     409,   410,   344,     0,     0,     0,   325,   617,   616,   613,
     615,   614,   634,   636,   635,   605,   576,   577,   595,   611,
     610,   572,   582,   583,   585,   584,   604,   588,   586,   587,
     589,   590,   591,   592,   593,   594,   596,   597,   598,   599,
     600,   601,   603,   602,   573,   574,   575,   578,   579,   581,
     619,   620,   629,   628,   627,   626,   625,   624,   612,   631,
     621,   622,   623,   606,   607,   608,   609,   632,   633,   637,
     639,   638,   640,   641,   618,   643,   642,   645,   647,   646,
     580,   650,   648,   649,   644,   630,   571,   357,   568,     0,
     326,   378,   379,   377,   370,     0,   371,   327,   404,     0,
       0,     0,     0,   408,     0,   182,   191,   308,     0,     0,
       0,   276,   290,   846,     0,   122,   380,   122,   185,     0,
       0,     0,   197,   949,   771,     0,   122,   232,   135,   156,
       0,   531,   894,   176,   328,   329,   407,   226,     0,     0,
     743,     0,   353,   341,     0,     0,     0,   359,   361,     0,
       0,   366,   373,   374,   372,     0,     0,   315,   936,     0,
       0,     0,   411,     0,   310,     0,   289,     0,   551,   745,
       0,     0,   122,   187,   193,     0,   779,     0,     0,   158,
     331,   111,     0,   332,   333,     0,     0,   347,   742,   355,
     351,   356,   569,   570,     0,   342,   375,   376,   368,   369,
     367,   405,   402,   959,   321,   317,   406,     0,   311,   552,
     744,     0,   511,   381,     0,   189,     0,   235,     0,   180,
       0,   387,     0,   354,   358,     0,     0,   791,   323,     0,
     549,   508,   513,   233,     0,     0,   159,   338,     0,   386,
     348,   403,   937,     0,   745,   398,   791,   550,     0,   179,
       0,     0,   337,   949,   791,   262,   399,   400,   401,   984,
     397,     0,     0,     0,   336,     0,   398,     0,   949,     0,
     335,   382,   122,   322,   984,     0,   267,   265,     0,   122,
       0,     0,   268,     0,     0,   263,   324,     0,   383,     0,
     271,   261,     0,   264,   270,   175,   272,     0,     0,   259,
     269,     0,   260,   274,   273
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   855,   601,   171,  1406,   695,
     335,   554,   558,   336,   555,   559,   109,   110,   111,   112,
     113,   114,   387,   633,   634,   522,   240,  1471,   528,  1387,
    1472,  1709,   811,   330,   549,  1669,  1034,  1209,  1726,   404,
     172,   635,   895,  1094,  1264,   118,   604,   912,   636,   655,
     916,   584,   911,   220,   503,   637,   605,   913,   406,   353,
     370,   121,   897,   858,   841,  1049,  1409,  1147,   965,  1618,
    1475,   771,   971,   527,   780,   973,  1297,   763,   954,   957,
    1136,  1733,  1734,   623,   624,   649,   650,   340,   341,   347,
    1443,  1597,  1598,  1218,  1333,  1432,  1591,  1717,  1736,  1628,
    1673,  1674,  1675,  1419,  1420,  1421,  1422,  1630,  1631,  1637,
    1685,  1425,  1426,  1430,  1584,  1585,  1586,  1608,  1763,  1334,
    1335,   173,   123,  1749,  1750,  1589,  1337,  1338,  1339,  1340,
     124,   233,   523,   524,   125,   126,   127,   128,   129,   130,
     131,   132,  1455,   133,   894,  1093,   134,   620,   621,   622,
     237,   379,   518,   610,   611,  1171,   612,  1172,   135,   136,
     137,   802,   138,   139,  1659,   140,   606,  1445,   607,  1063,
     863,  1235,  1232,  1577,  1578,   141,   142,   143,   223,   144,
     224,   234,   391,   510,   145,   993,   806,   146,   994,   886,
     878,   995,   940,  1116,   941,  1118,  1119,  1120,   943,  1275,
    1276,   944,   739,   493,   184,   185,   638,   626,   476,  1079,
    1080,   725,   726,   882,   148,   226,   149,   150,   175,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   687,   230,
     231,   587,   213,   214,   690,   691,  1177,  1178,   363,   364,
     849,   161,   575,   162,   619,   163,   322,  1599,  1649,   354,
     399,   644,   645,   987,  1074,  1216,   838,   839,   785,   786,
     787,   323,   324,   808,  1408,   880
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1452
static const yytype_int16 yypact[] =
{
   -1452,   174, -1452, -1452,  5212, 12932, 12932,   -10, 12932, 12932,
   12932, 10809, 12932, -1452, 12932, 12932, 12932, 12932, 12932, 12932,
   12932, 12932, 12932, 12932, 12932, 12932, 15719, 15719, 11002, 12932,
   14171,    25,   191, -1452, -1452, -1452, -1452, -1452,   195, -1452,
   -1452,   158, 12932, -1452,   191,   213,   226,   304, -1452,   191,
   11195,   833, 11388, -1452, 13834,  9844,   124, 12932,  1223,    81,
   -1452, -1452, -1452,   261,   271,    69,   332,   341,   349,   359,
   -1452,   833,   369,   371, -1452, -1452, -1452, -1452, -1452, 12932,
     620,   898, -1452, -1452,   833, -1452, -1452, -1452, -1452,   833,
   -1452,   833, -1452,   204,   386,   833,   833, -1452,   406, -1452,
   11581, -1452, -1452,   328,   533,   541,   541, -1452,   546,   441,
      -1, -1452,   419, -1452,    80, -1452,   573, -1452, -1452, -1452,
   -1452,  1463,  1165, -1452, -1452,   353,   443,   450,   457,   468,
     472, 11180, -1452, -1452, -1452, -1452,   108, -1452,   547,   562,
   -1452,   152,   478, -1452,   521,    15, -1452,  2154,   161, -1452,
   -1452,  3001,    51,   489,   166, -1452,   154,   181,   492,   220,
   -1452, -1452,   625, -1452, -1452, -1452,   545,   512,   550, -1452,
   12932, -1452,   573,  1165, 16299,  3032, 16299, 12932, 16299, 16299,
    3827,   525, 15109,  3827,   674,   833,   659,   659,   295,   659,
     659,   659,   659,   659,   659,   659,   659,   659, -1452, -1452,
   -1452,    87, 12932,   566, -1452, -1452,   590,   560,    63,   567,
      63, 15719, 15319,   555,   741, -1452,   545, -1452, 12932,   566,
   -1452,   609, -1452,   610,   586, -1452,   164, -1452, -1452, -1452,
      63,    51, 11774, -1452, -1452, 12932,  8493,   766,    82, 16299,
    9458, -1452, 12932, 12932,   833, -1452, -1452, 11759,   592, -1452,
   12917, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
    2633, -1452,  2633, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,    89,
      88,   550, -1452, -1452, -1452, -1452,   595,   559,    96, -1452,
   -1452,   627,   772, -1452,   636, 14674, -1452,   601, 14437, -1452,
      47, 14482,  1390,  1410,   833,    90, -1452,    50, -1452, 15343,
      97, -1452,   664, -1452,   671, -1452,   785,   100, 15719, 12932,
   12932,   614,   631, -1452, -1452, 15436, 11002,   103,    93,   554,
   -1452, 13125, 15719,   643, -1452,   833, -1452,   385,   441, -1452,
   -1452, -1452, -1452,  4177,   792,   709, -1452, -1452, -1452,    91,
   12932,   628,   629, 16299,   640,  1050,   649,  5405, 12932, -1452,
     420,   645,   646,   420,   436,   320, -1452,   833,  2633,   653,
   10037, 13834, -1452, -1452,   847, -1452, -1452, -1452, -1452, -1452,
     573, -1452, -1452, -1452, -1452, -1452, -1452, -1452, 12932, 12932,
   12932, 11967, 12932, 12932, 12932, 12932, 12932, 12932, 12932, 12932,
   12932, 12932, 12932, 12932, 12932, 12932, 12932, 12932, 12932, 12932,
   12932, 12932, 12932, 12932,  4447, 12932, -1452, 12932, 12932, 12932,
   13287,   833,   833,   833,   833,   833,  1463,   733,   702,  9651,
   12932, 12932, 12932, 12932, 12932, 12932, 12932, 12932, 12932, 12932,
   12932, 12932, -1452, -1452, -1452, -1452,  1031, 12932, 12932, -1452,
   10037, 10037, 12932, 12932, 15436,   657,   573, 12160, 14527, -1452,
   12932, -1452,   665,   835,   705,   663,   667, 13420,    63, 12353,
   -1452, 12546, -1452,   586,   670,   672,  2102, -1452,   163, 10037,
   -1452,  1039, -1452, -1452, 14572, -1452, -1452, 10230, -1452, 12932,
   -1452,   768,  8686,   853,   685, 16231,   860,    79,    70, -1452,
   -1452, -1452,   703, -1452, -1452, -1452,  2633,   650,   690,   870,
   15250,   833, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
     695, -1452, -1452,   833,   104, -1452,   290,   833,   105, -1452,
     352,   394,  1530, -1452,   833, 12932,    63,    81, -1452, -1452,
   -1452, 15250,   805, -1452,    63,   119,   122,   707,   708,  2179,
      67,   710,   712,   514,   776,   722,    63,   127,   731, -1452,
    1435,   833, -1452, -1452,   854,  1117,    32, -1452, -1452, -1452,
     441, -1452, -1452, -1452,   890,   795,   756,   230,   780, 12932,
     801,   926,   750,   788, -1452,   169, -1452,  2633,  2633,   927,
     766,    91, -1452,   758,   931, -1452,  2633,    60, -1452,   412,
     178, -1452, -1452, -1452, -1452, -1452, -1452, -1452,  1006,  2709,
   -1452, -1452, -1452, -1452,   938,   777, -1452, 15719, 12932,   762,
     943, 16299,   939, -1452, -1452,   827,  1108, 11373, 16427,  3827,
   12932, 13110, 16575, 13288, 10017, 10981, 12331, 12524, 12524, 12524,
   12524,  3071,  3071,  3071,  3071,  3071,  1911,  1911,   779,   779,
     779,   295,   295,   295, -1452,   659, 16299,   763,   769, 15880,
     771,   956,     6, 12932,    43,   566,   177, -1452, -1452, -1452,
     952,   709, -1452,   573, 15533, -1452, -1452,  3827,  3827,  3827,
    3827,  3827,  3827,  3827,  3827,  3827,  3827,  3827,  3827,  3827,
   -1452, 12932,   210, -1452,   180, -1452,   566,   362,   774,  3256,
     782,   784,   786,  3301,   129,   787, -1452, 16299,  2805, -1452,
     833, -1452,    60,   379, 15719, 16299, 15719, 15925,   827,    60,
      63,   183, -1452,   169,   824,   797, 12932, -1452,   190, -1452,
   -1452, -1452,  8300,   511, -1452, -1452, 16299, 16299,   191, -1452,
   -1452, -1452, 12932,   887, 15133, 15250,   833,  8879,   799,   802,
   -1452,    83,   906,   866,   849, -1452,   995,   820,  2261,  2633,
   15250, 15250, 15250, 15250, 15250,   826,   862,   829, 15250,    28,
   -1452,   871, -1452,   828, -1452, 16344, -1452,    17, -1452,  5598,
    1321,   834,   423,  1390, -1452,   833,   426,  1410, -1452,   833,
     833, -1452, -1452,  3791, -1452, 16344,  1010, 15719,   837, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,    92,   833,
    1321,   840, 15436, 15626,  1016, -1452, -1452, -1452, -1452,   839,
   -1452, 12932, -1452, -1452,  4690, -1452,  2633,  1321,   845, -1452,
   -1452, -1452, -1452,  1022,   852, 12932,  4177, -1452, -1452, 13287,
     855, -1452,  2633, -1452,   858,  5791,  1015,   137, -1452, -1452,
     118,  1031, -1452,  1039, -1452,  2633, -1452, -1452,    63, 16299,
   -1452, 10423, -1452, 15250,   123,   861,  1321,   795, -1452, -1452,
   16504, 12932, -1452, -1452, 12932, -1452, 12932, -1452,  4054,   868,
   10037,   776,  1025,   795,  2633,  1032,   827,   833,  4447,    63,
    4467,   872, -1452, -1452,   179,   876, -1452, -1452,  1038,  1166,
    1166,  2805, -1452, -1452, -1452,  1014,   892,   299,   893, -1452,
   -1452, -1452, -1452,  1071,   897,   665,    63,    63, 12739,   795,
    1039, -1452, -1452,  4632,   520,   191,  9458, -1452,  5984,   900,
    6177,   901, 15133, 15719,   904,   959,    63, 16344,  1078, -1452,
   -1452, -1452, -1452,   498, -1452,    44,  2633, -1452,   963,  2633,
     833,   650, -1452, -1452, -1452,  1087, -1452,   911,   938,   658,
     658,  1034,  1034, 16027,   908,  1093, 15250, 14940,  4177, 14617,
   14807, 15250, 15250, 15250, 15250, 15040, 15250, 15250, 15250, 15250,
   15250, 15250, 15250, 15250, 15250, 15250, 15250, 15250, 15250, 15250,
   15250, 15250, 15250, 15250, 15250, 15250, 15250, 15250, 15250,   833,
   -1452, -1452,  1023, -1452, -1452,   833, -1452, -1452,   833, -1452,
   -1452, -1452, -1452, 15250,    63, -1452,   514, -1452,   504,  1095,
   -1452, -1452,   130,   922,    63, 10616, -1452,  2533, -1452,  5019,
     709,  1095, -1452,   506,    20, -1452, 16299,   986,   949, -1452,
     951,  1015, -1452,  2633,   766,  2633,    53,  1126,  1061,   279,
   -1452,   566,   306, -1452, -1452, 15719, 12932, 16299, 16344,   955,
     123, -1452,   984,   123,   958, 16504, 16299, 15982,   988, 10037,
     989,   993,  2633,  1002,   957,  2633,   795, -1452,   586,   365,
   10037, 12932, -1452, -1452, -1452, -1452, -1452, -1452,  1052,  1011,
    1194,  1118,  2805,  1058, -1452,  4177,  2805, -1452, -1452, -1452,
   15719, 16299,  1019, -1452,   191,  1179,  1143,  9458, -1452, -1452,
   -1452,  1030, 12932,   959,    63, 15436, 15133,  1036, 15250,  6370,
     654,  1033, 12932,   112,    49, -1452,  1051,  2633, -1452,  1094,
   -1452,  2298,  1197,  1047, 15250, -1452, 15250, -1452,  1053, -1452,
    1106,  1236,  1060, -1452, -1452, -1452, 16084,  1063,  1239, 16386,
   16468,  4420, 15250,  4969, 16610, 15600, 10403, 12139, 12717, 13421,
   13421, 13421, 13421,  3135,  3135,  3135,  3135,  3135,  1919,  1919,
     658,   658,   658,  1034,  1034,  1034,  1034, -1452,  1064, -1452,
   -1452, -1452, 16344,   833,  2633,  2633, -1452,  1321,   748, -1452,
   15436, -1452, -1452,  3827,  1065, -1452,  1069,  1553, -1452,    56,
   12932, -1452, -1452, -1452, 12932, -1452, 12932, -1452,   766, -1452,
   -1452,   171,  1240,  1180, 12932, -1452,  1073,    63, 16299,  1015,
    1074, -1452,  1076,   123, 12932, 10037,  1077, -1452, -1452,   709,
   -1452, -1452,  1085,  1086,  1088, -1452,  1090,  2805, -1452,  2805,
   -1452, -1452,  1091, -1452,  1155,  1102,  1281, -1452,    63, -1452,
    1262, -1452,  1114, -1452, -1452,  1116,  1122,   131, -1452, -1452,
   16344,  1123,  1125, -1452, 10794, -1452, -1452, -1452, -1452, -1452,
   -1452,  2633, -1452,  2633, -1452, 16344, 16129, -1452, 15250,  4177,
   -1452, -1452, 15250, -1452, 15250, -1452, 16540, 15250,  1115,  6563,
     504, -1452, -1452, -1452,   482, 13972,  1321,  1206, -1452,  1483,
    1145,   488, -1452, -1452, -1452,   733,  3421,   107,   110,  1127,
     709,   702,   132, -1452, -1452, -1452,  1160,  4878,  4924, 16299,
   -1452,    76,  1303,  1241, 12932, -1452, 16299, 10037,  1219,  1015,
    1774,  1015,  1146, 16299,  1147, -1452,  1823,  1148,  1901, -1452,
   -1452,   123, -1452, -1452,  1203, -1452,  2805, -1452,  4177, -1452,
    1992, -1452,  8300, -1452, -1452, -1452, -1452,  9072, -1452, -1452,
   -1452,  8300, -1452,  1153, 15250, 16344,  1207, 16344, 16186, 16540,
   -1452, -1452, -1452,  1321,  1321,   833, -1452,  1331, 14940,    85,
   -1452, 13972,   709,  1653, -1452,  1174, -1452,   111,  1159,   113,
   -1452, 14277, -1452, -1452, -1452,   114, -1452, -1452,  1137, -1452,
    1162, -1452,  1271,   573, -1452, 14110, -1452, 14110, -1452, -1452,
    1342,   733, -1452, 13558, -1452, -1452, -1452, -1452,  1343,  1280,
   12932, -1452, 16299,  1175,  1178,  1176,   447, -1452,  1219,  1015,
   -1452, -1452, -1452, -1452,  2060,  1182,  2805, -1452,  1238, -1452,
    8300,  9265,  9072, -1452, -1452, -1452,  8300, -1452, 16344, 15250,
   15250,  6756,  1183,  1184, -1452, 15250, -1452,  1321, -1452, -1452,
   -1452, -1452, -1452,  2633,  2472,  1483, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452,   516, -1452,  1145,
   -1452, -1452, -1452, -1452, -1452,   102,   355, -1452,  1360,   115,
   14674,  1271,  1361, -1452,  2633,   573, -1452, -1452,  1188,  1363,
   12932, -1452, 16299, -1452,   146, -1452, -1452, -1452, -1452,  1199,
     447, 13696, -1452,  1015, -1452,  2805, -1452, -1452, -1452, -1452,
    6949, 16344, 16344, -1452, -1452, -1452, 16344, -1452,   581,   134,
    1374,  1200, -1452, -1452, 15250, 14277, 14277,  1336, -1452,  1137,
    1137,   473, -1452, -1452, -1452, 15250,  1314, -1452,  1224,  1212,
     116, 15250, -1452,   833, -1452, 15250, 16299,  1323, -1452,  1392,
    7142,  7335, -1452, -1452, -1452,   447, -1452,  7528,  1216,  1297,
   -1452,  1311,  1260, -1452, -1452,  1315,  2633, -1452,  2472, -1452,
   -1452, 16344, -1452, -1452,  1249, -1452,  1386, -1452, -1452, -1452,
   -1452, 16344,  1408,   514, -1452, -1452, 16344,  1237, 16344, -1452,
     324,  1243, -1452, -1452,  7721, -1452,  1244, -1452,  1242,  1255,
     833,   702,  1252, -1452, -1452, 15250,   135,   136, -1452,  1346,
   -1452, -1452, -1452, -1452,  1321,   834, -1452,  1265,   833,  1537,
   -1452, 16344, -1452,  1246,  1426,   648,   136, -1452,  1356, -1452,
    1321,  1250, -1452,  1015,   138, -1452, -1452, -1452, -1452,  2633,
   -1452,  1253,  1257,   117, -1452,   464,   648,   172,  1015,  1258,
   -1452, -1452, -1452, -1452,  2633,    77,  1432,  1369,   464, -1452,
    7914,   175,  1436,  1371, 12932, -1452, -1452,  8107, -1452,   246,
    1439,  1375, 12932, -1452, 16299, -1452,  1440,  1376, 12932, -1452,
   16299, 12932, -1452, 16299, 16299
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1452, -1452, -1452,  -535, -1452, -1452, -1452,   437,     2,   -46,
   -1452, -1452, -1452,   899,   647,   642,    -3,  1513,  2896, -1452,
    2634, -1452,   367, -1452,    75, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452,  -262, -1452, -1452,  -147,
      23,    18, -1452, -1452, -1452, -1452, -1452, -1452,    24, -1452,
   -1452, -1452, -1452, -1452, -1452,    26, -1452, -1452,  1009,  1018,
    1021,   -86,  -658,  -820,   556,   621,  -269,   342,  -879, -1452,
      30, -1452, -1452, -1452, -1452,  -692,   193, -1452, -1452, -1452,
   -1452,  -241, -1452,  -546, -1452,  -413, -1452, -1452,   934, -1452,
      36, -1452, -1452,  -989, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452,     8, -1452,    95, -1452, -1452, -1452,
   -1452, -1452,   -74, -1452,   186,  -970, -1452, -1451,  -258, -1452,
    -145,    61,  -101,  -245, -1452,   -73, -1452, -1452, -1452,   192,
     -16,     9,    29,  -696,   -69, -1452, -1452,    -6, -1452, -1452,
      -5,   -38,    65, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452,  -576,  -803, -1452, -1452, -1452, -1452, -1452,  1152,
   -1452, -1452, -1452, -1452, -1452,   458, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452,  -951, -1452,  2066,    27, -1452,   615,
    -380, -1452, -1452,  -464,  3407,  2995, -1452, -1452,   532,  -160,
    -624, -1452, -1452,   599,   409,  -687,   410, -1452, -1452, -1452,
   -1452, -1452,   588, -1452, -1452, -1452,   126,  -834,  -126,  -396,
    -394, -1452,   655,  -110, -1452, -1452,    39,    40,   564, -1452,
   -1452,   270,   -15, -1452,  -337,     5,  -341,   157,  -278, -1452,
   -1452,  -444,  1198, -1452, -1452, -1452, -1452, -1452,   743,  1344,
   -1452, -1452, -1452,  -327,  -653, -1452,  1134,  -858, -1452,   -62,
    -159,   -58,   767, -1452,  -985,   218,  -143,   510,   577, -1452,
   -1452, -1452, -1452,   531,  1825, -1037
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -969
static const yytype_int16 yytable[] =
{
     174,   176,   457,   178,   179,   180,   182,   183,   320,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   411,   117,   212,   215,   485,   892,   116,   119,   371,
     120,   382,   615,   374,   375,   229,   614,   239,  1241,   748,
     734,   236,   479,  1075,   873,   247,   616,   250,   915,   327,
     328,   942,   331,   241,   502,   337,   319,   222,   245,   407,
     507,   456,   384,  1067,   684,   122,   854,   730,   731,   227,
     228,   238,   411,  1227,   239,   874,   961,  1092,   367,   115,
     723,   368,   724,  1143,   381,   386,  1238,   975,   776,   401,
     949,   519,   976,  1103,  1487,   383,   755,   -30,   -65,   562,
     778,  1046,   -30,   -65,   511,   -29,   567,  1346,   357,   572,
     -29,  1639,   519,   813,   817,   758,  1435,   759,   477,  1437,
    -343,   512,  1495,  1579,  1646,  1646,  1487,   477,   827,  1132,
     147,   519,   384,  1242,  -546,  1640,   843,    13,   843,   843,
     843,   843,  1295,  1676,  1046,   410,  1152,  1153,   564,   346,
    -941,  1152,  1153,  1076,   381,   386,  1448,  1772,   550,  1663,
     590,   338,   474,   475,   496,   383,  -655,   753,  1029,    13,
     688,   398,   488,   494,     3,  -544,   177,  1233,   -92,  -941,
     505,  1657,    13,    13,    13,  -941,   495,   386,  -941,   398,
     474,   475,   -92,   474,   475,  1170,   486,   383,  1077,   728,
     398,  -837,   504,  -520,   732,   397,  1351,  1765,   458,  1234,
    1779,   232,   372,   383,  1705,   -91,   551,  -941,  1243,  -824,
    -656,  -825,   853,   474,   475,  -547,  1658,   514,  -827,   -91,
     514,  -866,   591,  -546,  1155,   360,   482,   239,   525,  1298,
     478,  1449,  1773,   198,  -832,  -831,  -830,  -828,  -826,   478,
    -869,  1352,  1766,   481,   833,  1780,   977,  -868,  1106,   779,
     389,   536,  1360,   656,  1358,  1047,   777,  1288,   402,  1366,
     520,  1368,   339,  1488,  1489,  -282,   -30,   -65,   563,   546,
    1150,   860,  1154,  1078,   -29,   568,  1263,   482,   573,  1641,
    1380,   589,   814,   818,  -836,  1436,   209,   209,  1438,  -343,
    1296,  1496,  1580,  1647,  1695,  1760,   828,  1677,  1732,   829,
    -282,   516,   578,   329,   844,   521,   928,  1219,  1386,  1442,
    1059,  -744,  1274,  -266,  -744,  -744,  1786,   319,  -663,   556,
     560,   561,  -662,   741,   577,   581,  1353,  1767,  -835,   735,
    1781,  -824,   654,  -825,   239,   383,  -811,  -834,  1089,   444,
    -827,   212,   481,  -866,   103,   320,   595,   411,   483,  1719,
     372,   445,   600,   910,  -838,  -841,  -832,  -831,  -830,  -828,
    -826,  -657,  -869,  -812,  1456,   182,  1458,   235,   576,  -868,
     342,  1464,  -799,   639,  1642,  1123,   861,   343,   815,   344,
     705,   371,   700,   701,   407,   651,  -799,   345,  1052,   242,
     358,   862,  1643,   319,  1720,  1644,   597,  1226,   198,   483,
     116,  1787,   243,   657,   658,   659,   661,   662,   663,   664,
     665,   666,   667,   668,   669,   670,   671,   672,   673,   674,
     675,   676,   677,   678,   679,   680,   681,   682,   683,  1277,
     685,   108,   686,   686,   689,   397,  1285,  1124,   122,   229,
     819,   867,   694,   706,   707,   708,   709,   710,   711,   712,
     713,   714,   715,   716,   717,   718,   719,  1407,  -811,   361,
     362,   222,   686,   729,  1610,   651,   651,   686,   733,  -548,
     703,   209,   707,   227,   228,   737,   376,  1082,   248,  1083,
     244,   318,   820,   457,   745,  -812,   747,  1100,  1240,   319,
     358,   881,  1688,   883,   651,   625,  1396,   397,   352,   474,
     475,   765,   766,   752,   767,   642,   358,   388,   348,   103,
    1689,  1035,   597,  1690,  1038,   615,   369,   349,   352,   614,
    1634,   907,   352,   352,  -802,   350,   909,  -800,  1108,   616,
     397,   412,   474,   475,  1250,   351,  1635,  1252,  -802,   397,
     812,  -800,   456,  1490,   816,   355,   917,   356,   352,   337,
     823,   836,   837,  -658,  1636,  -839,    36,   921,   151,   361,
     362,   864,   373,   602,   603,  1468,   396,  1592,   397,  1593,
    1373,   397,  1374,   955,   956,   361,   362,    48,   260,   377,
     208,   210,  1134,  1135,   899,   378,   397,   770,  -839,  1151,
    1152,  1153,  1214,  1215,   383,   400,  1367,   696,   403,   209,
    1403,  1404,  1427,   358,   447,   262,   881,   883,   209,   390,
     580,   358,   492,   950,   883,   209,   982,   393,  1665,   448,
     641,   413,   209,   727,   358,  1606,  1607,    36,   414,   951,
     507,   206,   206,   889,  1265,   415,    86,    87,  1030,    88,
     169,    90,  1761,  1762,   696,   900,   416,   615,    48,    36,
     417,   614,  1350,  1228,   385,   754,   538,   449,   760,  1686,
    1687,   616,   450,   108,  1428,   480,  1229,   108,  -833,   782,
      48,   526,   361,   362,  1682,  1683,  1256,  1440,   908,  1467,
     361,   362,  -545,   532,   533,  1230,  -656,  1266,   484,   458,
     358,  1287,   592,   361,   362,   365,   359,  1362,  1025,  1026,
    1027,   168,  1757,   489,    84,   312,   920,    86,    87,   491,
      88,   169,    90,   358,  1028,   445,   358,  1771,    36,   597,
     985,   988,   597,   168,   385,   316,    84,   783,   398,    86,
      87,   497,    88,   169,    90,   317,  -837,   625,   500,    48,
     501,   953,  1324,   481,   209,  1292,  1152,  1153,  1755,  1491,
    -654,   508,   545,  1746,  1747,  1748,   385,   239,   360,   361,
     362,  1670,   509,  1768,   517,   498,  1342,   959,  -968,  1614,
     530,   537,   506,  1127,   615,   116,   540,   541,   614,   547,
     569,   598,   361,   362,    13,   361,   362,   570,   616,   571,
     151,   582,   168,   583,   151,    84,   617,   618,    86,    87,
     556,    88,   169,    90,   560,   627,   628,    60,    61,    62,
     164,   165,   408,   122,   108,  1465,   206,   629,  1163,   441,
     442,   443,   116,   444,   643,  1167,   631,   640,   318,  -117,
      53,   352,  1364,   653,   740,   445,  1057,   392,   394,   395,
     742,   738,   970,   592,   743,  1107,  1325,   749,   768,   750,
    1066,  1326,   519,    60,    61,    62,   164,  1327,   408,  1328,
     122,   694,   117,   772,   775,   536,   788,   116,   119,   789,
     120,   810,   762,  1735,   409,   826,  1087,   545,   352,   698,
     352,   352,   352,   352,   830,   831,  1095,   834,   116,  1096,
     835,  1097,  1735,   566,   840,   651,  1329,  1330,   842,  1331,
    1756,    36,   574,   722,   579,   122,   809,   209,   845,   586,
     856,   851,  1246,   229,   857,    36,   596,   859,  1666,   115,
     409,  -678,    48,   865,   545,   866,   122,   868,  1332,   869,
     877,   872,   876,  1131,  1453,   222,    48,   885,   757,   890,
     887,   151,   891,   893,   206,   896,   902,   227,   228,   108,
    1270,   905,   903,   206,  1137,   906,   914,   922,   615,   924,
     206,   925,   614,   898,   209,  -660,    36,   206,   807,   926,
     147,   116,   616,   116,   952,   962,  1221,   972,   613,   875,
     974,    86,    87,   978,    88,   169,    90,    48,   979,  1169,
     980,   822,  1175,   625,   981,    86,    87,   983,    88,   169,
      90,  1310,   996,   997,   209,   998,   209,  1001,  1315,   122,
     625,   122,  1000,  1033,  1043,  1045,  1068,   848,   850,  1051,
    1055,  1138,  1056,   653,  1062,  1701,  1064,   615,   727,  1065,
     760,   614,  1071,  1069,   209,  1073,  1105,  1090,   586,  1222,
    1223,   616,  1111,   365,  1099,  1102,    86,    87,  1110,    88,
     169,    90,  -840,  1121,   487,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   117,  1122,  1125,
    1126,  1248,   116,   119,  1128,   120,   151,   366,  1140,  1142,
    1145,  1146,  1148,   352,   651,  1157,  1161,   209,  1162,   206,
    1028,  1165,  1166,  1208,  1217,   651,  1223,   760,  1220,    36,
    1745,   198,   209,   209,   472,   473,  1379,    36,  1236,   198,
     122,    60,    61,    62,    63,    64,   408,   418,   419,   420,
      48,   958,    70,   451,   115,   910,   960,   239,    48,  1237,
    1244,  1245,  1249,  1280,  1253,  1261,   421,  1294,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     453,   444,   116,  1251,  1255,   939,  1257,   945,   204,   204,
     474,   475,  1258,   445,  1267,   147,    36,   720,   409,    86,
      87,  1260,    88,   169,    90,   720,  1441,    86,    87,   108,
      88,   169,    90,  1269,  1268,   935,  1273,    48,  1279,  1281,
     122,   888,  1283,   968,   108,    36,   625,  1282,  1284,   625,
     721,  1293,   103,  1289,  1299,  1347,  1301,  1303,   756,  1348,
     103,  1349,   209,   209,  1304,   411,    48,   630,  1308,  1356,
    1307,  1112,  1113,  1114,    36,  1309,   108,  1311,  1314,  1363,
     651,  1318,  1037,  1313,  1354,  1343,  1040,  1041,  1344,  1357,
    1355,  1359,   206,  1361,  1365,    48,    86,    87,   919,    88,
     169,    90,    53,  1369,  1371,  1370,  1048,  1372,  1375,  1336,
      60,    61,    62,   164,   165,   408,  1590,  1376,  1336,  1377,
    1378,   108,  1381,  1581,   898,    86,    87,  1582,    88,   169,
      90,    36,  1383,  1384,  1400,  1424,   545,   852,   946,  1385,
     947,  1388,   108,  1389,  1411,  1439,  1444,  1450,   722,   206,
     757,  1451,    48,  1428,    86,    87,   151,    88,   169,    90,
     332,   333,  1454,  1459,  1460,  1466,  1149,  1462,   966,  1479,
    1477,   151,   116,  1484,  1341,  1485,  1493,   409,  1494,  1452,
    1587,  1588,   651,  1341,   352,   209,  1594,  1600,  1433,   206,
    1601,   206,  1603,   204,  1604,  1605,  1115,  1115,   939,  1613,
    1615,  1624,  1625,   151,  1645,  1651,  1654,  1655,   334,   625,
     122,    86,    87,  1678,    88,   169,    90,   757,  1662,   206,
    1680,  1044,  1684,   108,  1692,   108,  1693,   108,  1694,    36,
     209,  1700,   458,  1699,  1707,   116,   586,  1054,  1708,  -339,
    1486,  1710,  1714,  1711,   116,   209,   209,  1159,   151,  1640,
      48,  1336,  1715,  1718,  1725,  1730,  1737,  1336,  1724,  1336,
    1721,  1740,  1723,  1743,   545,  1744,  1752,   545,  1754,   151,
    1758,  1336,   206,   122,  1759,  1602,  1774,  1769,  1653,  1775,
    1782,  1783,   122,  1788,  1791,  1789,  1792,   206,   206,  1039,
    1036,   821,  1474,  1739,  1595,   702,   807,  1101,    36,   697,
    1679,  1753,  1210,   168,   699,  1211,    84,    85,  1061,    86,
      87,   613,    88,   169,    90,  1286,  1341,  1390,    36,    48,
     209,   204,  1341,   116,  1341,  1751,   108,   625,  1611,   116,
     204,   824,  1619,  1633,   116,  1638,  1341,   204,  1492,    48,
    1776,  1764,  1413,    36,   204,   846,   847,  1431,  1650,  1412,
     151,  1231,   151,  1609,   151,  1336,   966,  1144,  1168,  1117,
    1271,   122,  1272,  1129,    48,   652,  1081,   122,  1402,   203,
     203,    36,   122,   219,  1648,   553,  1617,  1474,    86,    87,
    1716,    88,   169,    90,   588,   986,  1213,  1324,  1160,   939,
    1207,    36,    48,   939,     0,   557,  1728,   219,    86,    87,
       0,    88,   169,    90,   108,     0,  1319,   206,   206,     0,
       0,     0,    48,     0,     0,     0,   108,     0,     0,     0,
    1341,  1697,   319,    86,    87,  1656,    88,   169,    90,    13,
       0,     0,     0,     0,  1414,     0,     0,     0,    36,     0,
       0,     0,     0,   613,     0,     0,     0,  1415,  1416,   405,
       0,    86,    87,   151,    88,   169,    90,     0,   411,    48,
       0,     0,     0,     0,     0,   168,   204,     0,    84,  1417,
       0,    86,    87,   116,    88,  1418,    90,     0,  1382,  1247,
    1320,     0,    60,    61,    62,   164,   165,   408,     0,     0,
       0,  1325,     0,     0,  1391,     0,  1326,     0,    60,    61,
      62,   164,  1327,   408,  1328,     0,     0,     0,     0,     0,
       0,   122,     0,   116,   116,   334,     0,     0,    86,    87,
     116,    88,   169,    90,  1278,     0,     0,     0,     0,     0,
     206,   151,     0,   593,   939,     0,   939,   599,     0,   586,
     966,  1329,  1330,   151,  1331,     0,     0,     0,     0,   409,
       0,   122,   122,     0,   203,  1742,     0,   116,   122,     0,
       0,    36,     0,     0,   593,   409,   599,   593,   599,   599,
     613,     0,     0,  1345,     0,   206,     0,     0,  1470,     0,
       0,     0,    48,     0,     0,     0,   108,  1476,     0,     0,
     206,   206,   318,     0,     0,   122,     0,  1481,  1429,  1784,
       0,     0,  1729,   219,  1414,   219,     0,  1790,  1324,     0,
       0,     0,     0,  1793,   586,     0,  1794,  1415,  1416,     0,
       0,     0,     0,   116,     0,     0,     0,     0,     0,   204,
     116,     0,     0,     0,     0,   168,     0,     0,    84,    85,
       0,    86,    87,   939,    88,  1418,    90,     0,     0,   108,
      13,     0,     0,     0,   108,     0,     0,  1324,   108,     0,
     219,   122,     0,     0,     0,   206,     0,     0,   122,     0,
    1620,     0,   352,   625,     0,   545,     0,     0,   318,     0,
       0,     0,   203,     0,     0,     0,   204,     0,  1576,     0,
       0,   203,   625,     0,     0,  1583,     0,     0,   203,    13,
     625,     0,   318,     0,   318,   203,     0,     0,     0,   325,
     318,     0,  1325,   151,     0,     0,   219,  1326,     0,    60,
      61,    62,   164,  1327,   408,  1328,   204,     0,   204,     0,
       0,     0,     0,   939,     0,  1324,     0,   108,   108,   108,
       0,   219,     0,   108,   219,     0,     0,     0,   108,     0,
       0,     0,     0,     0,   613,     0,   204,     0,     0,     0,
       0,  1325,  1329,  1330,     0,  1331,  1326,     0,    60,    61,
      62,   164,  1327,   408,  1328,     0,   151,    13,     0,     0,
       0,   151,     0,     0,     0,   151,   409,   219,   438,   439,
     440,   441,   442,   443,  1457,   444,  1022,  1023,  1024,  1025,
    1026,  1027,  1660,     0,  1661,     0,     0,   445,     0,   204,
       0,  1329,  1330,  1667,  1331,  1028,     0,     0,     0,     0,
       0,     0,     0,   613,   204,   204,  1324,   203,     0,     0,
       0,     0,     0,     0,     0,   409,     0,     0,     0,  1325,
       0,     0,     0,  1461,  1326,     0,    60,    61,    62,   164,
    1327,   408,  1328,     0,     0,     0,     0,   545,     0,  1704,
       0,     0,     0,     0,   151,   151,   151,     0,    13,     0,
     151,     0,     0,     0,     0,   151,     0,     0,   318,   219,
     219,     0,   939,   799,     0,     0,     0,   108,     0,  1329,
    1330,     0,  1331,     0,  1324,  1671,     0,     0,     0,     0,
       0,     0,  1576,  1576,     0,     0,  1583,  1583,     0,     0,
       0,     0,     0,   409,   799,   534,     0,   535,     0,     0,
     352,  1463,   205,   205,     0,     0,   221,   108,   108,     0,
    1325,     0,     0,     0,   108,  1326,    13,    60,    61,    62,
     164,  1327,   408,  1328,   204,   204,   487,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,  1770,
     219,   219,     0,     0,     0,     0,  1777,     0,     0,   219,
       0,   108,   539,     0,     0,     0,     0,  1727,     0,     0,
    1329,  1330,     0,  1331,     0,     0,     0,     0,     0,     0,
     203,     0,     0,     0,     0,  1741,   472,   473,  1325,     0,
       0,     0,     0,  1326,   409,    60,    61,    62,   164,  1327,
     408,  1328,  1469,     0,   151,     0,     0,     0,     0,     0,
       0,     0,     0,   487,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,     0,   108,     0,     0,
       0,     0,     0,     0,   108,     0,     0,   203,  1329,  1330,
       0,  1331,     0,   646,   151,   151,   325,     0,     0,     0,
       0,   151,   474,   475,     0,     0,     0,   204,     0,     0,
       0,     0,   409,   472,   473,     0,     0,     0,     0,     0,
    1612,     0,     0,     0,     0,     0,     0,   203,     0,   203,
       0,     0,     0,     0,     0,     0,     0,     0,   151,    60,
      61,    62,    63,    64,   408,     0,     0,   205,     0,     0,
      70,   451,   204,     0,     0,     0,     0,   203,   799,   751,
     260,     0,     0,     0,     0,     0,     0,   204,   204,     0,
       0,   219,   219,   799,   799,   799,   799,   799,     0,   474,
     475,   799,     0,     0,     0,     0,   452,   262,   453,     0,
       0,     0,     0,   219,     0,     0,     0,   260,     0,     0,
       0,   454,     0,   455,   151,     0,   409,     0,     0,    36,
     203,   151,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,   262,   203,   203,     0,     0,     0,
      48,   781,     0,     0,     0,     0,   832,     0,     0,   219,
     219,     0,   204,     0,     0,     0,    36,     0,     0,   219,
       0,     0,     0,     0,     0,   219,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   532,   533,    48,   219,     0,
       0,     0,     0,     0,     0,   205,   799,     0,     0,   219,
       0,     0,     0,   168,   205,     0,    84,   312,     0,    86,
      87,   205,    88,   169,    90,     0,   984,   219,   205,     0,
       0,   219,   532,   533,     0,     0,     0,   316,     0,   205,
       0,     0,   870,   871,     0,     0,     0,   317,     0,     0,
     168,   879,     0,    84,   312,     0,    86,    87,     0,    88,
     169,    90,     0,  1302,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   316,   203,   203,     0,     0,     0,
       0,     0,     0,     0,   317,     0,     0,     0,     0,   219,
       0,     0,   219,     0,   219,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   799,
     221,   219,     0,     0,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,     0,   418,   419,   420,     0,    33,    34,    35,
     205,     0,     0,     0,     0,     0,   799,     0,     0,   199,
       0,     0,   421,     0,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   219,   444,   219,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   203,   445,
       0,     0,     0,     0,     0,     0,   803,     0,    74,    75,
      76,    77,    78,   646,   646,   219,     0,     0,   219,   201,
       0,     0,     0,     0,     0,    82,    83,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   803,   219,    92,
       0,     0,     0,   203,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,     0,     0,   203,   203,
       0,   799,   260,     0,     0,     0,     0,     0,     0,     0,
     219,     0,     0,     0,   219,     0,     0,   799,     0,   799,
       0,  1060,     0,     0,     0,     0,     0,     0,   321,   262,
       0,     0,     0,     0,     0,   799,     0,  1070,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1084,    36,     0,   205,     0,     0,     0,     0,     0,   418,
     419,   420,     0,     0,     0,     0,  1224,   219,   219,     0,
     219,     0,    48,   203,     0,     0,     0,     0,   421,  1104,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,     0,     0,     0,   532,   533,     0,
     205,     0,     0,     0,     0,   445,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   168,     0,     0,    84,   312,
       0,    86,    87,     0,    88,   169,    90,     0,     0,     0,
       0,  1156,     0,     0,  1158,     0,     0,     0,     0,   316,
     205,     0,   205,     0,   219,     0,   219,     0,     0,   317,
       0,   799,   219,     0,     0,   799,     0,   799,     0,     0,
     799,     0,     0,     0,     0,     0,     0,     0,   219,   219,
     205,   803,   219,     0,     0,     0,     0,     0,     0,   219,
       0,     0,   929,   930,     0,     0,   803,   803,   803,   803,
     803,     0,     0,     0,   803,     0,     0,     0,     0,     0,
       0,     0,   931,     0,     0,     0,  1032,     0,     0,     0,
     932,   933,   934,    36,     0,     0,     0,     0,     0,     0,
       0,   219,   935,   205,   321,     0,   321,     0,  1239,   884,
     879,     0,     0,     0,    48,     0,  1050,   799,   205,   205,
       0,     0,     0,     0,     0,     0,   219,   219,     0,     0,
       0,     0,     0,  1050,   219,     0,   219,  1259,     0,     0,
    1262,     0,   205,     0,     0,     0,     0,     0,     0,   936,
       0,     0,     0,     0,     0,     0,     0,     0,   219,     0,
     219,   321,   937,     0,     0,     0,   219,     0,     0,   803,
       0,     0,  1091,    86,    87,     0,    88,   169,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   938,  1300,     0,   221,     0,  1084,     0,     0,     0,
       0,     0,   799,   799,     0,     0,     0,     0,   799,     0,
     219,     0,     0,     0,     0,     0,   219,     0,   219,     0,
       0,     0,     0,     0,     0,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   205,   205,
       0,     0,   321,     0,     0,   321,     0,     0,     0,  1321,
    1322,     0,     0,     0,     0,     0,   487,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,     0,
       0,     0,   803,     0,   205,   472,   473,   803,   803,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   803,   803,   803,   803,   803,   803,   803,   803,   803,
     803,   803,   803,   803,   803,     0,   472,   473,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   219,     0,   803,
       0,  -969,  -969,  -969,  -969,  -969,   436,   437,   438,   439,
     440,   441,   442,   443,   219,   444,  1392,     0,  1393,     0,
       0,   474,   475,     0,     0,     0,     0,   445,     0,     0,
       0,   219,     0,     0,     0,     0,     0,   799,     0,     0,
       0,   205,     0,     0,     0,     0,     0,     0,   799,     0,
       0,  1434,   474,   475,   799,     0,     0,     0,   799,     0,
     321,   784,     0,     0,   801,  -969,  -969,  -969,  -969,  -969,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,     0,   219,
       0,   205,     0,     0,     0,     0,   205,     0,     0,     0,
       0,  1028,     0,     0,     0,   801,     0,     0,     0,     0,
       0,   205,   205,     0,   803,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   799,     0,
     803,     0,   803,     0,     0,     0,     0,   219,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   803,     0,
       0,   321,   321,   219,     0,     0,     0,     0,     0,     0,
     321,     0,   219,     0,     0,     0,   418,   419,   420,     0,
       0,     0,     0,     0,     0,     0,     0,   219,     0,     0,
       0,     0,     0,  1323,     0,   421,   205,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,     0,
     444,   418,   419,   420,     0,     0,     0,     0,  1629,     0,
       0,     0,   445,     0,     0,     0,     0,     0,     0,     0,
     421,     0,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
       0,     0,     0,     0,   803,   205,     0,     0,   803,     0,
     803,     0,     0,   803,     0,     0,     0,     0,     0,     0,
       0,     0,  1410,     0,     0,  1423,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   801,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1652,
       0,     0,   321,   321,   801,   801,   801,   801,   801,     0,
       0,     0,   801,   207,   207,     0,   800,   225,     0,     0,
       0,     0,     0,     0,   205,     0,   923,     0,     0,     0,
     260,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     803,     0,     0,     0,     0,     0,     0,   800,     0,  1482,
    1483,     0,     0,     0,     0,     0,     0,   262,     0,  1423,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     321,   927,     0,     0,     0,     0,     0,     0,     0,    36,
       0,  1712,     0,     0,     0,     0,   321,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   321,
      48,     0,     0,     0,     0,     0,     0,   801,  -386,     0,
       0,     0,     0,     0,     0,   805,    60,    61,    62,   164,
     165,   408,     0,     0,     0,   803,   803,     0,   321,     0,
       0,   803,     0,  1627,     0,   532,   533,     0,     0,     0,
       0,  1423,     0,     0,     0,     0,   825,     0,     0,     0,
       0,     0,     0,   168,   879,     0,    84,   312,     0,    86,
      87,     0,    88,   169,    90,     0,     0,     0,     0,   879,
       0,     0,     0,     0,     0,     0,     0,   316,     0,     0,
       0,     0,     0,   409,     0,     0,     0,   317,     0,     0,
     321,     0,     0,   321,     0,   784,     0,     0,   207,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     801,     0,     0,     0,     0,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,     0,     0,     0,     0,     0,     0,     0,
       0,   800,     0,     0,     0,     0,     0,   801,     0,     0,
       0,     0,     0,     0,     0,     0,   800,   800,   800,   800,
     800,     0,     0,     0,   800,     0,     0,     0,     0,     0,
     803,     0,     0,     0,     0,     0,     0,   321,     0,   321,
       0,   803,     0,     0,     0,     0,     0,   803,     0,     0,
       0,   803,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   321,     0,     0,   321,
       0,     0,     0,     0,     0,     0,   207,     0,     0,     0,
       0,     0,     0,     0,     0,   207,     0,     0,     0,     0,
       0,     0,   207,     0,     0,     0,     0,     0,     0,   207,
     967,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     225,   803,   801,     0,     0,   989,   990,   991,   992,   800,
    1738,   321,     0,   999,     0,   321,     0,     0,   801,     0,
     801,   418,   419,   420,     0,     0,  1410,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   801,     0,     0,     0,
     421,     0,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,   321,   321,
       0,   225,     0,     0,     0,     0,   421,   445,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,     0,     0,     0,     0,     0,  1088,     0,
       0,   207,   800,   445,     0,     0,     0,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,   800,   800,   800,   800,   800,
     800,   800,   800,   800,   800,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   321,     0,   321,     0,   800,
       0,     0,   801,     0,     0,     0,   801,   804,   801,     0,
       0,   801,     0,     0,     0,     0,     0,     0,     0,   321,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     321,     0,     0,     0,     0,     0,     0,     0,   804,     0,
       0,  1042,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1176,  1179,  1180,  1181,
    1183,  1184,  1185,  1186,  1187,  1188,  1189,  1190,  1191,  1192,
    1193,  1194,  1195,  1196,  1197,  1198,  1199,  1200,  1201,  1202,
    1203,  1204,  1205,  1206,     0,     0,     0,     0,   801,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1212,     0,
       0,     0,     0,     0,   800,   321,     0,     0,     0,     0,
       0,     0,     0,     0,   207,     0,     0,     0,     0,     0,
     800,     0,   800,     0,   418,   419,   420,     0,     0,   321,
       0,   321,     0,     0,     0,     0,     0,   321,   800,     0,
       0,     0,     0,   421,     0,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,     0,
       0,   207,     0,   801,   801,     0,     0,     0,     0,   801,
     445,     0,     0,     0,     0,     0,     0,   321,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1290,     0,     0,     0,     0,     0,     0,
       0,   207,     0,   207,     0,     0,     0,     0,     0,  1305,
       0,  1306,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1316,     0,     0,
       0,   207,   804,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   804,   804,   804,
     804,   804,     0,     0,   800,   804,     0,     0,   800,     0,
     800,     0,     0,   800,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   321,     0,
       0,     0,     0,     0,   207,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1098,   321,     0,     0,     0,   207,
     207,     0,    33,    34,    35,    36,     0,   198,     0,     0,
       0,     0,  1672,     0,   608,     0,     0,     0,   801,     0,
       0,     0,     0,   225,     0,     0,    48,     0,     0,   801,
       0,     0,     0,     0,     0,   801,     0,     0,     0,   801,
     800,     0,     0,     0,     0,     0,     0,   200,     0,     0,
     804,     0,     0,  1395,     0,     0,     0,  1397,     0,  1398,
     321,     0,  1399,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   201,   225,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,     0,     0,     0,    92,     0,     0,     0,     0,   801,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,     0,     0,   609,     0,     0,     0,     0,   103,   207,
     207,     0,     0,     0,     0,   800,   800,     0,     0,     0,
       0,   800,     0,   321,     0,     0,     0,     0,     0,  1478,
       0,     0,     0,     0,     0,     0,     0,     0,   321,     0,
       0,     0,     0,   804,     0,   225,     0,     0,   804,   804,
     804,   804,   804,   804,   804,   804,   804,   804,   804,   804,
     804,   804,   804,   804,   804,   804,   804,   804,   804,   804,
     804,   804,   804,   804,   804,   804,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1005,
     804,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,     0,  1621,  1622,     0,   418,   419,   420,
    1626,     0,     0,     0,     0,     0,  1028,     0,     0,     0,
       0,     0,   207,     0,     0,     0,   421,     0,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,    33,    34,    35,    36,     0,   198,     0,     0,
     800,     0,   225,   445,   199,     0,     0,   207,     0,     0,
       0,   800,     0,     0,     0,     0,    48,   800,     0,     0,
       0,   800,   207,   207,     0,   804,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   216,     0,     0,
       0,   804,     0,   804,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,     0,   804,
       0,     0,     0,     0,   201,     0,     0,     0,     0,   168,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   169,
      90,   800,     0,     0,    92,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   207,    97,  1681,
       0,     0,     0,   218,     0,     0,     0,     0,   103,     0,
    1691,     0,   418,   419,   420,     0,  1696,     0,     0,     0,
    1698,     0,     0,     0,     0,     0,     0,  1109,     0,     0,
       0,   421,     0,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,   445,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
    1731,     0,     0,     0,     0,   804,   225,    11,    12,   804,
       0,   804,     0,     0,   804,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,   225,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,    54,    55,
      56,   804,    57,    58,    59,    60,    61,    62,    63,    64,
      65,     0,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,  1133,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,    81,    82,    83,    84,    85,     0,    86,    87,
       0,    88,    89,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,    95,     0,    96,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
    1058,   103,   104,     0,   105,   106,   804,   804,   418,   419,
     420,     0,   804,     0,     0,     0,     0,     0,     0,     0,
       0,  1632,     0,     0,     0,     0,     0,   421,     0,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,     0,   444,     0,   418,   419,   420,     0,     0,     0,
       0,     0,     0,     0,   445,     0,     0,     0,     0,     0,
       0,     0,     0,   421,     0,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,  1002,
    1003,  1004,     0,     0,     0,     0,     0,     0,     0,     0,
     445,     0,     0,     0,     0,     0,     0,     0,  1005,  1317,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,  1028,     0,     0,     0,     0,
       0,   804,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,   804,     0,     0,     0,     0,     0,   804,     0,
       0,     0,   804,     0,     0,    13,    14,    15,  1446,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,  1713,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,  1447,    47,     0,     0,    48,    49,
       0,     0,   804,    50,    51,    52,    53,    54,    55,    56,
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
       0,    96,     0,    97,    98,    99,     0,     0,   100,     0,
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
       0,   100,     0,   101,   102,   632,   103,   104,     0,   105,
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
      98,    99,     0,     0,   100,     0,   101,   102,  1031,   103,
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
     102,  1072,   103,   104,     0,   105,   106,     5,     6,     7,
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
     100,     0,   101,   102,  1139,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,  1141,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
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
      44,     0,    45,     0,    46,     0,    47,  1291,     0,    48,
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
       0,   101,   102,  1401,   103,   104,     0,   105,   106,     5,
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
       0,     0,   100,     0,   101,   102,  1623,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
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
     101,   102,  1702,   103,   104,     0,   105,   106,     5,     6,
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
       0,   100,     0,   101,   102,  1703,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,  1706,    46,     0,    47,     0,     0,    48,    49,     0,
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
     102,  1722,   103,   104,     0,   105,   106,     5,     6,     7,
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
     100,     0,   101,   102,  1778,   103,   104,     0,   105,   106,
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
      99,     0,     0,   100,     0,   101,   102,  1785,   103,   104,
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
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,   515,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    11,    12,     0,   769,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,    11,    12,     0,   969,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,   164,   165,    65,
       0,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,  1473,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,    49,     0,     0,     0,    50,    51,    52,    53,
       0,    55,    56,     0,    57,     0,    59,    60,    61,    62,
     164,   165,    65,     0,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,    79,     0,     0,    80,
       0,     0,     0,     0,   168,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   169,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,     0,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,  1616,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,   380,    12,
       0,     0,     0,     0,     0,     0,   704,     0,     0,     0,
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
       0,     0,    97,    98,    99,     0,     0,   100,     0,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,     0,     0,     0,     0,     0,     0,     0,
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
     170,     0,   326,     0,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,   647,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   445,    14,    15,     0,     0,     0,     0,
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
      90,     0,   648,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   170,     0,     0,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
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
       0,     0,     0,     0,     0,    60,    61,    62,   164,   165,
     166,     0,     0,    67,    68,     0,     0,     0,     0,     0,
       0,     0,     0,   167,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    80,     0,     0,
       0,     0,   168,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   169,    90,     0,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   170,     0,     0,   764,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,  1027,     0,     0,  1085,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1028,
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
       0,    86,    87,     0,    88,   169,    90,     0,  1086,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   170,
       0,     0,     0,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   380,    12,     0,     0,     0,     0,     0,
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
       0,     0,   100,     0,   418,   419,   420,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   421,  1295,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     445,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,   181,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   164,   165,   166,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   167,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,  1296,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   170,     0,     0,     0,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,   211,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,    14,
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
     418,   419,   420,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   421,
       0,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   445,     0,    16,     0,
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
       0,     0,    92,     0,     0,    93,     0,     0,   446,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   170,     0,   246,   419,   420,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   421,     0,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,     0,   444,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,   445,
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
      98,    99,     0,     0,   170,     0,   249,     0,     0,   103,
     104,     0,   105,   106,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   380,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    97,    98,    99,     0,     0,   100,     0,   418,
     419,   420,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   421,     0,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,   445,     0,    16,     0,    17,
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
       0,    92,     0,     0,    93,     0,     0,   529,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     170,   513,     0,     0,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   660,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      99,     0,     0,   170,     0,     0,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,     0,     0,     0,   704,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1028,     0,    14,    15,     0,
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
       0,    97,    98,    99,     0,     0,   170,     0,     0,     0,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,   744,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
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
       0,     0,     0,     0,    97,    98,    99,     0,     0,   170,
       0,     0,     0,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    -969,  -969,  -969,  -969,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,     0,
       0,   746,     0,     0,     0,     0,     0,     0,     0,     0,
     445,     0,     0,    14,    15,     0,     0,     0,     0,    16,
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
       0,     0,   170,     0,     0,     0,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
       0,     0,     0,     0,  1130,     0,     0,     0,     0,     0,
       0,     0,     0,  1028,     0,     0,    14,    15,     0,     0,
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
      97,    98,    99,     0,     0,   170,     0,   418,   419,   420,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   421,     0,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   445,     0,    16,     0,    17,    18,    19,
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
       0,     0,    93,     0,     0,   531,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   170,     0,
     418,   419,   420,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   421,
     901,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   445,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,   594,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   164,   165,   166,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   167,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   168,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   169,    90,     0,
     251,   252,    92,   253,   254,    93,     0,   255,   256,   257,
     258,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   170,     0,     0,   259,     0,   103,   104,     0,   105,
     106,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   261,   444,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   445,   263,   264,   265,   266,   267,
     268,   269,     0,     0,     0,    36,     0,   198,     0,     0,
       0,     0,     0,     0,     0,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,    48,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
       0,     0,     0,   692,   305,   306,   307,     0,     0,     0,
     308,   542,   543,   251,   252,     0,   253,   254,     0,     0,
     255,   256,   257,   258,     0,     0,     0,     0,     0,   544,
       0,     0,     0,     0,     0,    86,    87,   259,    88,   169,
      90,   313,     0,   314,     0,     0,   315,  -969,  -969,  -969,
    -969,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,   261,     0,   693,     0,   103,     0,
       0,     0,     0,     0,     0,     0,     0,  1028,   263,   264,
     265,   266,   267,   268,   269,     0,     0,     0,    36,     0,
     198,     0,     0,     0,     0,     0,     0,     0,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,    48,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,     0,     0,     0,   304,   305,   306,   307,
       0,     0,     0,   308,   542,   543,     0,     0,     0,     0,
       0,   251,   252,     0,   253,   254,     0,     0,   255,   256,
     257,   258,   544,     0,     0,     0,     0,     0,    86,    87,
       0,    88,   169,    90,   313,   259,   314,   260,     0,   315,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   693,
       0,   103,   261,     0,   262,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,   317,     0,     0,     0,  1596,     0,
     261,     0,   262,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   263,   264,   265,   266,   267,   268,
     269,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,    48,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,     0,
       0,     0,     0,   305,   306,   307,     0,     0,     0,   308,
     309,   310,     0,     0,     0,     0,     0,   251,   252,     0,
     253,   254,     0,     0,   255,   256,   257,   258,   311,     0,
       0,    84,   312,     0,    86,    87,     0,    88,   169,    90,
     313,   259,   314,   260,     0,   315,     0,     0,     0,     0,
       0,     0,   316,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   317,     0,     0,     0,  1664,     0,   261,     0,
     262,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   263,   264,   265,   266,   267,   268,   269,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,    48,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,     0,     0,     0,
     304,   305,   306,   307,     0,     0,     0,   308,   309,   310,
       0,     0,     0,     0,     0,   251,   252,     0,   253,   254,
       0,     0,   255,   256,   257,   258,   311,     0,     0,    84,
     312,     0,    86,    87,     0,    88,   169,    90,   313,   259,
     314,   260,     0,   315,     0,     0,     0,     0,     0,     0,
     316,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     317,     0,     0,     0,     0,     0,   261,     0,   262,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     263,   264,   265,   266,   267,   268,   269,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,    48,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,     0,     0,     0,     0,   305,
     306,   307,     0,     0,     0,   308,   309,   310,     0,     0,
       0,     0,     0,   251,   252,     0,   253,   254,     0,     0,
     255,   256,   257,   258,   311,     0,     0,    84,   312,     0,
      86,    87,     0,    88,   169,    90,   313,   259,   314,   260,
       0,   315,     0,     0,     0,     0,     0,     0,   316,  1405,
       0,     0,     0,     0,     0,     0,     0,     0,   317,     0,
       0,     0,     0,     0,   261,     0,   262,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   263,   264,
     265,   266,   267,   268,   269,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,    48,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,     0,     0,     0,     0,   305,   306,   307,
       0,     0,     0,   308,   309,   310,    33,    34,    35,    36,
       0,   198,     0,     0,     0,     0,     0,     0,   199,     0,
       0,     0,   311,     0,     0,    84,   312,     0,    86,    87,
      48,    88,   169,    90,   313,     0,   314,     0,     0,   315,
    1497,  1498,  1499,  1500,  1501,     0,   316,  1502,  1503,  1504,
    1505,   216,     0,     0,     0,     0,   317,   217,     0,     0,
       0,     0,     0,     0,  1506,  1507,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   201,     0,
       0,     0,     0,   168,    82,    83,    84,    85,     0,    86,
      87,  1508,    88,   169,    90,     0,     0,     0,    92,     0,
       0,     0,     0,     0,     0,  1509,  1510,  1511,  1512,  1513,
    1514,  1515,    97,     0,     0,    36,     0,   218,     0,     0,
       0,     0,   103,     0,     0,  1516,  1517,  1518,  1519,  1520,
    1521,  1522,  1523,  1524,  1525,  1526,    48,  1527,  1528,  1529,
    1530,  1531,  1532,  1533,  1534,  1535,  1536,  1537,  1538,  1539,
    1540,  1541,  1542,  1543,  1544,  1545,  1546,  1547,  1548,  1549,
    1550,  1551,  1552,  1553,  1554,  1555,  1556,     0,     0,     0,
    1557,  1558,     0,  1559,  1560,  1561,  1562,  1563,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1564,
    1565,  1566,     0,     0,     0,    86,    87,     0,    88,   169,
      90,  1567,     0,  1568,  1569,     0,  1570,   418,   419,   420,
       0,     0,     0,  1571,  1572,     0,  1573,     0,  1574,  1575,
       0,     0,     0,     0,     0,     0,   421,     0,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,   418,   419,   420,     0,     0,     0,     0,     0,
       0,     0,     0,   445,     0,     0,     0,     0,     0,     0,
       0,   421,     0,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,   418,   419,   420,
       0,     0,     0,     0,     0,     0,     0,     0,   445,     0,
       0,     0,     0,     0,     0,     0,   421,     0,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
       0,   444,   418,   419,   420,     0,     0,     0,     0,     0,
       0,     0,     0,   445,     0,     0,     0,     0,     0,     0,
       0,   421,     0,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   548,   444,  1002,  1003,  1004,
       0,     0,     0,     0,     0,     0,     0,     0,   445,     0,
       0,     0,     0,     0,     0,     0,  1005,     0,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
     552,     0,     0,     0,     0,     0,     0,   251,   252,     0,
     253,   254,     0,  1028,   255,   256,   257,   258,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   259,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   736,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   261,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   263,   264,   265,   266,   267,   268,   269,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,   761,
       0,     0,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,    48,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,     0,     0,     0,
     304,   305,   306,   307,  1173,     0,     0,   308,   542,   543,
     251,   252,     0,   253,   254,     0,     0,   255,   256,   257,
     258,     0,     0,     0,     0,     0,   544,     0,     0,     0,
       0,     0,    86,    87,   259,    88,   169,    90,   313,     0,
     314,     0,     0,   315,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   261,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   263,   264,   265,   266,   267,
     268,   269,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,    48,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
       0,     0,     0,  1174,   305,   306,   307,     0,     0,     0,
     308,   542,   543,   251,   252,     0,   253,   254,     0,     0,
     255,   256,   257,   258,     0,     0,     0,     0,     0,   544,
       0,     0,     0,     0,     0,    86,    87,   259,    88,   169,
      90,   313,     0,   314,     0,     0,   315,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   261,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   263,   264,
     265,   266,   267,   268,   269,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,    48,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,     0,     0,     0,     0,   305,   306,   307,
    1182,     0,     0,   308,   542,   543,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   790,   791,     0,
       0,     0,   544,   792,     0,   793,     0,     0,    86,    87,
       0,    88,   169,    90,   313,     0,   314,   794,     0,   315,
       0,     0,     0,     0,     0,    33,    34,    35,    36,   418,
     419,   420,     0,     0,     0,     0,     0,   199,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   421,    48,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,     0,     0,     0,     0,   963,     0,
       0,     0,     0,     0,   795,   445,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   201,     0,     0,
       0,     0,   168,    82,    83,    84,   796,     0,    86,    87,
      28,    88,   169,    90,     0,     0,     0,    92,    33,    34,
      35,    36,     0,   198,     0,     0,   797,     0,     0,     0,
     199,    97,     0,     0,     0,     0,   798,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,   490,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   964,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     201,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   169,    90,   790,   791,     0,
      92,     0,     0,   792,     0,   793,     0,     0,     0,     0,
       0,     0,     0,     0,    97,     0,     0,   794,     0,   202,
       0,     0,     0,     0,   103,    33,    34,    35,    36,   418,
     419,   420,     0,     0,     0,     0,     0,   199,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   421,    48,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   795,   445,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   201,     0,     0,
       0,     0,   168,    82,    83,    84,   796,     0,    86,    87,
      28,    88,   169,    90,     0,     0,     0,    92,    33,    34,
      35,    36,     0,   198,     0,     0,   797,     0,     0,     0,
     199,    97,     0,     0,     0,     0,   798,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,   499,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     201,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,    28,    88,   169,    90,     0,     0,     0,
      92,    33,    34,    35,    36,     0,   198,     0,     0,     0,
       0,     0,     0,   199,    97,     0,     0,     0,     0,   202,
       0,     0,   565,     0,   103,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   585,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   201,     0,     0,     0,     0,   168,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   169,    90,
      28,     0,   918,    92,     0,     0,     0,     0,    33,    34,
      35,    36,     0,   198,     0,     0,     0,    97,     0,     0,
     199,     0,   202,     0,     0,     0,     0,   103,     0,     0,
       0,     0,    48,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,   200,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1028,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     201,     0,     0,     0,     0,   168,    82,    83,    84,    85,
       0,    86,    87,    28,    88,   169,    90,     0,     0,     0,
      92,    33,    34,    35,    36,     0,   198,     0,     0,     0,
       0,     0,     0,   199,    97,     0,     0,     0,     0,   202,
       0,     0,     0,     0,   103,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1053,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   201,     0,     0,     0,     0,   168,    82,
      83,    84,    85,     0,    86,    87,    28,    88,   169,    90,
       0,     0,     0,    92,    33,    34,    35,    36,     0,   198,
       0,     0,     0,     0,     0,     0,   199,    97,     0,     0,
       0,     0,   202,     0,     0,     0,     0,   103,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   201,     0,     0,     0,
       0,   168,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   169,    90,     0,     0,     0,    92,     0,     0,     0,
     418,   419,   420,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   202,     0,     0,     0,   421,
     103,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,     0,   444,   418,   419,   420,     0,     0,
       0,     0,     0,     0,     0,     0,   445,     0,     0,     0,
       0,     0,     0,     0,   421,     0,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   445,   418,   419,   420,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   421,   904,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,     0,   444,  1002,  1003,  1004,
       0,     0,     0,     0,     0,     0,     0,     0,   445,     0,
       0,     0,     0,     0,     0,     0,  1005,   948,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1028,  1002,  1003,  1004,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1005,  1254,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,     0,     0,  1002,
    1003,  1004,     0,     0,     0,     0,     0,     0,     0,     0,
    1028,     0,     0,     0,     0,     0,     0,     0,  1005,  1164,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1028,  1002,  1003,  1004,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1005,  1312,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,     0,
       0,   418,   419,   420,     0,     0,     0,     0,     0,     0,
       0,     0,  1028,     0,     0,     0,     0,     0,     0,   773,
     421,  1394,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,     0,   444,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   418,
     419,   420,     0,     0,     0,     0,     0,     0,  1480,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   421,   774,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   443,     0,   444,  1002,  1003,  1004,     0,     0,     0,
       0,     0,     0,     0,     0,   445,     0,     0,     0,     0,
       0,     0,     0,  1005,     0,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,  1026,  1027,  1003,  1004,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1028,     0,     0,     0,     0,  1005,     0,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,   420,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1028,     0,     0,     0,   421,     0,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
    1004,   444,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   445,     0,     0,     0,  1005,     0,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1028,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,   443,     0,   444,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     445,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1028,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,     0,   444,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   445,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1028
};

static const yytype_int16 yycheck[] =
{
       5,     6,   147,     8,     9,    10,    11,    12,    54,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   122,     4,    28,    29,   172,   650,     4,     4,    91,
       4,   100,   373,    95,    96,    30,   373,    42,  1075,   503,
     484,    32,   152,   877,   620,    50,   373,    52,   701,    55,
      55,   738,    57,    44,   214,    58,    54,    30,    49,   121,
     219,   147,   100,   866,   444,     4,   601,   480,   481,    30,
      30,    42,   173,  1062,    79,   621,   772,   897,    81,     4,
     476,    84,   476,   962,   100,   100,  1071,   779,     9,     9,
     748,     9,     9,   913,     9,   100,   509,     9,     9,     9,
      30,     9,    14,    14,   230,     9,     9,    51,    79,     9,
      14,     9,     9,     9,     9,   511,     9,   511,    67,     9,
       9,   231,     9,     9,     9,     9,     9,    67,     9,   949,
       4,     9,   170,    80,    67,    33,     9,    46,     9,     9,
       9,     9,    30,     9,     9,   122,   102,   103,    98,    80,
     151,   102,   103,    35,   170,   170,    80,    80,   111,  1610,
      67,    80,   130,   131,   202,   170,   151,   508,   151,    46,
     448,   172,   177,    86,     0,    67,   186,   157,   172,   151,
     218,    35,    46,    46,    46,   186,   202,   202,   189,   172,
     130,   131,   186,   130,   131,   998,   173,   202,    80,   477,
     172,   186,   218,     8,   482,   155,    35,    35,   147,   189,
      35,   186,   156,   218,  1665,   172,   169,   189,   165,    67,
     151,    67,   190,   130,   131,    67,    80,   232,    67,   186,
     235,    67,   358,    67,   190,   148,    67,   242,   243,   190,
     189,   165,   165,    80,    67,    67,    67,    67,    67,   189,
      67,    80,    80,   186,   187,    80,   173,    67,   916,   189,
     103,   172,  1251,   410,  1249,   173,   187,  1146,   188,  1258,
     188,  1260,   191,   188,   189,   184,   188,   188,   188,   325,
     972,    51,   974,   165,   188,   188,  1106,    67,   188,   187,
    1279,   188,   188,   188,   186,   188,    26,    27,   188,   188,
     188,   188,   188,   188,   188,   188,   187,   173,   173,   187,
     187,   236,   350,   189,   187,   240,   187,   187,   187,   187,
     855,   184,  1125,   187,   187,   187,    80,   325,   151,   332,
     333,   334,   151,   493,   350,   350,   165,   165,   186,   486,
     165,   189,   404,   189,   349,   350,    67,   186,   894,    54,
     189,   356,   186,   189,   191,   401,   361,   458,   189,    35,
     156,    66,   365,   186,   186,   186,   189,   189,   189,   189,
     189,   151,   189,    67,  1359,   380,  1361,   186,   349,   189,
     119,  1370,   172,   388,    29,    86,   156,   126,    98,   118,
     459,   453,   454,   455,   456,   400,   186,   126,   842,   186,
      80,   171,    47,   401,    80,    50,    86,  1060,    80,   189,
     387,   165,   186,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,  1126,
     445,     4,   447,   448,   449,   155,  1142,   148,   387,   444,
      98,   611,   450,   459,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,  1325,   189,   149,
     150,   444,   477,   478,  1459,   480,   481,   482,   483,    67,
     457,   211,   487,   444,   444,   490,    80,   883,    51,   883,
     186,    54,    98,   638,   499,   189,   501,   910,  1074,   497,
      80,   627,    29,   629,   509,   379,  1309,   155,    71,   130,
     131,   517,   517,   508,   519,   195,    80,   189,   186,   191,
      47,    98,    86,    50,    98,   866,    89,   186,    91,   866,
      14,   691,    95,    96,   172,   186,   695,   172,   918,   866,
     155,   188,   130,   131,  1090,   186,    30,  1093,   186,   155,
     553,   186,   638,  1411,   557,   186,   703,   186,   121,   562,
     565,    47,    48,   151,    48,   186,    78,   726,     4,   149,
     150,   609,   186,   188,   189,  1378,    30,  1435,   155,  1437,
    1267,   155,  1269,    72,    73,   149,   150,    99,    29,   183,
      26,    27,    72,    73,   656,   189,   155,   522,   186,   101,
     102,   103,    98,    99,   609,   186,  1259,   450,    35,   339,
     128,   129,   124,    80,    67,    56,   742,   743,   348,    86,
     350,    80,   185,   749,   750,   355,   786,    86,  1613,    67,
     194,   188,   362,   476,    80,   188,   189,    78,   188,   749,
     799,    26,    27,   648,  1108,   188,   158,   159,   807,   161,
     162,   163,   188,   189,   497,   660,   188,   998,    99,    78,
     188,   998,  1238,   157,   100,   508,   107,   189,   511,  1639,
    1640,   998,   151,   236,   186,   186,   170,   240,   186,    29,
      99,   244,   149,   150,  1635,  1636,  1099,  1340,   693,  1376,
     149,   150,    67,   134,   135,   189,   151,  1110,   186,   638,
      80,  1145,   148,   149,   150,   155,    86,  1253,    50,    51,
      52,   152,  1749,   188,   155,   156,   721,   158,   159,    45,
     161,   162,   163,    80,    66,    66,    80,  1764,    78,    86,
     788,   789,    86,   152,   170,   176,   155,    87,   172,   158,
     159,   151,   161,   162,   163,   186,   186,   621,   193,    99,
       9,   756,     4,   186,   484,   101,   102,   103,  1743,  1412,
     151,   151,   325,   115,   116,   117,   202,   772,   148,   149,
     150,   190,   186,  1758,     8,   211,  1220,   768,   151,  1466,
     188,   186,   218,   943,  1125,   762,    14,   151,  1125,   188,
     126,   148,   149,   150,    46,   149,   150,   126,  1125,    14,
     236,   187,   152,   172,   240,   155,    14,    98,   158,   159,
     813,   161,   162,   163,   817,   187,   187,   115,   116,   117,
     118,   119,   120,   762,   387,  1371,   211,   187,   988,    50,
      51,    52,   809,    54,   397,   995,   187,   192,   401,   186,
     107,   404,  1255,   186,     9,    66,   851,   104,   105,   106,
     187,   186,   777,   148,   187,   917,   108,   187,    90,   187,
     865,   113,     9,   115,   116,   117,   118,   119,   120,   121,
     809,   869,   854,   188,    14,   172,   186,   854,   854,     9,
     854,   186,   515,  1717,   182,    80,   891,   450,   451,   452,
     453,   454,   455,   456,   187,   187,   901,   187,   875,   904,
     188,   906,  1736,   339,   128,   910,   158,   159,   186,   161,
    1744,    78,   348,   476,   350,   854,   549,   647,   187,   355,
      30,    67,  1081,   918,   129,    78,   362,   171,  1615,   854,
     182,   151,    99,   132,   497,     9,   875,   187,   190,   151,
       9,    14,   184,   948,  1357,   918,    99,     9,   511,   187,
     173,   387,     9,    14,   339,   128,   193,   918,   918,   522,
    1120,   190,   193,   348,   955,     9,    14,   193,  1309,   187,
     355,   187,  1309,   186,   704,   151,    78,   362,   541,   193,
     854,   958,  1309,   960,   187,    98,  1055,   188,   373,   622,
     188,   158,   159,    87,   161,   162,   163,    99,   132,   997,
     151,   564,  1000,   877,     9,   158,   159,   187,   161,   162,
     163,  1171,   186,   151,   744,   186,   746,   189,  1178,   958,
     894,   960,   151,   189,    14,   188,   869,   590,   591,   189,
      14,   956,   193,   186,   189,  1659,    14,  1378,   881,   187,
     883,  1378,   184,   188,   774,    30,    14,   186,   484,  1055,
    1055,  1378,    14,   155,   186,    30,   158,   159,   186,   161,
     162,   163,   186,    49,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,  1059,   186,   186,
       9,  1086,  1059,  1059,   187,  1059,   522,   189,   188,   188,
     186,   132,    14,   656,  1099,   132,     9,   827,   187,   484,
      66,   193,     9,    80,     9,  1110,  1111,   950,   186,    78,
    1734,    80,   842,   843,    64,    65,  1276,    78,   132,    80,
    1059,   115,   116,   117,   118,   119,   120,    10,    11,    12,
      99,   764,   126,   127,  1059,   186,   769,  1142,    99,   188,
      14,    80,   187,  1134,   186,   188,    29,  1152,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
     164,    54,  1149,   189,   186,   738,   187,   740,    26,    27,
     130,   131,   189,    66,   132,  1059,    78,   156,   182,   158,
     159,   189,   161,   162,   163,   156,  1341,   158,   159,   762,
     161,   162,   163,     9,   193,    87,   148,    99,   189,    30,
    1149,   647,  1137,   776,   777,    78,  1090,    74,   188,  1093,
     189,   188,   191,   187,   173,  1230,   132,    30,   189,  1234,
     191,  1236,   962,   963,   187,  1336,    99,   187,   132,  1244,
     187,    75,    76,    77,    78,     9,   809,   187,     9,  1254,
    1255,   187,   815,   190,    14,   190,   819,   820,   189,   186,
      80,   187,   647,   187,   187,    99,   158,   159,   704,   161,
     162,   163,   107,   188,   186,   189,   839,   187,   187,  1218,
     115,   116,   117,   118,   119,   120,  1433,   132,  1227,   187,
       9,   854,    30,   156,   186,   158,   159,   160,   161,   162,
     163,    78,   188,   187,   189,   160,   869,   190,   744,   187,
     746,   188,   875,   188,   108,   188,   156,    14,   881,   704,
     883,    80,    99,   186,   158,   159,   762,   161,   162,   163,
     107,   108,   113,   187,   187,   132,   969,   189,   774,   132,
     187,   777,  1319,  1405,  1218,    14,   172,   182,   189,  1354,
     188,    80,  1357,  1227,   917,  1085,    14,    14,  1335,   744,
      80,   746,   187,   211,   186,   189,   929,   930,   931,   187,
     132,   188,   188,   809,    14,    14,   188,    14,   155,  1253,
    1319,   158,   159,     9,   161,   162,   163,   950,   189,   774,
     190,   827,    56,   956,    80,   958,   172,   960,   186,    78,
    1130,     9,  1341,    80,   188,  1382,   842,   843,   111,    98,
    1408,   151,   163,    98,  1391,  1145,  1146,   980,   854,    33,
      99,  1360,    14,   186,   169,   173,    80,  1366,   186,  1368,
     187,   166,   188,   187,   997,     9,    80,  1000,   188,   875,
     187,  1380,   827,  1382,   187,  1450,    14,   189,  1595,    80,
      14,    80,  1391,    14,    14,    80,    80,   842,   843,   817,
     813,   562,  1387,  1725,  1441,   456,  1029,   911,    78,   451,
    1630,  1740,  1035,   152,   453,  1038,   155,   156,   857,   158,
     159,   866,   161,   162,   163,  1143,  1360,  1294,    78,    99,
    1220,   339,  1366,  1470,  1368,  1736,  1059,  1371,  1462,  1476,
     348,   567,  1472,  1495,  1481,  1579,  1380,   355,  1413,    99,
    1768,  1756,    29,    78,   362,    80,    81,  1331,  1591,  1327,
     956,  1063,   958,  1458,   960,  1464,   962,   963,   996,   930,
    1121,  1470,  1122,   945,    99,   401,   881,  1476,  1320,    26,
      27,    78,  1481,    30,  1590,   155,  1471,  1472,   158,   159,
    1693,   161,   162,   163,   356,   788,  1046,     4,   981,  1122,
    1029,    78,    99,  1126,    -1,   155,  1711,    54,   158,   159,
      -1,   161,   162,   163,  1137,    -1,  1209,   962,   963,    -1,
      -1,    -1,    99,    -1,    -1,    -1,  1149,    -1,    -1,    -1,
    1464,  1653,  1590,   158,   159,  1600,   161,   162,   163,    46,
      -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    -1,   998,    -1,    -1,    -1,   134,   135,   156,
      -1,   158,   159,  1059,   161,   162,   163,    -1,  1729,    99,
      -1,    -1,    -1,    -1,    -1,   152,   484,    -1,   155,   156,
      -1,   158,   159,  1620,   161,   162,   163,    -1,  1281,  1085,
    1213,    -1,   115,   116,   117,   118,   119,   120,    -1,    -1,
      -1,   108,    -1,    -1,  1297,    -1,   113,    -1,   115,   116,
     117,   118,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,  1620,    -1,  1660,  1661,   155,    -1,    -1,   158,   159,
    1667,   161,   162,   163,  1130,    -1,    -1,    -1,    -1,    -1,
    1085,  1137,    -1,   359,  1267,    -1,  1269,   363,    -1,  1145,
    1146,   158,   159,  1149,   161,    -1,    -1,    -1,    -1,   182,
      -1,  1660,  1661,    -1,   211,   188,    -1,  1704,  1667,    -1,
      -1,    78,    -1,    -1,   390,   182,   392,   393,   394,   395,
    1125,    -1,    -1,   190,    -1,  1130,    -1,    -1,  1381,    -1,
      -1,    -1,    99,    -1,    -1,    -1,  1319,  1390,    -1,    -1,
    1145,  1146,  1325,    -1,    -1,  1704,    -1,  1400,  1331,  1774,
      -1,    -1,  1711,   260,   121,   262,    -1,  1782,     4,    -1,
      -1,    -1,    -1,  1788,  1220,    -1,  1791,   134,   135,    -1,
      -1,    -1,    -1,  1770,    -1,    -1,    -1,    -1,    -1,   647,
    1777,    -1,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,
      -1,   158,   159,  1376,   161,   162,   163,    -1,    -1,  1382,
      46,    -1,    -1,    -1,  1387,    -1,    -1,     4,  1391,    -1,
     317,  1770,    -1,    -1,    -1,  1220,    -1,    -1,  1777,    -1,
    1473,    -1,  1405,  1717,    -1,  1408,    -1,    -1,  1411,    -1,
      -1,    -1,   339,    -1,    -1,    -1,   704,    -1,  1421,    -1,
      -1,   348,  1736,    -1,    -1,  1428,    -1,    -1,   355,    46,
    1744,    -1,  1435,    -1,  1437,   362,    -1,    -1,    -1,    54,
    1443,    -1,   108,  1319,    -1,    -1,   373,   113,    -1,   115,
     116,   117,   118,   119,   120,   121,   744,    -1,   746,    -1,
      -1,    -1,    -1,  1466,    -1,     4,    -1,  1470,  1471,  1472,
      -1,   398,    -1,  1476,   401,    -1,    -1,    -1,  1481,    -1,
      -1,    -1,    -1,    -1,  1309,    -1,   774,    -1,    -1,    -1,
      -1,   108,   158,   159,    -1,   161,   113,    -1,   115,   116,
     117,   118,   119,   120,   121,    -1,  1382,    46,    -1,    -1,
      -1,  1387,    -1,    -1,    -1,  1391,   182,   444,    47,    48,
      49,    50,    51,    52,   190,    54,    47,    48,    49,    50,
      51,    52,  1605,    -1,  1607,    -1,    -1,    66,    -1,   827,
      -1,   158,   159,  1616,   161,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1378,   842,   843,     4,   484,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,   108,
      -1,    -1,    -1,   190,   113,    -1,   115,   116,   117,   118,
     119,   120,   121,    -1,    -1,    -1,    -1,  1590,    -1,  1662,
      -1,    -1,    -1,    -1,  1470,  1471,  1472,    -1,    46,    -1,
    1476,    -1,    -1,    -1,    -1,  1481,    -1,    -1,  1611,   536,
     537,    -1,  1615,   540,    -1,    -1,    -1,  1620,    -1,   158,
     159,    -1,   161,    -1,     4,  1628,    -1,    -1,    -1,    -1,
      -1,    -1,  1635,  1636,    -1,    -1,  1639,  1640,    -1,    -1,
      -1,    -1,    -1,   182,   571,   260,    -1,   262,    -1,    -1,
    1653,   190,    26,    27,    -1,    -1,    30,  1660,  1661,    -1,
     108,    -1,    -1,    -1,  1667,   113,    46,   115,   116,   117,
     118,   119,   120,   121,   962,   963,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,  1762,
     617,   618,    -1,    -1,    -1,    -1,  1769,    -1,    -1,   626,
      -1,  1704,   317,    -1,    -1,    -1,    -1,  1710,    -1,    -1,
     158,   159,    -1,   161,    -1,    -1,    -1,    -1,    -1,    -1,
     647,    -1,    -1,    -1,    -1,  1728,    64,    65,   108,    -1,
      -1,    -1,    -1,   113,   182,   115,   116,   117,   118,   119,
     120,   121,   190,    -1,  1620,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,  1770,    -1,    -1,
      -1,    -1,    -1,    -1,  1777,    -1,    -1,   704,   158,   159,
      -1,   161,    -1,   398,  1660,  1661,   401,    -1,    -1,    -1,
      -1,  1667,   130,   131,    -1,    -1,    -1,  1085,    -1,    -1,
      -1,    -1,   182,    64,    65,    -1,    -1,    -1,    -1,    -1,
     190,    -1,    -1,    -1,    -1,    -1,    -1,   744,    -1,   746,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1704,   115,
     116,   117,   118,   119,   120,    -1,    -1,   211,    -1,    -1,
     126,   127,  1130,    -1,    -1,    -1,    -1,   774,   775,   187,
      29,    -1,    -1,    -1,    -1,    -1,    -1,  1145,  1146,    -1,
      -1,   788,   789,   790,   791,   792,   793,   794,    -1,   130,
     131,   798,    -1,    -1,    -1,    -1,   162,    56,   164,    -1,
      -1,    -1,    -1,   810,    -1,    -1,    -1,    29,    -1,    -1,
      -1,   177,    -1,   179,  1770,    -1,   182,    -1,    -1,    78,
     827,  1777,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   840,    56,   842,   843,    -1,    -1,    -1,
      99,   536,    -1,    -1,    -1,    -1,   187,    -1,    -1,   856,
     857,    -1,  1220,    -1,    -1,    -1,    78,    -1,    -1,   866,
      -1,    -1,    -1,    -1,    -1,   872,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,    99,   885,    -1,
      -1,    -1,    -1,    -1,    -1,   339,   893,    -1,    -1,   896,
      -1,    -1,    -1,   152,   348,    -1,   155,   156,    -1,   158,
     159,   355,   161,   162,   163,    -1,   165,   914,   362,    -1,
      -1,   918,   134,   135,    -1,    -1,    -1,   176,    -1,   373,
      -1,    -1,   617,   618,    -1,    -1,    -1,   186,    -1,    -1,
     152,   626,    -1,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,   165,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   176,   962,   963,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   976,
      -1,    -1,   979,    -1,   981,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   996,
     444,   998,    -1,    -1,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
    1027,  1028,    -1,    10,    11,    12,    -1,    75,    76,    77,
     484,    -1,    -1,    -1,    -1,    -1,  1043,    -1,    -1,    87,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,  1073,    54,  1075,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1085,    66,
      -1,    -1,    -1,    -1,    -1,    -1,   540,    -1,   136,   137,
     138,   139,   140,   788,   789,  1102,    -1,    -1,  1105,   147,
      -1,    -1,    -1,    -1,    -1,   153,   154,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   571,  1125,   167,
      -1,    -1,    -1,  1130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,  1145,  1146,
      -1,  1148,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1157,    -1,    -1,    -1,  1161,    -1,    -1,  1164,    -1,  1166,
      -1,   856,    -1,    -1,    -1,    -1,    -1,    -1,    54,    56,
      -1,    -1,    -1,    -1,    -1,  1182,    -1,   872,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     885,    78,    -1,   647,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,   193,  1214,  1215,    -1,
    1217,    -1,    99,  1220,    -1,    -1,    -1,    -1,    29,   914,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,   134,   135,    -1,
     704,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
      -1,   976,    -1,    -1,   979,    -1,    -1,    -1,    -1,   176,
     744,    -1,   746,    -1,  1301,    -1,  1303,    -1,    -1,   186,
      -1,  1308,  1309,    -1,    -1,  1312,    -1,  1314,    -1,    -1,
    1317,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1325,  1326,
     774,   775,  1329,    -1,    -1,    -1,    -1,    -1,    -1,  1336,
      -1,    -1,    47,    48,    -1,    -1,   790,   791,   792,   793,
     794,    -1,    -1,    -1,   798,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,   810,    -1,    -1,    -1,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1378,    87,   827,   260,    -1,   262,    -1,  1073,   190,
    1075,    -1,    -1,    -1,    99,    -1,   840,  1394,   842,   843,
      -1,    -1,    -1,    -1,    -1,    -1,  1403,  1404,    -1,    -1,
      -1,    -1,    -1,   857,  1411,    -1,  1413,  1102,    -1,    -1,
    1105,    -1,   866,    -1,    -1,    -1,    -1,    -1,    -1,   134,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1435,    -1,
    1437,   317,   147,    -1,    -1,    -1,  1443,    -1,    -1,   893,
      -1,    -1,   896,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   176,  1157,    -1,   918,    -1,  1161,    -1,    -1,    -1,
      -1,    -1,  1479,  1480,    -1,    -1,    -1,    -1,  1485,    -1,
    1487,    -1,    -1,    -1,    -1,    -1,  1493,    -1,  1495,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   962,   963,
      -1,    -1,   398,    -1,    -1,   401,    -1,    -1,    -1,  1214,
    1215,    -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,   996,    -1,   998,    64,    65,  1001,  1002,  1003,
    1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,    -1,    64,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1594,    -1,  1043,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,  1611,    54,  1301,    -1,  1303,    -1,
      -1,   130,   131,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,  1628,    -1,    -1,    -1,    -1,    -1,  1634,    -1,    -1,
      -1,  1085,    -1,    -1,    -1,    -1,    -1,    -1,  1645,    -1,
      -1,  1336,   130,   131,  1651,    -1,    -1,    -1,  1655,    -1,
     536,   537,    -1,    -1,   540,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,  1676,
      -1,  1125,    -1,    -1,    -1,    -1,  1130,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,   571,    -1,    -1,    -1,    -1,
      -1,  1145,  1146,    -1,  1148,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1715,    -1,
    1164,    -1,  1166,    -1,    -1,    -1,    -1,  1724,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1182,    -1,
      -1,   617,   618,  1740,    -1,    -1,    -1,    -1,    -1,    -1,
     626,    -1,  1749,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1764,    -1,    -1,
      -1,    -1,    -1,  1217,    -1,    29,  1220,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    10,    11,    12,    -1,    -1,    -1,    -1,  1493,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,  1308,  1309,    -1,    -1,  1312,    -1,
    1314,    -1,    -1,  1317,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1326,    -1,    -1,  1329,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   775,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1594,
      -1,    -1,   788,   789,   790,   791,   792,   793,   794,    -1,
      -1,    -1,   798,    26,    27,    -1,   540,    30,    -1,    -1,
      -1,    -1,    -1,    -1,  1378,    -1,   190,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1394,    -1,    -1,    -1,    -1,    -1,    -1,   571,    -1,  1403,
    1404,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,  1413,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     856,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      -1,  1676,    -1,    -1,    -1,    -1,   872,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   885,
      99,    -1,    -1,    -1,    -1,    -1,    -1,   893,   107,    -1,
      -1,    -1,    -1,    -1,    -1,   540,   115,   116,   117,   118,
     119,   120,    -1,    -1,    -1,  1479,  1480,    -1,   914,    -1,
      -1,  1485,    -1,  1487,    -1,   134,   135,    -1,    -1,    -1,
      -1,  1495,    -1,    -1,    -1,    -1,   571,    -1,    -1,    -1,
      -1,    -1,    -1,   152,  1749,    -1,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,    -1,  1764,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,   182,    -1,    -1,    -1,   186,    -1,    -1,
     976,    -1,    -1,   979,    -1,   981,    -1,    -1,   211,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     996,    -1,    -1,    -1,    -1,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
    1026,  1027,  1028,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   775,    -1,    -1,    -1,    -1,    -1,  1043,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   790,   791,   792,   793,
     794,    -1,    -1,    -1,   798,    -1,    -1,    -1,    -1,    -1,
    1634,    -1,    -1,    -1,    -1,    -1,    -1,  1073,    -1,  1075,
      -1,  1645,    -1,    -1,    -1,    -1,    -1,  1651,    -1,    -1,
      -1,  1655,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1102,    -1,    -1,  1105,
      -1,    -1,    -1,    -1,    -1,    -1,   339,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   348,    -1,    -1,    -1,    -1,
      -1,    -1,   355,    -1,    -1,    -1,    -1,    -1,    -1,   362,
     775,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     373,  1715,  1148,    -1,    -1,   790,   791,   792,   793,   893,
    1724,  1157,    -1,   798,    -1,  1161,    -1,    -1,  1164,    -1,
    1166,    10,    11,    12,    -1,    -1,  1740,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1182,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,  1214,  1215,
      -1,   444,    -1,    -1,    -1,    -1,    29,    66,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,   893,    -1,
      -1,   484,   996,    66,    -1,    -1,    -1,  1001,  1002,  1003,
    1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1026,  1027,  1028,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1301,    -1,  1303,    -1,  1043,
      -1,    -1,  1308,    -1,    -1,    -1,  1312,   540,  1314,    -1,
      -1,  1317,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1325,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1336,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   571,    -1,
      -1,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1001,  1002,  1003,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,  1026,  1027,  1028,    -1,    -1,    -1,    -1,  1394,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1043,    -1,
      -1,    -1,    -1,    -1,  1148,  1411,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   647,    -1,    -1,    -1,    -1,    -1,
    1164,    -1,  1166,    -1,    10,    11,    12,    -1,    -1,  1435,
      -1,  1437,    -1,    -1,    -1,    -1,    -1,  1443,  1182,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,   704,    -1,  1479,  1480,    -1,    -1,    -1,    -1,  1485,
      66,    -1,    -1,    -1,    -1,    -1,    -1,  1493,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1148,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   744,    -1,   746,    -1,    -1,    -1,    -1,    -1,  1164,
      -1,  1166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1182,    -1,    -1,
      -1,   774,   775,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   790,   791,   792,
     793,   794,    -1,    -1,  1308,   798,    -1,    -1,  1312,    -1,
    1314,    -1,    -1,  1317,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1594,    -1,
      -1,    -1,    -1,    -1,   827,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   190,  1611,    -1,    -1,    -1,   842,
     843,    -1,    75,    76,    77,    78,    -1,    80,    -1,    -1,
      -1,    -1,  1628,    -1,    87,    -1,    -1,    -1,  1634,    -1,
      -1,    -1,    -1,   866,    -1,    -1,    99,    -1,    -1,  1645,
      -1,    -1,    -1,    -1,    -1,  1651,    -1,    -1,    -1,  1655,
    1394,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,
     893,    -1,    -1,  1308,    -1,    -1,    -1,  1312,    -1,  1314,
    1676,    -1,  1317,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,   918,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,  1715,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   962,
     963,    -1,    -1,    -1,    -1,  1479,  1480,    -1,    -1,    -1,
      -1,  1485,    -1,  1749,    -1,    -1,    -1,    -1,    -1,  1394,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1764,    -1,
      -1,    -1,    -1,   996,    -1,   998,    -1,    -1,  1001,  1002,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1027,  1028,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
    1043,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,  1479,  1480,    -1,    10,    11,    12,
    1485,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,  1085,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    75,    76,    77,    78,    -1,    80,    -1,    -1,
    1634,    -1,  1125,    66,    87,    -1,    -1,  1130,    -1,    -1,
      -1,  1645,    -1,    -1,    -1,    -1,    99,  1651,    -1,    -1,
      -1,  1655,  1145,  1146,    -1,  1148,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,
      -1,  1164,    -1,  1166,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   136,   137,   138,   139,   140,    -1,  1182,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,  1715,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1220,   181,  1634,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,    -1,
    1645,    -1,    10,    11,    12,    -1,  1651,    -1,    -1,    -1,
    1655,    -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    66,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
    1715,    -1,    -1,    -1,    -1,  1308,  1309,    27,    28,  1312,
      -1,  1314,    -1,    -1,  1317,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,
      -1,    -1,    -1,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,
      90,    -1,    92,    -1,    94,  1378,    96,    -1,    -1,    99,
     100,    -1,    -1,    -1,   104,   105,   106,   107,   108,   109,
     110,  1394,   112,   113,   114,   115,   116,   117,   118,   119,
     120,    -1,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,    -1,   190,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,
      -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,   189,
     190,   191,   192,    -1,   194,   195,  1479,  1480,    10,    11,
      12,    -1,  1485,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1494,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,  1634,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,  1645,    -1,    -1,    -1,    -1,    -1,  1651,    -1,
      -1,    -1,  1655,    -1,    -1,    46,    47,    48,   190,    -1,
      -1,    -1,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,  1678,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
      -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,    90,
      -1,    92,    -1,    94,   190,    96,    -1,    -1,    99,   100,
      -1,    -1,  1715,   104,   105,   106,   107,   108,   109,   110,
      -1,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,   122,   123,   124,   125,   126,   127,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,   170,
      -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,    -1,
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
     108,   109,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    -1,   122,   123,   124,   125,   126,   127,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,   164,    -1,    -1,   167,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,   177,
      -1,   179,    -1,   181,   182,   183,    -1,    -1,   186,    -1,
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
      90,    -1,    92,    -1,    94,    -1,    96,    97,    -1,    99,
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
      -1,    92,    -1,    94,    95,    96,    -1,    -1,    99,   100,
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
      92,    93,    94,    -1,    96,    -1,    -1,    99,   100,    -1,
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
     181,   182,   183,    -1,    -1,   186,    -1,   188,   189,    -1,
     191,   192,    -1,   194,   195,     3,     4,     5,     6,     7,
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
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
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
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     186,    -1,   188,    -1,    -1,   191,   192,    -1,   194,   195,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     183,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
      -1,   194,   195,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,   181,   182,   183,    -1,    -1,   186,    -1,    -1,   189,
      -1,   191,   192,    -1,   194,   195,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    35,    -1,
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
      -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,   192,    -1,   194,   195,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   186,    -1,    10,    11,    12,   191,   192,    -1,
     194,   195,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,    -1,
      66,    -1,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,   104,    -1,    -1,   107,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   115,   116,   117,   118,   119,   120,
      -1,    -1,   123,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,   170,
      -1,    -1,   188,    -1,    -1,   176,    -1,    -1,    -1,    -1,
     181,   182,   183,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,    -1,   194,   195,     3,     4,     5,     6,     7,
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
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,
      10,    11,    12,   191,   192,    -1,   194,   195,     3,     4,
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
      -1,    -1,   167,    -1,    -1,   170,    -1,    -1,   188,    -1,
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,   186,    -1,   188,    11,    12,   191,   192,    -1,   194,
     195,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,   181,
     182,   183,    -1,    -1,   186,    -1,   188,    -1,    -1,   191,
     192,    -1,   194,   195,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
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
      -1,   167,    -1,    -1,   170,    -1,    -1,   188,    -1,    -1,
     176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,
     186,   187,    -1,    -1,    -1,   191,   192,    -1,   194,   195,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
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
     183,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
      -1,   194,   195,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,   181,   182,   183,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,    -1,   194,   195,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
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
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
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
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,    -1,
     194,   195,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
      -1,    -1,   170,    -1,    -1,   188,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,
      10,    11,    12,   191,   192,    -1,   194,   195,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
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
       3,     4,   167,     6,     7,   170,    -1,    10,    11,    12,
      13,   176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,   186,    -1,    -1,    27,    -1,   191,   192,    -1,   194,
     195,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    54,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    68,    69,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,   126,   127,   128,   129,    -1,    -1,    -1,
     133,   134,   135,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   152,
      -1,    -1,    -1,    -1,    -1,   158,   159,    27,   161,   162,
     163,   164,    -1,   166,    -1,    -1,   169,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    54,    -1,   189,    -1,   191,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    68,    69,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,   126,   127,   128,   129,
      -1,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,   152,    -1,    -1,    -1,    -1,    -1,   158,   159,
      -1,   161,   162,   163,   164,    27,   166,    29,    -1,   169,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   189,
      -1,   191,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,
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
     126,   127,   128,   129,    -1,    -1,    -1,   133,   134,   135,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,   152,    -1,    -1,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   164,    27,
     166,    29,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,    -1,    54,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    69,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,   127,
     128,   129,    -1,    -1,    -1,   133,   134,   135,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,   152,    -1,    -1,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,   164,    27,   166,    29,
      -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,   176,   177,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,    -1,    54,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,    -1,   127,   128,   129,
      -1,    -1,    -1,   133,   134,   135,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,   152,    -1,    -1,   155,   156,    -1,   158,   159,
      99,   161,   162,   163,   164,    -1,   166,    -1,    -1,   169,
       3,     4,     5,     6,     7,    -1,   176,    10,    11,    12,
      13,   120,    -1,    -1,    -1,    -1,   186,   126,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    54,   161,   162,   163,    -1,    -1,    -1,   167,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,
      73,    74,   181,    -1,    -1,    78,    -1,   186,    -1,    -1,
      -1,    -1,   191,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
     133,   134,    -1,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,
     153,   154,    -1,    -1,    -1,   158,   159,    -1,   161,   162,
     163,   164,    -1,   166,   167,    -1,   169,    10,    11,    12,
      -1,    -1,    -1,   176,   177,    -1,   179,    -1,   181,   182,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,   188,    54,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
     188,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,    66,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    69,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
     126,   127,   128,   129,   187,    -1,    -1,   133,   134,   135,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,
      -1,    -1,   158,   159,    27,   161,   162,   163,   164,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,   126,   127,   128,   129,    -1,    -1,    -1,
     133,   134,   135,     3,     4,    -1,     6,     7,    -1,    -1,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   152,
      -1,    -1,    -1,    -1,    -1,   158,   159,    27,   161,   162,
     163,   164,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,    -1,   127,   128,   129,
      30,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,
      -1,    -1,   152,    53,    -1,    55,    -1,    -1,   158,   159,
      -1,   161,   162,   163,   164,    -1,   166,    67,    -1,   169,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    99,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,   134,    66,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      67,   161,   162,   163,    -1,    -1,    -1,   167,    75,    76,
      77,    78,    -1,    80,    -1,    -1,   176,    -1,    -1,    -1,
      87,   181,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    47,    48,    -1,
     167,    -1,    -1,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    67,    -1,   186,
      -1,    -1,    -1,    -1,   191,    75,    76,    77,    78,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    99,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,    66,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      67,   161,   162,   163,    -1,    -1,    -1,   167,    75,    76,
      77,    78,    -1,    80,    -1,    -1,   176,    -1,    -1,    -1,
      87,   181,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    67,   161,   162,   163,    -1,    -1,    -1,
     167,    75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    87,   181,    -1,    -1,    -1,    -1,   186,
      -1,    -1,   189,    -1,   191,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
      67,    -1,    69,   167,    -1,    -1,    -1,    -1,    75,    76,
      77,    78,    -1,    80,    -1,    -1,    -1,   181,    -1,    -1,
      87,    -1,   186,    -1,    -1,    -1,    -1,   191,    -1,    -1,
      -1,    -1,    99,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    67,   161,   162,   163,    -1,    -1,    -1,
     167,    75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    87,   181,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    67,   161,   162,   163,
      -1,    -1,    -1,   167,    75,    76,    77,    78,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    87,   181,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     181,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    29,
     191,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   132,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   132,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   132,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    -1,    10,
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
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      29,   132,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    98,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      12,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    29,    -1,    31,
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
     251,   257,   317,   318,   326,   330,   331,   332,   333,   334,
     335,   336,   337,   339,   342,   354,   355,   356,   358,   359,
     361,   371,   372,   373,   375,   380,   383,   402,   410,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     423,   437,   439,   441,   118,   119,   120,   133,   152,   162,
     186,   203,   236,   317,   336,   414,   336,   186,   336,   336,
     336,   104,   336,   336,   400,   401,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,    80,    87,
     120,   147,   186,   213,   355,   372,   375,   380,   414,   417,
     414,    35,   336,   428,   429,   336,   120,   126,   186,   213,
     249,   372,   373,   374,   376,   380,   411,   412,   413,   421,
     425,   426,   186,   327,   377,   186,   327,   346,   328,   336,
     222,   327,   186,   186,   186,   327,   188,   336,   203,   188,
     336,     3,     4,     6,     7,    10,    11,    12,    13,    27,
      29,    54,    56,    68,    69,    70,    71,    72,    73,    74,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   126,   127,   128,   129,   133,   134,
     135,   152,   156,   164,   166,   169,   176,   186,   203,   204,
     205,   216,   442,   457,   458,   460,   188,   333,   336,   189,
     229,   336,   107,   108,   155,   206,   209,   212,    80,   191,
     283,   284,   119,   126,   118,   126,    80,   285,   186,   186,
     186,   186,   203,   255,   445,   186,   186,   328,    80,    86,
     148,   149,   150,   434,   435,   155,   189,   212,   212,   203,
     256,   445,   156,   186,   445,   445,    80,   183,   189,   347,
      27,   326,   330,   336,   337,   414,   418,   218,   189,   423,
      86,   378,   434,    86,   434,   434,    30,   155,   172,   446,
     186,     9,   188,    35,   235,   156,   254,   445,   120,   182,
     236,   318,   188,   188,   188,   188,   188,   188,    10,    11,
      12,    29,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    54,    66,   188,    67,    67,   189,
     151,   127,   162,   164,   177,   179,   257,   316,   317,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    64,    65,   130,   131,   404,    67,   189,   409,
     186,   186,    67,   189,   186,   235,   236,    14,   336,   188,
     132,    45,   203,   399,    86,   326,   337,   151,   414,   132,
     193,     9,   385,   250,   326,   337,   414,   446,   151,   186,
     379,   404,   409,   187,   336,    30,   220,     8,   348,     9,
     188,   220,   221,   328,   329,   336,   203,   269,   224,   188,
     188,   188,   134,   135,   460,   460,   172,   186,   107,   460,
      14,   151,   134,   135,   152,   203,   205,   188,   188,   230,
     111,   169,   188,   155,   207,   210,   212,   155,   208,   211,
     212,   212,     9,   188,    98,   189,   414,     9,   188,   126,
     126,    14,     9,   188,   414,   438,   328,   326,   337,   414,
     417,   418,   187,   172,   247,   133,   414,   427,   428,   188,
      67,   404,   148,   435,    79,   336,   414,    86,   148,   435,
     212,   202,   188,   189,   242,   252,   362,   364,    87,   186,
     349,   350,   352,   375,   420,   422,   439,    14,    98,   440,
     343,   344,   345,   279,   280,   402,   403,   187,   187,   187,
     187,   187,   190,   219,   220,   237,   244,   251,   402,   336,
     192,   194,   195,   203,   447,   448,   460,    35,   165,   281,
     282,   336,   442,   186,   445,   245,   235,   336,   336,   336,
      30,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   376,   336,   336,   424,   424,   336,
     430,   431,   126,   189,   204,   205,   423,   255,   203,   256,
     445,   445,   254,   236,    35,   330,   333,   336,   336,   336,
     336,   336,   336,   336,   336,   336,   336,   336,   336,   336,
     156,   189,   203,   405,   406,   407,   408,   423,   424,   336,
     281,   281,   424,   336,   427,   235,   187,   336,   186,   398,
       9,   385,   187,   187,    35,   336,    35,   336,   379,   187,
     187,   187,   421,   422,   423,   281,   189,   203,   405,   406,
     423,   187,   218,   273,   189,   333,   336,   336,    90,    30,
     220,   267,   188,    28,    98,    14,     9,   187,    30,   189,
     270,   460,    29,    87,   216,   454,   455,   456,   186,     9,
      47,    48,    53,    55,    67,   134,   156,   176,   186,   213,
     214,   216,   357,   372,   380,   381,   382,   203,   459,   218,
     186,   228,   212,     9,   188,    98,   212,     9,   188,    98,
      98,   209,   203,   336,   284,   381,    80,     9,   187,   187,
     187,   187,   187,   187,   187,   188,    47,    48,   452,   453,
     128,   260,   186,     9,   187,   187,    80,    81,   203,   436,
     203,    67,   190,   190,   199,   201,    30,   129,   259,   171,
      51,   156,   171,   366,   337,   132,     9,   385,   187,   151,
     460,   460,    14,   348,   279,   218,   184,     9,   386,   460,
     461,   404,   409,   404,   190,     9,   385,   173,   414,   336,
     187,     9,   386,    14,   340,   238,   128,   258,   186,   445,
     336,    30,   193,   193,   132,   190,     9,   385,   336,   446,
     186,   248,   243,   253,    14,   440,   246,   235,    69,   414,
     336,   446,   193,   190,   187,   187,   193,   190,   187,    47,
      48,    67,    75,    76,    77,    87,   134,   147,   176,   203,
     388,   390,   391,   394,   397,   203,   414,   414,   132,   258,
     404,   409,   187,   336,   274,    72,    73,   275,   218,   327,
     218,   329,    98,    35,   133,   264,   414,   381,   203,    30,
     220,   268,   188,   271,   188,   271,     9,   173,    87,   132,
     151,     9,   385,   187,   165,   447,   448,   449,   447,   381,
     381,   381,   381,   381,   384,   387,   186,   151,   186,   381,
     151,   189,    10,    11,    12,    29,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    66,   151,
     446,   190,   372,   189,   232,    98,   210,   203,    98,   211,
     203,   203,   190,    14,   414,   188,     9,   173,   203,   261,
     372,   189,   427,   133,   414,    14,   193,   336,   190,   199,
     460,   261,   189,   365,    14,   187,   336,   349,   423,   188,
     460,   184,   190,    30,   450,   403,    35,    80,   165,   405,
     406,   408,   405,   406,   460,    35,   165,   336,   381,   279,
     186,   372,   259,   341,   239,   336,   336,   336,   190,   186,
     281,   260,    30,   259,   460,    14,   258,   445,   376,   190,
     186,    14,    75,    76,    77,   203,   389,   389,   391,   392,
     393,    49,   186,    86,   148,   186,     9,   385,   187,   398,
      35,   336,   259,   190,    72,    73,   276,   327,   220,   190,
     188,    91,   188,   264,   414,   186,   132,   263,    14,   218,
     271,   101,   102,   103,   271,   190,   460,   132,   460,   203,
     454,     9,   187,   385,   132,   193,     9,   385,   384,   204,
     349,   351,   353,   187,   126,   204,   381,   432,   433,   381,
     381,   381,    30,   381,   381,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   381,   381,   459,    80,   233,
     203,   203,   381,   453,    98,    99,   451,     9,   289,   187,
     186,   330,   333,   336,   193,   190,   440,   289,   157,   170,
     189,   361,   368,   157,   189,   367,   132,   188,   450,   460,
     348,   461,    80,   165,    14,    80,   446,   414,   336,   187,
     279,   189,   279,   186,   132,   186,   281,   187,   189,   460,
     189,   188,   460,   259,   240,   379,   281,   132,   193,     9,
     385,   390,   392,   148,   349,   395,   396,   391,   414,   189,
     327,    30,    74,   220,   188,   329,   263,   427,   264,   187,
     381,    97,   101,   188,   336,    30,   188,   272,   190,   173,
     460,   132,   165,    30,   187,   381,   381,   187,   132,     9,
     385,   187,   132,   190,     9,   385,   381,    30,   187,   218,
     203,   460,   460,   372,     4,   108,   113,   119,   121,   158,
     159,   161,   190,   290,   315,   316,   317,   322,   323,   324,
     325,   402,   427,   190,   189,   190,    51,   336,   336,   336,
     348,    35,    80,   165,    14,    80,   336,   186,   450,   187,
     289,   187,   279,   336,   281,   187,   289,   440,   289,   188,
     189,   186,   187,   391,   391,   187,   132,   187,     9,   385,
     289,    30,   218,   188,   187,   187,   187,   225,   188,   188,
     272,   218,   460,   460,   132,   381,   349,   381,   381,   381,
     189,   190,   451,   128,   129,   177,   204,   443,   460,   262,
     372,   108,   325,    29,   121,   134,   135,   156,   162,   299,
     300,   301,   302,   372,   160,   307,   308,   124,   186,   203,
     309,   310,   291,   236,   460,     9,   188,     9,   188,   188,
     440,   316,   187,   286,   156,   363,   190,   190,    80,   165,
      14,    80,   336,   281,   113,   338,   450,   190,   450,   187,
     187,   190,   189,   190,   289,   279,   132,   391,   349,   190,
     218,   223,   226,    30,   220,   266,   218,   187,   381,   132,
     132,   218,   372,   372,   445,    14,   204,     9,   188,   189,
     443,   440,   302,   172,   189,     9,   188,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    54,    68,
      69,    70,    71,    72,    73,    74,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   133,   134,   136,
     137,   138,   139,   140,   152,   153,   154,   164,   166,   167,
     169,   176,   177,   179,   181,   182,   203,   369,   370,     9,
     188,   156,   160,   203,   310,   311,   312,   188,    80,   321,
     235,   292,   443,   443,    14,   236,   190,   287,   288,   443,
      14,    80,   336,   187,   186,   189,   188,   189,   313,   338,
     450,   286,   190,   187,   391,   132,    30,   220,   265,   266,
     218,   381,   381,   190,   188,   188,   381,   372,   295,   460,
     303,   304,   380,   300,    14,    30,    48,   305,   308,     9,
      33,   187,    29,    47,    50,    14,     9,   188,   205,   444,
     321,    14,   460,   235,   188,    14,   336,    35,    80,   360,
     218,   218,   189,   313,   190,   450,   391,   218,    95,   231,
     190,   203,   216,   296,   297,   298,     9,   173,     9,   385,
     190,   381,   370,   370,    56,   306,   311,   311,    29,    47,
      50,   381,    80,   172,   186,   188,   381,   445,   381,    80,
       9,   386,   190,   190,   218,   313,    93,   188,   111,   227,
     151,    98,   460,   380,   163,    14,   452,   293,   186,    35,
      80,   187,   190,   188,   186,   169,   234,   203,   316,   317,
     173,   381,   173,   277,   278,   403,   294,    80,   372,   232,
     166,   203,   188,   187,     9,   386,   115,   116,   117,   319,
     320,   277,    80,   262,   188,   450,   403,   461,   187,   187,
     188,   188,   189,   314,   319,    35,    80,   165,   450,   189,
     218,   461,    80,   165,    14,    80,   314,   218,   190,    35,
      80,   165,    14,    80,   336,   190,    80,   165,    14,    80,
     336,    14,    80,   336,   336
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
#line 1188 "hphp.y"
    { _p->onClassExpressionStart(); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1191 "hphp.y"
    { _p->onClassExpression((yyval), (yyvsp[(3) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)])); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1198 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1230 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { (yyval).reset();;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { (yyval).reset();;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { (yyval).reset();;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval).reset();;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1299 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { (yyval).reset();;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { (yyval).reset();;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { (yyval).reset();;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1318 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    { (yyval).reset();;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1325 "hphp.y"
    { (yyval).reset();;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1333 "hphp.y"
    { (yyval).reset();;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1366 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1371 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1394 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1399 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { (yyval).reset();;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1472 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1481 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { (yyval).reset();;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { (yyval).reset();;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { (yyval).reset();;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { (yyval).reset();;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { (yyval).reset();;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval).reset();;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { (yyval).reset();;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval).reset();;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval).reset();;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { (yyval).reset();;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
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
#line 1996 "hphp.y"
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
#line 2006 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
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
#line 2024 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2030 "hphp.y"
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
#line 2041 "hphp.y"
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
#line 2049 "hphp.y"
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
#line 2056 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2064 "hphp.y"
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
#line 2074 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
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
#line 2182 "hphp.y"
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
#line 2193 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval).reset();;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { (yyval).reset();;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
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
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { (yyval).reset();;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval).reset();;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval).reset();;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval).reset();;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { (yyval).reset();;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval).reset();;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { (yyval).reset();;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { (yyval).reset();;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { (yyval).reset();;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { (yyval).reset();;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { (yyval).reset();;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
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
#line 2694 "hphp.y"
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
#line 2709 "hphp.y"
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
#line 2721 "hphp.y"
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
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
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
#line 2757 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
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
#line 2786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2823 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
    { (yyval).reset();;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
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
#line 2902 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2908 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
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
#line 2920 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { (yyval).reset();;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2997 "hphp.y"
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
#line 3007 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3025 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3033 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3087 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3091 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3097 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3113 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3135 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    {;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    {;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    {;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3166 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3171 "hphp.y"
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
#line 3182 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3187 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3188 "hphp.y"
    { ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3193 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3194 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3210 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3214 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3221 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3224 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3231 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3237 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3241 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3244 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3247 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3253 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3259 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3268 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13688 "hphp.7.tab.cpp"
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
#line 3271 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

