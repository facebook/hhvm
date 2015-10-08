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
#define YYLAST   19079

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  210
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  283
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1028
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1863

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
    2808,  2810,  2812,  2816,  2818,  2823,  2827,  2831,  2833,  2835,
    2837,  2839,  2841,  2845,  2849,  2854,  2859,  2863,  2865,  2867,
    2875,  2885,  2893,  2900,  2909,  2911,  2916,  2921,  2923,  2925,
    2930,  2933,  2935,  2936,  2938,  2940,  2942,  2946,  2950,  2954,
    2955,  2957,  2959,  2963,  2967,  2970,  2974,  2981,  2982,  2984,
    2989,  2992,  2993,  2999,  3003,  3007,  3009,  3016,  3021,  3026,
    3029,  3032,  3033,  3039,  3043,  3047,  3049,  3052,  3053,  3059,
    3063,  3067,  3069,  3072,  3075,  3077,  3080,  3082,  3087,  3091,
    3095,  3102,  3106,  3108,  3110,  3112,  3117,  3122,  3127,  3132,
    3137,  3142,  3145,  3148,  3153,  3156,  3159,  3161,  3165,  3169,
    3173,  3174,  3177,  3183,  3190,  3197,  3205,  3207,  3210,  3212,
    3215,  3217,  3222,  3224,  3229,  3233,  3234,  3236,  3240,  3243,
    3247,  3249,  3251,  3252,  3253,  3256,  3259,  3262,  3267,  3270,
    3276,  3280,  3282,  3284,  3285,  3289,  3294,  3300,  3304,  3306,
    3309,  3310,  3315,  3317,  3321,  3324,  3327,  3330,  3332,  3334,
    3336,  3338,  3342,  3347,  3354,  3356,  3365,  3372,  3374
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
      90,    -1,    98,    -1,   111,    -1,   122,    -1,   218,    -1,
     126,    -1,   220,     9,   223,    -1,   223,    -1,   224,     9,
     224,    -1,   224,    -1,   225,     9,   225,    -1,   225,    -1,
     226,    -1,   155,   226,    -1,   226,    98,   217,    -1,   155,
     226,    98,   217,    -1,   226,    -1,   155,   226,    -1,   226,
      98,   217,    -1,   155,   226,    98,   217,    -1,   226,    -1,
     155,   226,    -1,   226,    98,   217,    -1,   155,   226,    98,
     217,    -1,   217,    -1,   226,   155,   217,    -1,   226,    -1,
     152,   155,   226,    -1,   155,   226,    -1,   227,    -1,   227,
     477,    -1,   227,   477,    -1,   231,     9,   473,    14,   412,
      -1,   108,   473,    14,   412,    -1,   232,   233,    -1,    -1,
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
     122,   200,   469,   201,   202,    -1,   202,    -1,    82,    -1,
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
     249,   476,   252,   200,   291,   201,   481,   325,    -1,    -1,
     329,   250,   249,   476,   253,   200,   291,   201,   481,   325,
      -1,    -1,   433,   328,   250,   249,   476,   254,   200,   291,
     201,   481,   325,    -1,    -1,   162,   217,   256,    30,   491,
     471,   203,   298,   204,    -1,    -1,   433,   162,   217,   257,
      30,   491,   471,   203,   298,   204,    -1,    -1,   269,   266,
     259,   270,   271,   203,   301,   204,    -1,    -1,   433,   269,
     266,   260,   270,   271,   203,   301,   204,    -1,    -1,   127,
     267,   261,   272,   203,   301,   204,    -1,    -1,   433,   127,
     267,   262,   272,   203,   301,   204,    -1,    -1,   164,   268,
     264,   271,   203,   301,   204,    -1,    -1,   433,   164,   268,
     265,   271,   203,   301,   204,    -1,   476,    -1,   156,    -1,
     476,    -1,   476,    -1,   126,    -1,   119,   126,    -1,   119,
     118,   126,    -1,   118,   119,   126,    -1,   118,   126,    -1,
     128,   403,    -1,    -1,   129,   273,    -1,    -1,   128,   273,
      -1,    -1,   403,    -1,   273,     9,   403,    -1,   403,    -1,
     274,     9,   403,    -1,   132,   276,    -1,    -1,   445,    -1,
      35,   445,    -1,   133,   200,   458,   201,    -1,   234,    -1,
      30,   232,    93,   202,    -1,   234,    -1,    30,   232,    95,
     202,    -1,   234,    -1,    30,   232,    91,   202,    -1,   234,
      -1,    30,   232,    97,   202,    -1,   217,    14,   412,    -1,
     281,     9,   217,    14,   412,    -1,   203,   283,   204,    -1,
     203,   202,   283,   204,    -1,    30,   283,   101,   202,    -1,
      30,   202,   283,   101,   202,    -1,   283,   102,   348,   284,
     232,    -1,   283,   103,   284,   232,    -1,    -1,    30,    -1,
     202,    -1,   285,    72,   339,   234,    -1,    -1,   286,    72,
     339,    30,   232,    -1,    -1,    73,   234,    -1,    -1,    73,
      30,   232,    -1,    -1,   290,     9,   434,   331,   492,   165,
      80,    -1,   290,     9,   434,   331,   492,    35,   165,    80,
      -1,   290,     9,   434,   331,   492,   165,    -1,   290,   417,
      -1,   434,   331,   492,   165,    80,    -1,   434,   331,   492,
      35,   165,    80,    -1,   434,   331,   492,   165,    -1,    -1,
     434,   331,   492,    80,    -1,   434,   331,   492,    35,    80,
      -1,   434,   331,   492,    35,    80,    14,   348,    -1,   434,
     331,   492,    80,    14,   348,    -1,   290,     9,   434,   331,
     492,    80,    -1,   290,     9,   434,   331,   492,    35,    80,
      -1,   290,     9,   434,   331,   492,    35,    80,    14,   348,
      -1,   290,     9,   434,   331,   492,    80,    14,   348,    -1,
     292,     9,   434,   492,   165,    80,    -1,   292,     9,   434,
     492,    35,   165,    80,    -1,   292,     9,   434,   492,   165,
      -1,   292,   417,    -1,   434,   492,   165,    80,    -1,   434,
     492,    35,   165,    80,    -1,   434,   492,   165,    -1,    -1,
     434,   492,    80,    -1,   434,   492,    35,    80,    -1,   434,
     492,    35,    80,    14,   348,    -1,   434,   492,    80,    14,
     348,    -1,   292,     9,   434,   492,    80,    -1,   292,     9,
     434,   492,    35,    80,    -1,   292,     9,   434,   492,    35,
      80,    14,   348,    -1,   292,     9,   434,   492,    80,    14,
     348,    -1,   294,   417,    -1,    -1,   348,    -1,    35,   445,
      -1,   165,   348,    -1,   294,     9,   348,    -1,   294,     9,
     165,   348,    -1,   294,     9,    35,   445,    -1,   295,     9,
     296,    -1,   296,    -1,    80,    -1,   205,   445,    -1,   205,
     203,   348,   204,    -1,   297,     9,    80,    -1,   297,     9,
      80,    14,   412,    -1,    80,    -1,    80,    14,   412,    -1,
     298,   299,    -1,    -1,   300,   202,    -1,   474,    14,   412,
      -1,   301,   302,    -1,    -1,    -1,   327,   303,   333,   202,
      -1,    -1,   329,   491,   304,   333,   202,    -1,   334,   202,
      -1,   335,   202,    -1,   336,   202,    -1,    -1,   328,   250,
     249,   475,   200,   305,   289,   201,   481,   326,    -1,    -1,
     433,   328,   250,   249,   476,   200,   306,   289,   201,   481,
     326,    -1,   158,   311,   202,    -1,   159,   319,   202,    -1,
     161,   321,   202,    -1,     4,   128,   403,   202,    -1,     4,
     129,   403,   202,    -1,   113,   274,   202,    -1,   113,   274,
     203,   307,   204,    -1,   307,   308,    -1,   307,   309,    -1,
      -1,   230,   151,   217,   166,   274,   202,    -1,   310,    98,
     328,   217,   202,    -1,   310,    98,   329,   202,    -1,   230,
     151,   217,    -1,   217,    -1,   312,    -1,   311,     9,   312,
      -1,   313,   400,   317,   318,    -1,   156,    -1,    29,   314,
      -1,   314,    -1,   134,    -1,   134,   172,   491,   173,    -1,
     134,   172,   491,     9,   491,   173,    -1,   403,    -1,   121,
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
     474,    14,   412,    -1,   108,   474,    14,   412,    -1,   335,
       9,   474,    -1,   119,   108,   474,    -1,   119,   337,   471,
      -1,   337,   471,    14,   491,    -1,   108,   177,   476,    -1,
     200,   338,   201,    -1,    69,   407,   410,    -1,    68,   348,
      -1,   392,    -1,   368,    -1,   200,   348,   201,    -1,   340,
       9,   348,    -1,   348,    -1,   340,    -1,    -1,    27,    -1,
      27,   348,    -1,    27,   348,   132,   348,    -1,   200,   342,
     201,    -1,   445,    14,   342,    -1,   133,   200,   458,   201,
      14,   342,    -1,    28,   348,    -1,   445,    14,   345,    -1,
     133,   200,   458,   201,    14,   345,    -1,   349,    -1,   445,
      -1,   338,    -1,   449,    -1,   448,    -1,   133,   200,   458,
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
     348,    29,    30,   348,    -1,   468,    -1,    63,   348,    -1,
      62,   348,    -1,    61,   348,    -1,    60,   348,    -1,    59,
     348,    -1,    58,   348,    -1,    57,   348,    -1,    70,   408,
      -1,    56,   348,    -1,   414,    -1,   367,    -1,   366,    -1,
     206,   409,   206,    -1,    13,   348,    -1,   370,    -1,   113,
     200,   391,   417,   201,    -1,    -1,    -1,   250,   249,   200,
     352,   291,   201,   481,   350,   203,   232,   204,    -1,    -1,
     329,   250,   249,   200,   353,   291,   201,   481,   350,   203,
     232,   204,    -1,    -1,    80,   355,   360,    -1,    -1,   182,
      80,   356,   360,    -1,    -1,   197,   357,   291,   198,   481,
     360,    -1,    -1,   182,   197,   358,   291,   198,   481,   360,
      -1,    -1,   182,   203,   359,   232,   204,    -1,     8,   348,
      -1,     8,   345,    -1,     8,   203,   232,   204,    -1,    87,
      -1,   470,    -1,   362,     9,   361,   132,   348,    -1,   361,
     132,   348,    -1,   363,     9,   361,   132,   412,    -1,   361,
     132,   412,    -1,   362,   416,    -1,    -1,   363,   416,    -1,
      -1,   176,   200,   364,   201,    -1,   134,   200,   459,   201,
      -1,    67,   459,   207,    -1,   403,   203,   461,   204,    -1,
     403,   203,   463,   204,    -1,   370,    67,   455,   207,    -1,
     371,    67,   455,   207,    -1,   367,    -1,   470,    -1,   448,
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
     200,   349,   201,    -1,   404,    -1,   405,   151,   454,    -1,
     404,    -1,   451,    -1,   406,   151,   454,    -1,   403,    -1,
     120,    -1,   456,    -1,   200,   201,    -1,   339,    -1,    -1,
      -1,    86,    -1,   465,    -1,   200,   293,   201,    -1,    -1,
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
      79,    -1,   470,    -1,   411,    -1,   208,   465,   208,    -1,
     209,   465,   209,    -1,   147,   465,   148,    -1,   418,   416,
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
     454,    -1,   217,    -1,   203,   348,   204,    -1,   438,    -1,
     454,    -1,    67,   455,   207,    -1,   203,   348,   204,    -1,
     446,   440,    -1,   200,   338,   201,   440,    -1,   457,   440,
      -1,   200,   338,   201,   440,    -1,   200,   338,   201,   435,
     437,    -1,   200,   349,   201,   435,   437,    -1,   200,   338,
     201,   435,   436,    -1,   200,   349,   201,   435,   436,    -1,
     452,    -1,   402,    -1,   450,    -1,   451,    -1,   441,    -1,
     443,    -1,   445,   435,   437,    -1,   406,   151,   454,    -1,
     447,   200,   293,   201,    -1,   448,   200,   293,   201,    -1,
     200,   445,   201,    -1,   402,    -1,   450,    -1,   451,    -1,
     441,    -1,   445,   435,   437,    -1,   444,    -1,   447,   200,
     293,   201,    -1,   200,   445,   201,    -1,   406,   151,   454,
      -1,   452,    -1,   441,    -1,   402,    -1,   367,    -1,   411,
      -1,   200,   445,   201,    -1,   200,   349,   201,    -1,   448,
     200,   293,   201,    -1,   447,   200,   293,   201,    -1,   200,
     449,   201,    -1,   351,    -1,   354,    -1,   445,   435,   439,
     477,   200,   293,   201,    -1,   200,   338,   201,   435,   439,
     477,   200,   293,   201,    -1,   406,   151,   219,   477,   200,
     293,   201,    -1,   406,   151,   454,   200,   293,   201,    -1,
     406,   151,   203,   348,   204,   200,   293,   201,    -1,   453,
      -1,   453,    67,   455,   207,    -1,   453,   203,   348,   204,
      -1,   454,    -1,    80,    -1,   205,   203,   348,   204,    -1,
     205,   454,    -1,   348,    -1,    -1,   452,    -1,   442,    -1,
     443,    -1,   456,   435,   437,    -1,   405,   151,   452,    -1,
     200,   445,   201,    -1,    -1,   442,    -1,   444,    -1,   456,
     435,   436,    -1,   200,   445,   201,    -1,   458,     9,    -1,
     458,     9,   445,    -1,   458,     9,   133,   200,   458,   201,
      -1,    -1,   445,    -1,   133,   200,   458,   201,    -1,   460,
     416,    -1,    -1,   460,     9,   348,   132,   348,    -1,   460,
       9,   348,    -1,   348,   132,   348,    -1,   348,    -1,   460,
       9,   348,   132,    35,   445,    -1,   460,     9,    35,   445,
      -1,   348,   132,    35,   445,    -1,    35,   445,    -1,   462,
     416,    -1,    -1,   462,     9,   348,   132,   348,    -1,   462,
       9,   348,    -1,   348,   132,   348,    -1,   348,    -1,   464,
     416,    -1,    -1,   464,     9,   412,   132,   412,    -1,   464,
       9,   412,    -1,   412,   132,   412,    -1,   412,    -1,   465,
     466,    -1,   465,    86,    -1,   466,    -1,    86,   466,    -1,
      80,    -1,    80,    67,   467,   207,    -1,    80,   435,   217,
      -1,   149,   348,   204,    -1,   149,    79,    67,   348,   207,
     204,    -1,   150,   445,   204,    -1,   217,    -1,    81,    -1,
      80,    -1,   123,   200,   340,   201,    -1,   124,   200,   445,
     201,    -1,   124,   200,   349,   201,    -1,   124,   200,   449,
     201,    -1,   124,   200,   448,   201,    -1,   124,   200,   338,
     201,    -1,     7,   348,    -1,     6,   348,    -1,     5,   200,
     348,   201,    -1,     4,   348,    -1,     3,   348,    -1,   445,
      -1,   469,     9,   445,    -1,   406,   151,   218,    -1,   406,
     151,   126,    -1,    -1,    98,   491,    -1,   177,   476,    14,
     491,   202,    -1,   433,   177,   476,    14,   491,   202,    -1,
     179,   476,   471,    14,   491,   202,    -1,   433,   179,   476,
     471,    14,   491,   202,    -1,   219,    -1,   491,   219,    -1,
     218,    -1,   491,   218,    -1,   219,    -1,   219,   172,   483,
     173,    -1,   217,    -1,   217,   172,   483,   173,    -1,   172,
     479,   173,    -1,    -1,   491,    -1,   478,     9,   491,    -1,
     478,   416,    -1,   478,     9,   165,    -1,   479,    -1,   165,
      -1,    -1,    -1,    30,   491,    -1,    98,   491,    -1,    99,
     491,    -1,   483,     9,   484,   217,    -1,   484,   217,    -1,
     483,     9,   484,   217,   482,    -1,   484,   217,   482,    -1,
      47,    -1,    48,    -1,    -1,    87,   132,   491,    -1,    29,
      87,   132,   491,    -1,   230,   151,   217,   132,   491,    -1,
     486,     9,   485,    -1,   485,    -1,   486,   416,    -1,    -1,
     176,   200,   487,   201,    -1,   230,    -1,   217,   151,   490,
      -1,   217,   477,    -1,    29,   491,    -1,    56,   491,    -1,
     230,    -1,   134,    -1,   135,    -1,   488,    -1,   489,   151,
     490,    -1,   134,   172,   491,   173,    -1,   134,   172,   491,
       9,   491,   173,    -1,   156,    -1,   200,   107,   200,   480,
     201,    30,   491,   201,    -1,   200,   491,     9,   478,   416,
     201,    -1,   491,    -1,    -1
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
     860,   861,   862,   866,   870,   871,   875,   877,   881,   883,
     887,   889,   893,   894,   895,   896,   901,   902,   903,   904,
     909,   910,   911,   912,   917,   918,   922,   923,   925,   928,
     934,   941,   948,   952,   958,   960,   963,   964,   965,   966,
     969,   970,   974,   979,   979,   985,   985,   992,   991,   997,
     997,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,  1014,  1015,  1016,  1020,  1018,  1027,  1025,
    1032,  1040,  1034,  1044,  1042,  1046,  1047,  1051,  1052,  1053,
    1054,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1069,  1069,
    1074,  1080,  1084,  1084,  1092,  1093,  1097,  1098,  1102,  1108,
    1106,  1121,  1118,  1134,  1131,  1148,  1147,  1156,  1154,  1166,
    1165,  1184,  1182,  1201,  1200,  1209,  1207,  1219,  1218,  1230,
    1228,  1241,  1242,  1246,  1249,  1252,  1253,  1254,  1257,  1258,
    1261,  1263,  1266,  1267,  1270,  1271,  1274,  1275,  1279,  1280,
    1285,  1286,  1289,  1290,  1291,  1295,  1296,  1300,  1301,  1305,
    1306,  1310,  1311,  1316,  1317,  1322,  1323,  1324,  1325,  1328,
    1331,  1333,  1336,  1337,  1341,  1343,  1346,  1349,  1352,  1353,
    1356,  1357,  1361,  1367,  1373,  1380,  1382,  1387,  1392,  1398,
    1402,  1406,  1410,  1415,  1420,  1425,  1430,  1436,  1445,  1450,
    1455,  1461,  1463,  1467,  1471,  1476,  1480,  1483,  1486,  1490,
    1494,  1498,  1502,  1507,  1515,  1517,  1520,  1521,  1522,  1523,
    1525,  1527,  1532,  1533,  1536,  1537,  1538,  1542,  1543,  1545,
    1546,  1550,  1552,  1555,  1559,  1565,  1567,  1570,  1570,  1574,
    1573,  1577,  1579,  1582,  1585,  1583,  1599,  1595,  1609,  1611,
    1613,  1615,  1617,  1619,  1621,  1625,  1626,  1627,  1630,  1636,
    1640,  1646,  1649,  1654,  1656,  1661,  1666,  1670,  1671,  1675,
    1676,  1678,  1680,  1686,  1687,  1689,  1693,  1694,  1699,  1703,
    1704,  1708,  1709,  1713,  1715,  1721,  1726,  1727,  1729,  1733,
    1734,  1735,  1736,  1740,  1741,  1742,  1743,  1744,  1745,  1747,
    1752,  1755,  1756,  1760,  1761,  1765,  1766,  1769,  1770,  1773,
    1774,  1777,  1778,  1782,  1783,  1784,  1785,  1786,  1787,  1788,
    1792,  1793,  1796,  1797,  1798,  1801,  1803,  1805,  1806,  1809,
    1811,  1815,  1817,  1821,  1825,  1829,  1834,  1835,  1837,  1838,
    1839,  1842,  1846,  1847,  1851,  1852,  1856,  1857,  1858,  1859,
    1863,  1867,  1872,  1876,  1880,  1885,  1886,  1887,  1888,  1889,
    1893,  1895,  1896,  1897,  1900,  1901,  1902,  1903,  1904,  1905,
    1906,  1907,  1908,  1909,  1910,  1911,  1912,  1913,  1914,  1915,
    1916,  1917,  1918,  1919,  1920,  1921,  1922,  1923,  1924,  1925,
    1926,  1927,  1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,
    1936,  1937,  1938,  1939,  1940,  1941,  1942,  1944,  1945,  1947,
    1948,  1950,  1951,  1952,  1953,  1954,  1955,  1956,  1957,  1958,
    1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,  1967,  1968,
    1972,  1976,  1981,  1980,  1995,  1993,  2010,  2010,  2026,  2025,
    2043,  2043,  2059,  2058,  2077,  2076,  2097,  2098,  2099,  2104,
    2106,  2110,  2114,  2120,  2124,  2130,  2132,  2136,  2138,  2142,
    2146,  2147,  2151,  2158,  2165,  2167,  2172,  2173,  2174,  2175,
    2177,  2181,  2185,  2189,  2193,  2195,  2197,  2199,  2204,  2205,
    2210,  2211,  2212,  2213,  2214,  2215,  2219,  2223,  2227,  2231,
    2236,  2241,  2245,  2246,  2250,  2251,  2255,  2256,  2260,  2261,
    2265,  2269,  2273,  2277,  2278,  2279,  2280,  2284,  2290,  2299,
    2312,  2313,  2316,  2319,  2322,  2323,  2326,  2330,  2333,  2336,
    2343,  2344,  2348,  2349,  2351,  2355,  2356,  2357,  2358,  2359,
    2360,  2361,  2362,  2363,  2364,  2365,  2366,  2367,  2368,  2369,
    2370,  2371,  2372,  2373,  2374,  2375,  2376,  2377,  2378,  2379,
    2380,  2381,  2382,  2383,  2384,  2385,  2386,  2387,  2388,  2389,
    2390,  2391,  2392,  2393,  2394,  2395,  2396,  2397,  2398,  2399,
    2400,  2401,  2402,  2403,  2404,  2405,  2406,  2407,  2408,  2409,
    2410,  2411,  2412,  2413,  2414,  2415,  2416,  2417,  2418,  2419,
    2420,  2421,  2422,  2423,  2424,  2425,  2426,  2427,  2428,  2429,
    2430,  2431,  2432,  2433,  2434,  2438,  2443,  2444,  2448,  2449,
    2450,  2451,  2453,  2457,  2458,  2469,  2470,  2472,  2484,  2485,
    2486,  2490,  2491,  2492,  2496,  2497,  2498,  2501,  2503,  2507,
    2508,  2509,  2510,  2512,  2513,  2514,  2515,  2516,  2517,  2518,
    2519,  2520,  2521,  2524,  2529,  2530,  2531,  2533,  2534,  2536,
    2537,  2538,  2539,  2541,  2543,  2545,  2547,  2549,  2550,  2551,
    2552,  2553,  2554,  2555,  2556,  2557,  2558,  2559,  2560,  2561,
    2562,  2563,  2564,  2565,  2567,  2569,  2571,  2573,  2574,  2577,
    2578,  2582,  2586,  2588,  2592,  2595,  2598,  2604,  2605,  2606,
    2607,  2608,  2609,  2610,  2615,  2617,  2621,  2622,  2625,  2626,
    2630,  2633,  2635,  2637,  2641,  2642,  2643,  2644,  2647,  2651,
    2652,  2653,  2654,  2658,  2660,  2667,  2668,  2669,  2670,  2671,
    2672,  2674,  2675,  2680,  2682,  2685,  2688,  2690,  2692,  2695,
    2697,  2701,  2703,  2706,  2709,  2715,  2717,  2720,  2721,  2726,
    2729,  2733,  2733,  2738,  2741,  2742,  2746,  2747,  2751,  2752,
    2753,  2757,  2762,  2767,  2768,  2772,  2777,  2782,  2783,  2787,
    2788,  2793,  2795,  2800,  2811,  2825,  2837,  2852,  2853,  2854,
    2855,  2856,  2857,  2858,  2868,  2877,  2879,  2881,  2885,  2886,
    2887,  2888,  2889,  2905,  2906,  2908,  2910,  2917,  2918,  2919,
    2920,  2921,  2922,  2923,  2924,  2926,  2931,  2935,  2936,  2940,
    2943,  2950,  2954,  2963,  2970,  2978,  2980,  2981,  2985,  2986,
    2988,  2993,  2994,  3005,  3006,  3007,  3008,  3019,  3022,  3025,
    3026,  3027,  3028,  3039,  3043,  3044,  3045,  3047,  3048,  3049,
    3053,  3055,  3058,  3060,  3061,  3062,  3063,  3066,  3068,  3069,
    3073,  3075,  3078,  3080,  3081,  3082,  3086,  3088,  3091,  3094,
    3096,  3098,  3102,  3103,  3105,  3106,  3112,  3113,  3115,  3125,
    3127,  3129,  3132,  3133,  3134,  3138,  3139,  3140,  3141,  3142,
    3143,  3144,  3145,  3146,  3147,  3148,  3152,  3153,  3157,  3159,
    3167,  3169,  3173,  3177,  3182,  3186,  3194,  3195,  3199,  3200,
    3206,  3207,  3216,  3217,  3225,  3228,  3232,  3235,  3240,  3245,
    3247,  3248,  3249,  3253,  3254,  3258,  3259,  3262,  3265,  3267,
    3271,  3277,  3278,  3279,  3283,  3287,  3297,  3305,  3307,  3311,
    3313,  3318,  3324,  3327,  3332,  3340,  3343,  3346,  3347,  3350,
    3353,  3354,  3359,  3362,  3366,  3370,  3376,  3386,  3387
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
     446,   446,   446,   446,   446,   446,   446,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   448,   449,   449,   450,
     450,   451,   451,   451,   452,   453,   453,   453,   454,   454,
     454,   455,   455,   456,   456,   456,   456,   456,   456,   457,
     457,   457,   457,   457,   458,   458,   458,   458,   458,   458,
     459,   459,   460,   460,   460,   460,   460,   460,   460,   460,
     461,   461,   462,   462,   462,   462,   463,   463,   464,   464,
     464,   464,   465,   465,   465,   465,   466,   466,   466,   466,
     466,   466,   467,   467,   467,   468,   468,   468,   468,   468,
     468,   468,   468,   468,   468,   468,   469,   469,   470,   470,
     471,   471,   472,   472,   472,   472,   473,   473,   474,   474,
     475,   475,   476,   476,   477,   477,   478,   478,   479,   480,
     480,   480,   480,   481,   481,   482,   482,   483,   483,   483,
     483,   484,   484,   484,   485,   485,   485,   486,   486,   487,
     487,   488,   489,   490,   490,   491,   491,   491,   491,   491,
     491,   491,   491,   491,   491,   491,   491,   492,   492
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
       1,     1,     3,     1,     4,     3,     3,     1,     1,     1,
       1,     1,     3,     3,     4,     4,     3,     1,     1,     7,
       9,     7,     6,     8,     1,     4,     4,     1,     1,     4,
       2,     1,     0,     1,     1,     1,     3,     3,     3,     0,
       1,     1,     3,     3,     2,     3,     6,     0,     1,     4,
       2,     0,     5,     3,     3,     1,     6,     4,     4,     2,
       2,     0,     5,     3,     3,     1,     2,     0,     5,     3,
       3,     1,     2,     2,     1,     2,     1,     4,     3,     3,
       6,     3,     1,     1,     1,     4,     4,     4,     4,     4,
       4,     2,     2,     4,     2,     2,     1,     3,     3,     3,
       0,     2,     5,     6,     6,     7,     1,     2,     1,     2,
       1,     4,     1,     4,     3,     0,     1,     3,     2,     3,
       1,     1,     0,     0,     2,     2,     2,     4,     2,     5,
       3,     1,     1,     0,     3,     4,     5,     3,     1,     2,
       0,     4,     1,     3,     2,     2,     2,     1,     1,     1,
       1,     3,     4,     6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   436,     0,   831,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   921,     0,
     909,   713,     0,   719,   720,   721,    22,   778,   898,   164,
     165,   722,     0,   145,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   198,     0,     0,     0,     0,     0,     0,
     403,   404,   405,   408,   407,   406,     0,     0,     0,     0,
     225,     0,     0,     0,   726,   728,   729,   723,   724,     0,
       0,     0,   730,   725,     0,   697,    24,    25,    26,    28,
      27,     0,   727,     0,     0,     0,     0,   731,   409,    29,
      30,    32,    31,    33,    34,    35,    36,    37,    38,    39,
      40,    41,   530,     0,   163,   135,     0,   714,     0,     0,
       4,   124,   126,   129,   777,     0,   696,     0,     6,   197,
       7,     9,     8,    10,     0,     0,   401,   447,     0,     0,
       0,     0,     0,     0,     0,   445,   887,   888,   516,   515,
     430,   519,     0,     0,   429,   858,   698,   705,     0,   780,
     514,   400,   861,   862,   873,   446,     0,     0,   449,   448,
     859,   860,   857,   894,   897,   504,   779,    11,   408,   407,
     406,     0,     0,    28,     0,   124,   197,     0,   965,   446,
     964,     0,   962,   961,   518,     0,   437,   442,     0,     0,
     487,   488,   489,   490,   513,   511,   510,   509,   508,   507,
     506,   505,   898,   722,   700,     0,     0,   985,   880,   698,
       0,   699,   469,     0,   467,     0,   925,     0,   787,   428,
     709,     0,   985,   708,   703,     0,   718,   699,   904,   905,
     911,   903,   710,     0,     0,   712,   512,     0,     0,     0,
       0,   433,     0,   143,   435,     0,     0,   149,   151,     0,
       0,   153,     0,    82,    81,    76,    75,    67,    68,    59,
      79,    90,     0,    62,     0,    74,    66,    72,    92,    85,
      84,    57,    80,    99,   100,    58,    95,    55,    96,    56,
      97,    54,   101,    89,    93,    98,    86,    87,    61,    88,
      91,    53,    83,    69,   102,    77,    70,    60,    52,    51,
      50,    49,    48,    47,    71,   103,   105,    64,    45,    46,
      73,  1018,  1019,    65,  1024,    44,    63,    94,     0,     0,
     124,   104,   976,  1017,     0,  1020,     0,     0,     0,   155,
       0,     0,     0,     0,   188,     0,     0,     0,     0,     0,
       0,   107,   112,   314,     0,     0,   313,     0,   229,     0,
     226,   319,     0,     0,     0,     0,     0,   982,   213,   223,
     917,   921,     0,   946,     0,   733,     0,     0,     0,   944,
       0,    16,     0,   128,   205,   217,   224,   603,   546,     0,
     970,   528,   532,   534,   835,   436,   447,     0,     0,   445,
     446,   448,     0,     0,   900,   715,     0,   716,     0,     0,
       0,   187,     0,     0,   131,   305,     0,    21,   196,     0,
     222,   209,   221,   406,   409,   197,   402,   178,   179,   180,
     181,   182,   184,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   909,
       0,   177,   902,   902,   185,   931,     0,     0,     0,     0,
       0,     0,     0,     0,   399,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   468,   466,
     836,   837,     0,   902,     0,   849,   305,   305,   902,     0,
     917,     0,   197,     0,     0,   157,     0,   833,   828,   787,
       0,   447,   445,     0,   929,     0,   551,   786,   920,   447,
     445,   446,   131,     0,   305,   427,     0,   851,   711,     0,
     135,   265,     0,   527,     0,   160,     0,     0,   434,     0,
       0,     0,     0,     0,   152,   176,   154,  1018,  1019,  1015,
    1016,     0,  1010,     0,     0,     0,     0,    78,    43,    65,
      42,   977,     0,   183,   156,   186,     0,     0,     0,     0,
       0,     0,     0,   561,     0,   568,   570,   571,   572,   573,
     574,   575,   566,   588,   589,   135,     0,   173,   175,     0,
       0,   109,   116,     0,     0,   111,   120,   113,     0,    18,
       0,     0,   315,     0,   158,   228,   227,     0,     0,   159,
     966,     0,     0,   447,   445,   446,   449,   448,     0,  1003,
     235,     0,   918,     0,     0,   161,     0,     0,   732,   945,
     778,     0,     0,   943,   783,   942,   127,     5,    13,    14,
       0,   233,     0,     0,   539,     0,     0,   787,     0,     0,
     706,   701,   540,     0,     0,     0,     0,   835,   135,     0,
     789,   834,  1028,   426,   439,   501,   867,   886,   140,   134,
     136,   137,   138,   139,   400,     0,   517,   781,   782,   125,
     787,     0,   986,     0,     0,     0,   789,   306,     0,   522,
     199,   231,     0,   472,   474,   473,     0,     0,   470,   471,
     475,   477,   476,   492,   491,   494,   493,   495,   497,   499,
     498,   496,   486,   485,   479,   480,   478,   481,   482,   484,
     500,   483,   901,     0,     0,   935,     0,   787,   969,     0,
     968,   985,   864,   215,   207,   219,     0,   970,   211,   197,
       0,   440,   443,   451,   562,   465,   464,   463,   462,   461,
     460,   459,   458,   457,   456,   455,   454,   839,     0,   838,
     841,   863,   845,   985,   842,     0,     0,     0,     0,     0,
       0,     0,     0,   963,   438,   826,   830,   786,   832,     0,
     702,     0,   924,     0,   923,     0,   702,   908,   907,   894,
     897,     0,     0,   838,   841,   906,   842,   431,   267,   269,
     135,   537,   536,   432,     0,   135,   249,   144,   435,     0,
       0,     0,     0,     0,   261,   261,   150,     0,     0,     0,
       0,  1008,   787,     0,   992,     0,     0,     0,     0,     0,
     785,     0,   697,     0,     0,   129,   735,   696,   740,     0,
     734,   133,   739,   985,  1021,     0,     0,   578,     0,     0,
     584,   581,   582,   590,     0,   569,   564,     0,   567,     0,
       0,     0,   117,    19,     0,     0,   121,    20,     0,     0,
       0,   106,   114,     0,   312,   320,   317,     0,     0,   955,
     960,   957,   956,   959,   958,    12,  1001,  1002,     0,     0,
       0,     0,   917,   914,     0,   550,   954,   953,   952,     0,
     948,     0,   949,   951,     0,     5,     0,     0,     0,   597,
     598,   606,   605,     0,   445,     0,   786,   545,   549,     0,
       0,   971,     0,   529,     0,     0,   993,   835,   291,  1027,
       0,     0,   850,     0,   899,   786,   988,   984,   307,   308,
     695,   788,   304,     0,   835,     0,     0,   233,   524,   201,
     503,     0,   554,   555,     0,   552,   786,   930,     0,     0,
     305,   235,     0,   233,     0,     0,   231,     0,   909,   452,
       0,     0,   847,   848,   865,   866,   895,   896,     0,     0,
       0,   814,   794,   795,   796,   803,     0,     0,     0,   807,
     805,   806,   820,   787,     0,   828,   928,   927,     0,     0,
     852,   717,     0,   271,     0,     0,   141,     0,     0,     0,
       0,     0,     0,     0,   241,   242,   253,     0,   135,   251,
     170,   261,     0,   261,     0,     0,  1022,     0,     0,     0,
     786,  1009,  1011,   991,   787,   990,     0,   787,   761,   762,
     759,   760,   793,     0,   787,   785,     0,   548,     0,     0,
     937,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1014,
     563,     0,     0,     0,   586,   587,   585,     0,     0,   565,
       0,   189,     0,   192,   174,     0,   108,   118,     0,   110,
     122,   115,   316,     0,   967,   162,  1003,   983,   998,   234,
     236,   326,     0,     0,   915,     0,   947,     0,    17,     0,
     970,   232,   326,     0,     0,   702,   542,     0,   707,   972,
       0,   993,   535,     0,     0,  1028,     0,   296,   294,   841,
     853,   985,   841,   854,   987,     0,     0,   309,   132,     0,
     835,   230,     0,   835,     0,   502,   934,   933,     0,   305,
       0,     0,     0,     0,     0,     0,   233,   203,   718,   840,
     305,     0,   799,   800,   801,   802,   808,   809,   818,     0,
     787,     0,   814,     0,   798,   822,   786,   825,   827,   829,
       0,   922,   840,     0,     0,     0,     0,   268,   538,   146,
       0,   435,   241,   243,   917,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   255,     0,     0,  1004,     0,  1007,
     786,     0,     0,     0,   737,   786,   784,     0,   775,     0,
     787,     0,   741,   776,   774,   941,     0,   787,   744,   746,
     745,     0,     0,   742,   743,   747,   749,   748,   764,   763,
     766,   765,   767,   769,   771,   770,   768,   757,   756,   751,
     752,   750,   753,   754,   755,   758,  1013,   576,     0,   577,
     583,   591,   592,     0,   135,   119,   123,   318,     0,     0,
       0,  1000,     0,   400,   919,   917,   441,   444,   450,     0,
      15,     0,   400,   609,     0,     0,   611,   604,   607,     0,
     602,     0,   974,     0,   994,   531,     0,   297,     0,     0,
     292,     0,   311,   310,   993,     0,   326,     0,   835,     0,
     305,     0,   892,   326,   970,   326,   973,     0,     0,     0,
     453,     0,     0,   811,   786,   813,   804,     0,   797,     0,
       0,   787,   819,   926,     0,   135,     0,   264,   250,     0,
       0,     0,   240,   166,   254,     0,     0,   257,     0,   262,
     263,   135,   256,  1023,  1005,     0,   989,     0,  1026,   792,
     791,   736,     0,   786,   547,   738,     0,   553,   786,   936,
     773,     0,     0,     0,     0,   997,   995,   996,   237,     0,
       0,     0,   407,   398,     0,     0,     0,   214,   325,   327,
       0,   397,     0,     0,     0,   970,   400,     0,   950,   322,
     218,   600,     0,     0,   541,   533,     0,   300,   290,     0,
     293,   299,   305,   521,   993,   400,   993,     0,   932,     0,
     891,   400,     0,   400,   975,   326,   835,   889,   817,   816,
     810,     0,   812,   786,   821,   135,   270,   142,   147,   168,
     244,     0,   252,   258,   135,   260,  1006,     0,     0,   544,
       0,   940,   939,   772,     0,   135,   193,   999,     0,     0,
       0,   978,     0,     0,     0,   238,     0,   970,     0,   363,
     359,   365,   697,    28,     0,   353,     0,   358,   362,   375,
       0,   373,   378,     0,   377,     0,   376,     0,   197,   329,
       0,   331,     0,   332,   333,     0,     0,   916,     0,   601,
     599,   610,   608,   301,     0,     0,   288,   298,     0,     0,
       0,     0,   210,   521,   993,   893,   216,   322,   220,   400,
       0,     0,   824,     0,   266,     0,     0,   135,   247,   167,
     259,  1025,   790,     0,     0,     0,     0,     0,     0,   425,
       0,   979,     0,   343,   347,   422,   423,   357,     0,     0,
       0,   338,   661,   660,   657,   659,   658,   678,   680,   679,
     649,   620,   621,   639,   655,   654,   616,   626,   627,   629,
     628,   648,   632,   630,   631,   633,   634,   635,   636,   637,
     638,   640,   641,   642,   643,   644,   645,   647,   646,   617,
     618,   619,   622,   623,   625,   663,   664,   673,   672,   671,
     670,   669,   668,   656,   675,   665,   666,   667,   650,   651,
     652,   653,   676,   677,   681,   683,   682,   684,   685,   662,
     687,   686,   689,   691,   690,   624,   694,   692,   693,   688,
     674,   615,   370,   612,     0,   339,   391,   392,   390,   383,
       0,   384,   340,   417,     0,     0,     0,     0,   421,     0,
     197,   206,   321,     0,     0,     0,   289,   303,   890,     0,
     135,   393,   135,   200,     0,     0,     0,   212,   993,   815,
       0,   135,   245,   148,   169,     0,   543,   938,   579,   191,
     341,   342,   420,   239,     0,     0,   787,     0,   366,   354,
       0,     0,     0,   372,   374,     0,     0,   379,   386,   387,
     385,     0,     0,   328,   980,     0,     0,     0,   424,     0,
     323,     0,   302,     0,   595,   789,     0,     0,   135,   202,
     208,     0,   823,     0,     0,     0,   171,   344,   124,     0,
     345,   346,     0,     0,   360,   786,   368,   364,   369,   613,
     614,     0,   355,   388,   389,   381,   382,   380,   418,   415,
    1003,   334,   330,   419,     0,   324,   596,   788,     0,   523,
     394,     0,   204,     0,   248,   580,     0,   195,     0,   400,
       0,   367,   371,     0,     0,   835,   336,     0,   593,   520,
     525,   246,     0,     0,   172,   351,     0,   399,   361,   416,
     981,     0,   789,   411,   835,   594,     0,   194,     0,     0,
     350,   993,   835,   275,   412,   413,   414,  1028,   410,     0,
       0,     0,   349,     0,   411,     0,   993,     0,   348,   395,
     135,   335,  1028,     0,   280,   278,     0,   135,     0,     0,
     281,     0,     0,   276,   337,     0,   396,     0,   284,   274,
       0,   277,   283,   190,   285,     0,     0,   272,   282,     0,
     273,   287,   286
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   905,   637,   185,  1471,   731,
     350,   590,   594,   351,   591,   595,   122,   123,   124,   125,
     126,   127,   402,   669,   670,   537,   252,  1535,   543,  1451,
    1536,  1777,   861,   345,   585,  1736,  1094,  1274,  1794,   419,
     186,   671,   945,  1154,  1329,   131,   640,   962,   672,   691,
     966,   620,   961,   673,   641,   963,   421,   368,   385,   134,
     947,   908,   891,  1109,  1474,  1206,  1014,  1683,  1539,   807,
    1020,   542,   816,  1022,  1361,   799,  1003,  1006,  1195,  1801,
    1802,   659,   660,   685,   686,   355,   356,   362,  1508,  1662,
    1663,  1283,  1398,  1497,  1656,  1785,  1804,  1694,  1740,  1741,
    1742,  1484,  1485,  1486,  1487,  1696,  1697,  1703,  1752,  1490,
    1491,  1495,  1649,  1650,  1651,  1673,  1831,  1399,  1400,   187,
     136,  1817,  1818,  1654,  1402,  1403,  1404,  1405,   137,   245,
     538,   539,   138,   139,   140,   141,   142,   143,   144,   145,
    1520,   146,   944,  1153,   147,   249,   656,   394,   657,   658,
     533,   646,   647,  1230,   648,  1231,   148,   149,   150,   838,
     151,   152,   342,   153,   343,   573,   574,   575,   576,   577,
     578,   579,   580,   581,   851,   852,  1086,   582,   583,   584,
     858,  1725,   154,   642,  1510,   643,  1123,   913,  1300,  1297,
    1642,  1643,   155,   156,   157,   235,   158,   236,   246,   406,
     525,   159,  1042,   842,   160,  1043,   936,   928,  1044,   990,
    1176,   991,  1178,  1179,  1180,   993,  1340,  1341,   994,   776,
     509,   198,   199,   674,   662,   492,  1139,  1140,   762,   763,
     932,   162,   238,   163,   164,   189,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   723,   242,   243,   623,   227,
     228,   726,   727,  1236,  1237,   378,   379,   899,   175,   611,
     176,   655,   177,   334,  1664,  1715,   369,   414,   680,   681,
    1036,  1134,  1281,   888,   889,   821,   822,   823,   335,   336,
     844,  1473,   930
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1528
static const yytype_int16 yypact[] =
{
   -1528,   223, -1528, -1528,  5678, 13751, 13751,   -38, 13751, 13751,
   13751, 11474, 13751, -1528, 13751, 13751, 13751, 13751, 13751, 13751,
   13751, 13751, 13751, 13751, 13751, 13751, 17565, 17565, 11681, 13751,
   17691,   -12,    38, -1528, -1528, -1528, -1528, -1528,   244, -1528,
   -1528,   348, 13751, -1528,    38,   222,   267,   306, -1528,    38,
   11888, 14698, 12095, -1528, 14841, 10439,    52, 13751, 17756,   167,
   -1528, -1528, -1528,    44,    40,    67,   322,   357,   361,   388,
   -1528, 14698,   395,   398, -1528, -1528, -1528, -1528, -1528, 13751,
     461, 17630, -1528, -1528, 14698, -1528, -1528, -1528, -1528, 14698,
   -1528, 14698, -1528,   134,   404, 14698, 14698, -1528,   179, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, 12302, -1528, -1528,   174,   472,   547,   547,
   -1528,   387,   344,   484, -1528,   413, -1528,    73, -1528,   612,
   -1528, -1528, -1528, -1528, 18505,   626, -1528, -1528,   429,   459,
     464,   476,   478,   483,  4806, -1528, -1528, -1528, -1528,    71,
   -1528,   583,   621,   488, -1528,    84,   492, -1528,   512,   -16,
   -1528,  2385,   140, -1528, -1528,  1794,    60,   506,   138, -1528,
      76,    77,   526,   142, -1528, -1528,   670, -1528, -1528, -1528,
     589,   552,   592, -1528, 13751, -1528,   612,   626, 18819,  3516,
   18819, 13751, 18819, 18819, 15035,   558, 18004, 15035,   709, 14698,
     696,   696,   343,   696,   696,   696,   696,   696,   696,   696,
     696,   696, -1528, -1528, -1528,    85, 13751,   608, -1528, -1528,
     631,   572,   277,   588,   277, 17565, 18049,   595,   787, -1528,
     589, 13751,   608,   661, -1528,   673,   625, -1528,   129, -1528,
   -1528, -1528,   277,    60, 12509, -1528, -1528, 13751,  9197,   813,
      86, 18819, 10232, -1528, 13751, 13751, 14698, -1528, -1528,  5071,
     624, -1528,  5128, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, 16332, -1528, 16332, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528,    89,    90,   592, -1528, -1528, -1528, -1528,   627,  4373,
      92, -1528, -1528,   677,   819, -1528,   687, 15623,   761, -1528,
     640,  5173,   641,   600, -1528,    25, 16077, 18522, 18587, 14698,
      93, -1528,   266, -1528, 17040,   100, -1528,   721, -1528,   722,
   -1528,   835,   104, 17565, 13751, 13751,   652,   692, -1528, -1528,
   17171, 11681,   105,   471,   489, -1528, 13958, 17565,   505, -1528,
   14698, -1528,   473,   344, -1528, -1528, -1528, -1528, 17817,   851,
     768, -1528, -1528, -1528,    72, 13751,   669,   672, 18819,   674,
    2038,   676,  5885, 13751, -1528,   443,   665,   565,   443,   440,
      87, -1528, 14698, 16332,   678, 10646, 14841, -1528, -1528,  2988,
   -1528, -1528, -1528, -1528, -1528,   612, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, 13751, 13751, 13751, 12716, 13751, 13751, 13751,
   13751, 13751, 13751, 13751, 13751, 13751, 13751, 13751, 13751, 13751,
   13751, 13751, 13751, 13751, 13751, 13751, 13751, 13751, 13751, 17691,
   13751, -1528, 13751, 13751, -1528, 13751, 14165, 14698, 14698, 14698,
   14698, 14698, 18505,   767,   699,  4862, 13751, 13751, 13751, 13751,
   13751, 13751, 13751, 13751, 13751, 13751, 13751, 13751, -1528, -1528,
   -1528, -1528, 16405, 13751, 13751, -1528, 10646, 10646, 13751, 13751,
   17171,   680,   612, 12923, 16122, -1528, 13751, -1528,   682,   874,
     736,   684,   686, 14308,   277, 13130, -1528, 13337, -1528,   690,
     691,  3094, -1528,   187, 10646, -1528, 16845, -1528, -1528, 16167,
   -1528, -1528, 10853, -1528, 13751, -1528,   803,  9404,   885,   693,
   18774,   883,    98,    53, -1528, -1528, -1528,   726, -1528, -1528,
   -1528, 16332,  3904,   700,   890, 16780, 14698, -1528, -1528, -1528,
   -1528, -1528,   717, -1528, -1528, -1528,   824, 13751,   826,   827,
   13751, 13751, 13751, -1528,   600, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528,   720, -1528, -1528, -1528,   711, -1528, -1528, 14698,
     710,   904,   314, 14698,   712,   906,   418,   442, 18604, -1528,
   14698, 13751,   277,   167, -1528, -1528, -1528, 16780,   837, -1528,
     277,   103,   114,   718,   719,  3952,   168,   723,   731,   379,
     794,   725,   277,   121,   738, -1528, 17614, 14698, -1528, -1528,
     870,  2736,   378, -1528, -1528, -1528,   344, -1528, -1528, -1528,
     913,   815,   774,   217,   796, 13751,   816,   941,   752,   818,
   -1528,   141, -1528, 16332, 16332,   940,   813,    72, -1528,   773,
     966, -1528, 16332,    29, -1528,    83,   152, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528,   741,  2881, -1528, -1528, -1528, -1528,
     967,   806, -1528, 17565, 13751,   779,   972, 18819,   969, -1528,
   -1528,   856, 14503, 12080,  3819, 15035, 13751, 13943, 18978, 17107,
    4182, 10625,  4138, 11659, 11659, 11659, 11659,   916,   916,   916,
     916,   916,  1073,  1073,   668,   668,   668,   343,   343,   343,
   -1528,   696, 18819,   778,   781, 18106,   790,   989,   213, 13751,
     214,   608,    24, -1528, -1528, -1528,   986,   768, -1528,   612,
   17303, -1528, -1528, 15035, -1528, 15035, 15035, 15035, 15035, 15035,
   15035, 15035, 15035, 15035, 15035, 15035, 15035, -1528, 13751,   330,
   -1528,   143, -1528,   608,   353,   798,  2944,   809,   820,   800,
    3207,   122,   817, -1528, 18819, 16910, -1528, 14698, -1528,    29,
      22, 17565, 18819, 17565, 18151,    29,   277,   145, -1528,   141,
     869,   823, 13751, -1528,   153, -1528, -1528, -1528,  8990,   551,
   -1528, -1528, 18819, 18819,    38, -1528, -1528, -1528, 13751,   927,
   16612, 16780, 14698,  9611,   829,   830, -1528,    75,   939,   897,
     887, -1528,  1025,   838, 16211, 16332, 16780, 16780, 16780, 16780,
   16780,   836,   894,   846, 16780,    13, -1528,   896, -1528,   847,
   -1528, 18907, -1528,    17, -1528, 13751,   863, 18819,   867,  1040,
   11459,  1046, -1528, 18819, 16212, -1528,   720,   978, -1528,  6092,
   18464,   858,   448, -1528, 18522, 14698,   479, -1528, 18587, 14698,
   14698, -1528, -1528,  3567, -1528, 18907,  1045, 17565,   862, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,    80, 14698,
   18464,   864, 17171, 17434,  1051, -1528, -1528, -1528, -1528,   861,
   -1528, 13751, -1528, -1528,  5264, -1528, 16332, 18464,   872, -1528,
   -1528, -1528, -1528,  1065,   880, 13751, 17817, -1528, -1528, 14165,
     882, -1528, 16332, -1528,   891,  6299,  1060,   130, -1528, -1528,
     182, 16405, -1528, 16845, -1528, 16332, -1528, -1528,   277, 18819,
   -1528, 11060, -1528, 16780,   119,   892, 18464,   815, -1528, -1528,
   18689, 13751, -1528, -1528, 13751, -1528, 13751, -1528,  3673,   893,
   10646,   794,  1061,   815, 16332,  1081,   856, 14698, 17691,   277,
    4003,   898, -1528, -1528,   158,   899, -1528, -1528,  1086, 16832,
   16832, 16910, -1528, -1528, -1528,  1053,   905,    97,   907, -1528,
   -1528, -1528, -1528,  1099,   908,   682,   277,   277, 13544, 16845,
   -1528, -1528,  4283,   610,    38, 10232, -1528,  6506,   909,  6713,
     910, 16612, 17565,   914,   983,   277, 18907,  1105, -1528, -1528,
   -1528, -1528,    54, -1528,   291, 16332, -1528,   994, 16332, 14698,
    3904, -1528, -1528, -1528,  1120, -1528,   929,   967,   707,   707,
    1068,  1068, 18253,   924,  1126, 16780, 15909, 17817,  2420, 15766,
   16780, 16780, 16780, 16780, 16482, 16780, 16780, 16780, 16780, 16780,
   16780, 16780, 16780, 16780, 16780, 16780, 16780, 16780, 16780, 16780,
   16780, 16780, 16780, 16780, 16780, 16780, 16780, 16780, 14698, -1528,
   18819, 13751, 13751, 13751, -1528, -1528, -1528, 13751, 13751, -1528,
     600, -1528,  1056, -1528, -1528, 14698, -1528, -1528, 14698, -1528,
   -1528, -1528, -1528, 16780,   277, -1528,   379, -1528,   651,  1131,
   -1528, -1528,   123,   937,   277, 11267, -1528,  2341, -1528,  5471,
     768,  1131, -1528,   221,   -11, -1528, 18819,  1010,   945, -1528,
     948,  1060, -1528, 16332,   813, 16332,    65,  1133,  1071,   162,
   -1528,   608,   164, -1528, -1528, 17565, 13751, 18819, 18907,   956,
     119, -1528,   955,   119,   959, 18689, 18819, 18208,   960, 10646,
     961,   958, 16332,   964,   962, 16332,   815, -1528,   625,   408,
   10646, 13751, -1528, -1528, -1528, -1528, -1528, -1528,  1031,   963,
    1159,  1084, 16910,  1028, -1528, 17817, 16910, -1528, -1528, -1528,
   17565, 18819, -1528,    38,  1143,  1108, 10232, -1528, -1528, -1528,
     982, 13751,   983,   277, 17171, 16612,   985, 16780,  6920,   299,
     987, 13751,    55,   293, -1528,  1017, 16332, -1528,  1059, -1528,
   16257,  1162,   996, 16780, -1528, 16780, -1528,   997, -1528,  1067,
    1192,  1001, -1528, -1528, -1528, 18310,   999,  1195, 14162, 15620,
   15230, 16780, 18864, 19013, 17370,  4738, 11039, 12901, 13108, 13108,
   13108, 13108,  1569,  1569,  1569,  1569,  1569,  1022,  1022,   707,
     707,   707,  1068,  1068,  1068,  1068, -1528, 18819, 13736, 18819,
   -1528, 18819, -1528,  1004, -1528, -1528, -1528, 18907, 14698, 16332,
   16332, -1528, 18464,    82, -1528, 17171, -1528, -1528, 15035,  1003,
   -1528,  1006,   499, -1528,    88, 13751, -1528, -1528, -1528, 13751,
   -1528, 13751, -1528,   813, -1528, -1528,   186,  1196,  1132, 13751,
   -1528,  1011,   277, 18819,  1060,  1012, -1528,  1013,   119, 13751,
   10646,  1018, -1528, -1528,   768, -1528, -1528,  1021,  1023,  1020,
   -1528,  1024, 16910, -1528, 16910, -1528, -1528,  1027, -1528,  1092,
    1029,  1222, -1528,   277,  1202, -1528,  1032, -1528, -1528,  1039,
    1054,   124, -1528, -1528, 18907,  1050,  1052, -1528,  3393, -1528,
   -1528, -1528, -1528, -1528, -1528, 16332, -1528, 16332, -1528, 18907,
   18355, -1528, 16780, 17817, -1528, -1528, 16780, -1528, 16780, -1528,
   18943, 16780, 13751,  1057,  7127,   651, -1528, -1528, -1528,   570,
   15036, 18464,  1134, -1528, 16710,  1097, 18403, -1528, -1528, -1528,
     767,  2148,   106,   107,  1062,   768,   699,   125, -1528, -1528,
   -1528,  1103,  4334,  4657, 18819, -1528,   254,  1247,  1182, 13751,
   -1528, 18819, 10646,  1150,  1060,   648,  1060,  1064, 18819,  1066,
   -1528,  1128,  1063,  1337, -1528, -1528,   119, -1528, -1528,  1137,
   -1528, 16910, -1528, 17817, -1528, -1528,  8990, -1528, -1528, -1528,
   -1528,  9818, -1528, -1528, -1528,  8990, -1528,  1069, 16780, 18907,
    1139, 18907, 18412, 18943, 12494, -1528, -1528, -1528, 18464, 18464,
   14698, -1528,  1260, 15909,    79, -1528, 15036,   768, 18450, -1528,
    1104, -1528,   108,  1072,   110, -1528, 15429, -1528, -1528, -1528,
     112, -1528, -1528, 18330, -1528,  1075, -1528,  1198,   612, -1528,
   15231, -1528, 15231, -1528, -1528,  1265,   767, -1528, 14451, -1528,
   -1528, -1528, -1528,  1266,  1201, 13751, -1528, 18819,  1082,  1088,
    1079,   568, -1528,  1150,  1060, -1528, -1528, -1528, -1528,  1586,
    1083, 16910, -1528,  1160,  8990, 10025,  9818, -1528, -1528, -1528,
    8990, -1528, 18907, 16780, 16780, 13751,  7334,  1089,  1091, -1528,
   16780, -1528, 18464, -1528, -1528, -1528, -1528, -1528, 16332,  1041,
   16710, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528,   675, -1528,  1097, -1528, -1528, -1528, -1528, -1528,
      96,   596, -1528,  1283,   115, 15623,  1198,  1284, -1528, 16332,
     612, -1528, -1528,  1098,  1285, 13751, -1528, 18819, -1528,   126,
   -1528, -1528, -1528, -1528,  1100,   568, 14646, -1528,  1060, -1528,
   16910, -1528, -1528, -1528, -1528,  7541, 18907, 18907, 11873, -1528,
   -1528, -1528, 18907, -1528, 15300,   101,  1292,  1101, -1528, -1528,
   16780, 15429, 15429,  1246, -1528, 18330, 18330,   663, -1528, -1528,
   -1528, 16780,  1226, -1528,  1136,  1109,   116, 16780, -1528, 14698,
   -1528, 16780, 18819,  1231, -1528,  1303,  7748,  7955, -1528, -1528,
   -1528,   568, -1528,  8162,  1111,  1236,  1206, -1528,  1220,  1169,
   -1528, -1528,  1225, 16332, -1528,  1041, -1528, -1528, 18907, -1528,
   -1528,  1161, -1528,  1289, -1528, -1528, -1528, -1528, 18907,  1312,
     379, -1528, -1528, 18907,  1127, 18907, -1528,   381,  1130, -1528,
   -1528,  8369, -1528,  1135, -1528, -1528,  1140,  1164, 14698,   699,
    1155, -1528, -1528, 16780,   128,   132, -1528,  1256, -1528, -1528,
   -1528, -1528, 18464,   858, -1528,  1173, 14698,   553, -1528, 18907,
   -1528,  1141,  1334,   614,   132, -1528,  1264, -1528, 18464,  1145,
   -1528,  1060,   136, -1528, -1528, -1528, -1528, 16332, -1528,  1147,
    1149,   117, -1528,   573,   614,   300,  1060,  1151, -1528, -1528,
   -1528, -1528, 16332,   294,  1338,  1273,   573, -1528,  8576,   301,
    1342,  1278, 13751, -1528, -1528,  8783, -1528,   331,  1345,  1281,
   13751, -1528, 18819, -1528,  1349,  1286, 13751, -1528, 18819, 13751,
   -1528, 18819, 18819
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1528, -1528, -1528,  -578, -1528, -1528, -1528,   460,     2,   -32,
   -1528, -1528, -1528,   769,   501,   500,   188,  1663,  1593, -1528,
    2739, -1528,  -443, -1528,    37, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528, -1528,  -422, -1528, -1528,  -160,
     176,    30, -1528, -1528, -1528, -1528, -1528, -1528,    34, -1528,
   -1528, -1528, -1528,    36, -1528, -1528,   900,   915,   911,  -115,
     407,  -866,   414,   467,  -427,   190,  -932, -1528,  -148, -1528,
   -1528, -1528, -1528,  -745,    18, -1528, -1528, -1528, -1528,  -414,
   -1528,  -590, -1528,  -432, -1528, -1528,   786, -1528,  -132, -1528,
   -1528, -1065, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528, -1528,  -164, -1528,   -81, -1528, -1528, -1528, -1528, -1528,
    -246, -1528,     3,  -927, -1528, -1527,  -436, -1528,  -153,   137,
    -133,  -423, -1528,  -254, -1528, -1528, -1528,    14,   -44,     0,
      11,  -715,   -92, -1528, -1528,   -19, -1528, -1528,    -5,   -62,
    -120, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
    -601,  -856, -1528, -1528, -1528, -1528, -1528,  1224, -1528, -1528,
   -1528, -1528,   930, -1528, -1528,   317, -1528,   834, -1528, -1528,
   -1528, -1528, -1528, -1528, -1528,   324, -1528,   839, -1528, -1528,
     556, -1528,   292, -1528, -1528, -1528, -1528, -1528, -1528, -1528,
   -1528,  -909, -1528,  2452,    41, -1528,   323,  -396, -1528, -1528,
     248,  3530,  3584, -1528, -1528,   377,  -193,  -659, -1528, -1528,
     444,   242,  -702,   243, -1528, -1528, -1528, -1528, -1528,   431,
   -1528, -1528, -1528,    68,  -896,  -180,  -426,  -415, -1528,   497,
    -123, -1528, -1528,    45,    46,   638, -1528, -1528,   992,   -35,
   -1528,  -340,    31,  -346,    58,   211, -1528, -1528,  -472,  1076,
   -1528, -1528, -1528, -1528, -1528,   425,   427, -1528, -1528, -1528,
    -330,  -657, -1528,  1014, -1077, -1528,   -66,  -199,   -26,   607,
   -1528, -1037,    47,  -327,   328,   405, -1528, -1528, -1528, -1528,
     359,   756, -1096
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1013
static const yytype_int16 yytable[] =
{
     188,   190,   426,   192,   193,   194,   196,   197,   473,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   397,   332,   226,   229,   386,   501,   942,   771,   389,
     390,  1135,   248,   522,   130,   518,   340,   251,   132,  1306,
     133,   128,   651,   495,   253,   259,   472,   262,   650,   257,
     341,   399,   346,   250,   426,   923,   331,  1292,   652,   904,
    1127,   241,   526,   720,   767,   768,   760,   924,   422,   396,
    1024,   234,   161,   992,   251,   239,   240,   761,   401,  1202,
     965,  1152,   416,   814,  1025,  1359,  1389,   798,  1552,  1106,
     372,  -876,   791,  1010,  1303,   534,   493,  1163,   -78,   -43,
     794,   -42,   598,   -78,   -43,  1705,   -42,   812,   398,   603,
    1743,   795,   877,   608,   534,  1500,  1502,  -356,    13,  1560,
     527,  1644,   399,   534,  1712,  1712,  1552,   493,    13,  1706,
     893,   893,   893,   893,   893,  -699,   586,  1106,  -556,  1411,
     396,   135,   859,  -869,  -870,  1307,  1298,   361,  1729,   401,
    -560,  -868,   490,   491,   512,  1210,  1211,  1212,   359,   490,
     491,  1723,   191,   357,  -985,    13,   360,   373,  1078,   520,
     358,   510,   511,   633,   404,  -707,    13,   789,    13,   398,
     129,   401,    13,  1183,  -881,   413,   504,   519,   244,   413,
    1390,  1229,  1299,   627,   587,  1391,  -910,    60,    61,    62,
     178,  1392,   423,  1393,  1772,  -558,  1724,  -871,   498,   498,
    -872,   398,  -913,   490,   491,   925,  -985,  1136,  -700,  -875,
    -912,  1416,  -883,     3,   960,  -874,   398,  -876,  -706,  -855,
    1308,  -856,   494,   375,  -702,  -558,   376,   377,   247,   529,
    1394,  1395,   529,  1396,   387,  1184,   352,   353,  1026,   251,
     540,  1425,  -526,  1107,   212,   344,   815,  1360,  1431,   391,
    1433,   551,  1137,   494,   424,   692,  1417,   212,   910,   382,
    -295,  -880,   383,  1352,  1744,   417,  1209,  1423,  1213,  -869,
    -870,  1553,  1554,  -883,  -879,   531,  1397,  -868,   535,   536,
     387,   -78,   -43,  -701,   -42,   599,   678,  1707,   474,   813,
    1328,  1800,   604,   614,   878,   561,   609,   625,  1501,  1503,
    -356,   425,  1561,  1472,  1645,   879,   778,  1713,  1762,  1828,
    -295,   613,   894,   978,  1284,  1450,  1507,  1119,  -788,  1339,
     617,  -788,  -910,  -279,  1513,  1833,  1847,  -788,   497,   331,
    -878,   426,   772,  -871,   499,   499,  -872,  1138,  -913,   220,
     220,  1418,  -882,   690,  1149,  -875,  -912,  1007,  -885,   251,
     398,  -874,  1009,   502,   600,  -855,   226,  -856,   497,   883,
    1529,   631,   354,   911,  1840,   612,   392,   403,  1293,   116,
    1834,  1848,   393,   741,   332,  -105,  -104,  1521,   912,  1523,
     196,  1294,   116,  1211,  1212,  1211,  1212,   459,   675,  1555,
    1356,  1211,  1212,   386,   736,   737,   422,   490,   491,   460,
     687,  1854,   865,  -105,  -104,  -559,  1787,   411,   331,  1514,
    1112,   412,   254,  1657,  1295,  1658,   886,   887,   693,   694,
     695,   697,   698,   699,   700,   701,   702,   703,   704,   705,
     706,   707,   708,   709,   710,   711,   712,   713,   714,   715,
     716,   717,   718,   719,   917,   721,   742,   722,   722,  1841,
     725,  1788,   661,  1291,   121,  1835,  1849,   255,   730,   412,
     743,   745,   746,   747,   748,   749,   750,   751,   752,   753,
     754,   755,   756,   931,  1342,   933,  1349,  1675,   722,   766,
     241,   687,   687,   722,   770,  1214,  1855,  1362,   743,   412,
     234,   774,  -843,  1389,   239,   240,   256,  1142,   490,   491,
     782,   260,   784,   801,   330,   331,   869,  1460,  1143,   687,
     373,   473,   363,   373,   732,  -846,   633,   802,  1160,   803,
    -843,   367,   959,  1305,   957,   592,   596,   597,   626,   135,
     870,   373,   407,   409,   410,    13,  1095,   374,   220,   384,
     764,   367,   373,  -846,   788,   367,   367,   364,   405,   472,
    1315,   365,   847,  1317,   971,   850,   853,   854,   636,   373,
     651,   732,  1168,   412,   806,  1208,   650,  1098,   129,   967,
    -844,   790,   903,   914,   796,   373,   652,  1533,   366,   376,
     377,   633,   376,   377,   367,   370,   873,   412,   371,   931,
     933,   490,   491,   412,   388,   999,   933,  1390,  -844,   375,
     376,   377,  1391,   415,    60,    61,    62,   178,  1392,   423,
    1393,   376,   377,  1004,  1005,  1708,   949,   373,   412,  1031,
    1438,   427,  1439,   408,   412,  -985,   522,   628,   376,   377,
     398,  1731,   165,  1709,  1079,   373,  1710,   418,   677,   739,
     462,   633,  1389,   634,   376,   377,   413,  1394,  1395,   508,
    1396,   428,  1000,   466,   222,   224,   429,  1432,    60,    61,
      62,   178,   179,   423,   724,   638,   639,   220,   430,   939,
     431,   424,  1193,  1194,  -985,   432,   220,  -985,   463,  1700,
     464,   950,  1755,   220,    13,   465,   376,   377,  1468,  1469,
     220,   651,  1415,  1410,   765,  1701,   496,   650,   121,   769,
    1756,   649,   121,  1757,   376,   377,   541,   652,   456,   457,
     458,  1825,   459,  1702,   958,   661,  -877,  1321,  1427,  1814,
    1815,  1816,  1351,    53,   460,   424,  1839,  -557,  1331,  1532,
    -700,    60,    61,    62,   178,   179,   423,   380,  1505,  1279,
    1280,   400,   500,   970,   507,  1810,  1390,  1074,  1075,  1076,
     505,  1391,   460,    60,    61,    62,   178,  1392,   423,  1393,
    1671,  1672,  -881,  1077,  1823,  1829,  1830,   862,  1753,  1754,
     413,   866,   513,   566,   567,   568,   352,  1002,   497,  1836,
     569,   570,  1749,  1750,   571,   572,   517,   560,  1034,  1037,
    1187,   629,   516,   251,  1008,   635,  1394,  1395,   424,  1396,
     337,   474,  -698,  1407,    60,    61,    62,   178,   179,   423,
    1556,   532,   400,   220,   523,   524,   545,   552, -1012,  1679,
     424,  1384,   629,   555,   635,   629,   635,   635,   556,   651,
    1080,   562,   563,   565,  1222,   650,  1530,   605,   606,   607,
    1019,  1226,  1522,   618,   400,   652,    60,    61,    62,    63,
      64,   423,   121,   514,   619,   653,   654,    70,   467,   521,
     663,   676,   679,   664,    53,   665,   330,   667,  -130,   367,
     689,   424,   775,   777,   628,   779,   165,   780,  1429,  1803,
     165,   785,   786,   804,   534,   808,  1117,   811,   551,   825,
     824,  1167,  1446,   845,   846,   469,   848,   849,  1803,   857,
    1126,   860,   863,   864,   867,   868,  1824,   876,  1455,   880,
     881,   730,   890,   424,   884,   892,   560,   367,   734,   367,
     367,   367,   367,   885,   130,   135,  1147,   901,   132,   895,
     133,   128,  1311,   906,   907,   909,  1155,  -722,   915,  1156,
     916,  1157,   759,   918,   922,   687, -1013, -1013, -1013, -1013,
   -1013,   451,   452,   453,   454,   455,   456,   457,   458,   919,
     459,   926,   161,   560,   129,   927,   935,  1128,  1732,   937,
     940,   941,   460,   943,   946,   952,   793,  1335,   953,   764,
    1518,   796,   602,  1191,   955,   661,   135,   121,   956,   241,
     964,   610,  1534,   615,  1196,   972,   220,   976,   622,   234,
     974,  1540,   661,   239,   240,   632,   843,   948,   223,   223,
    -704,   975,  1546,  1286,  1001,  1011,  1027,   651,   549,  1028,
     550,  1021,  1023,   650,  1030,   129,  1045,  1374,  1029,  1032,
     165,   135,  1197,   652,  1379,  1046,  1047,  1049,  1228,  1081,
    1050,  1234,   592,  1082,  1083,  1087,   596,   796,  1090,  1103,
     872,  1093,   135,   220,  1105,  1115,  1768,  1111,  1116,  1071,
    1072,  1073,  1074,  1075,  1076,  1122,  1267,  1268,  1269,  1124,
     129,  1125,   850,  1271,  1129,   554,   898,   900,  1077,  1131,
    1133,  1162,  1150,  1159,  1685,  1165,  1287,   651,  1170,  -884,
    1171,   129,  1181,   650,   220,  1182,   220,  1185,  1186,  1188,
    1288,  1199,  1201,   652,  1204,  1205,    33,    34,    35,  1207,
     453,   454,   455,   456,   457,   458,  1216,   459,   213,  1220,
    1221,  1224,  1389,   220,  1077,  1225,  1273,  1285,   622,   460,
    1282,  1313,  1301,  1813,   135,   960,   135,  1309,  1444,   130,
    1302,  1310,   367,   132,   687,   133,   128,  1314,  1316,  1318,
    1320,  1323,  1322,  1332,  1326,   687,  1288,  1325,  1334,   682,
    1333,   985,   337,  1345,    13,   165,  1338,    74,    75,    76,
      77,    78,  1346,   129,  1348,   129,  1353,   161,   215,  1357,
    1363,  1365,  1367,  1344,    82,    83,   251,  1368,  1371,  1372,
     220,  1373,  1375,  1377,  1378,  1383,  1358,  1408,    92,  1409,
    1419,  1422,  1420,  1424,  1426,   220,   220,   223,   661,  1430,
    1436,   661,    97,  1434,  1441,  1437,  1435,  1726,  1440,  1727,
    1442,  1443,  1445,  1347,  1447,   989,  1390,   995,  1733,   649,
    1448,  1391,  1476,    60,    61,    62,   178,  1392,   423,  1393,
     218,   218,  1452,  1506,  1453,  1449,   135,  1489,   121,  1509,
    1465,  1515,  1516,  1519,  1504,  1524,  1527,  1525,   426,  1531,
    1541,  1543,  1017,   121,  1550,  1559,  1558,  1652,  1653,  1659,
    1665,  1666,  1670,  1668,  1678,  1771,  1394,  1395,  1669,  1396,
    1412,  1690,  1680,  1691,  1413,   129,  1414,  1711,  1717,  1721,
    1720,  1745,  1751,  1728,  1421,  1747,  1759,   817,  1760,  1761,
     424,  1766,  1767,  1774,  1428,   687,  1775,  1776,  -352,   121,
    1778,   938,  1706,  1779,  1782,  1097,  1783,  1786,  1798,  1100,
    1101,  1789,  1526,  1793,   220,   220,  1805,  1791,  1655,  1808,
    1792,  1389,  1811,  1812,  1820,   135,   223,  1822,  1826,  1108,
    1827,  1406,  1842,  1843,  1837,   223,  1850,   616,  1851,  1856,
    1406,  1857,   223,  1859,   121,  1096,  1860,   871,  1099,   223,
     649,  1807,   738,  1166,  1121,  1161,  1454,  1464,   969,   560,
     735,  1821,   733,    13,   129,   121,   661,  1838,  1684,   874,
    1819,   759,  1350,   793,  1845,  1676,  1699,  1557,  1704,  1496,
    1844,  1832,  1716,  1674,  1549,   744,  1477,  1272,   855,   920,
     921,  1270,  1089,   856,  1517,  1296,  1330,   687,   929,   996,
    1401,   997,  1227,  1336,  1177,  1337,  1189,   367,  1141,  1401,
     688,  1035,  1467,  1784,  1278,  1219,   165,  1266,     0,  1175,
    1175,   989,     0,     0,     0,  1390,     0,   624,  1015,   218,
    1391,   165,    60,    61,    62,   178,  1392,   423,  1393,   793,
       0,     0,     0,     0,     0,   121,     0,   121,   220,   121,
       0,     0,     0,     0,     0,  1551,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1538,  1218,
       0,     0,   223,  1406,     0,  1394,  1395,   165,  1396,  1406,
    1719,  1406,     0,  1746,   661,     0,   560,     0,   649,   560,
    1667,     0,     0,   220,     0,  1104,     0,     0,     0,   424,
       0,   135,     0,     0,     0,     0,     0,   220,   220,     0,
     622,  1114,     0,     0,     0,     0,     0,     0,   843,     0,
    1688,  1528,   165,   474,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1275,     0,     0,  1276,     0,
     129,     0,  1401,   165,     0,     0,     0,     0,  1401,     0,
    1401,     0,  1682,  1538,     0,     0,  1498,     0,   218,   121,
     682,   682,     0,   135,     0,     0,     0,   218,     0,     0,
    1389,     0,   135,     0,   218,     0,     0,  1406,     0,     0,
       0,   218,     0,     0,     0,     0,     0,     0,   220, -1013,
   -1013, -1013, -1013, -1013,  1069,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,   129,  1714,     0,     0,  1796,     0,     0,     0,
       0,   129,    13,     0,     0,  1077,     0,     0,     0,     0,
       0,     0,   989,   165,     0,   165,   989,   165,     0,  1015,
    1203,     0,     0,  1764,     0,     0,   121,   331,     0,     0,
    1722,     0,  1120,     0,   426,     0,  1401,     0,   121,     0,
       0,   135,     0,     0,     0,   223,     0,   135,  1130,     0,
       0,     0,  1660,   135,     0,     0,     0,     0,     0,   217,
     217,  1144,     0,   232,  1390,     0,   649,     0,     0,  1391,
       0,    60,    61,    62,   178,  1392,   423,  1393,     0,     0,
     129,     0,     0,     0,     0,     0,   129,   232,     0,     0,
    1164,     0,   129,     0,   218,     0,     0,     0,     0,     0,
       0,     0,   223,     0,     0,     0,     0,     0,  1385,     0,
       0,     0,     0,     0,  1394,  1395,     0,  1396,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   165,     0,     0,
       0,     0,     0,     0,     0,     0,   649,     0,   424,     0,
       0,     0,     0,   223,     0,   223,     0,     0,     0,     0,
       0,  1215,     0,  1312,  1217,     0,     0,     0,     0,     0,
    1677,     0,   989,     0,   989,     0,     0,     0,     0,     0,
       0,     0,   223,     0,     0,     0,     0,     0,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,     0,   135,     0,     0,     0,     0,     0,  1343,     0,
       0,     0,     0,     0,   165,     0,     0,  1852,     0,     0,
       0,     0,   622,  1015,   121,  1858,   165,     0,     0,     0,
     330,  1861,     0,   661,  1862,     0,  1494,     0,   488,   489,
       0,   129,     0,   135,   135,     0,     0,     0,     0,   223,
     135,     0,   661,     0,     0,     0,     0,     0,     0,     0,
     661,     0,     0,     0,   223,   223,     0,     0,   217,  1304,
       0,   929,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   989,   129,   129,     0,     0,   121,   218,   135,   129,
       0,   121,     0,     0,     0,   121,  1797,     0,  1324,     0,
       0,  1327,     0,   622,   490,   491,     0,     0,     0,     0,
     367,     0,     0,   560,     0,   232,   330,   232,     0,     0,
       0,     0,     0,     0,     0,     0,  1641,   129,     0,     0,
       0,     0,     0,  1648,     0,     0,     0,     0,     0,     0,
     330,     0,   330,     0,   218,     0,     0,     0,   330,     0,
       0,     0,  1364,     0,     0,   135,  1144,     0,     0,     0,
       0,     0,   135,     0,     0,     0,     0,     0,     0,     0,
       0,   989,   232,     0,   121,   121,   121,     0,     0,     0,
     121,     0,     0,   223,   223,   218,   121,   218,     0,     0,
       0,     0,     0,     0,   129,     0,     0,   217,     0,     0,
       0,   129,   165,     0,     0,     0,   217,     0,     0,     0,
       0,     0,     0,   217,   218,  1386,  1387,     0,     0,     0,
     217,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   232,   503,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   232,     0,     0,   232,
       0,     0,     0,     0,   165,     0,     0,     0,     0,   165,
       0,     0,     0,   165,     0,     0,     0,     0,     0,     0,
       0,   218,   488,   489,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   560,   218,   218,     0,     0,
       0,  1456,   232,  1457,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   330,   223,     0,     0,
     989,     0,     0,     0,     0,   121,     0,     0,   836,     0,
       0,     0,     0,     0,  1738,     0,     0,  1499,     0,     0,
       0,  1641,  1641,   217,     0,  1648,  1648,     0,   490,   491,
       0,     0,   165,   165,   165,     0,     0,   272,   165,   367,
       0,     0,   223,     0,   165,     0,   121,   121,     0,     0,
       0,     0,     0,   121,     0,     0,   223,   223,     0,     0,
     836,     0,     0,     0,   274,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   232,   232,     0,     0,   835,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,   121,     0,     0,     0,   218,   218,     0,  1795,   666,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,  -399,  1809,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   178,   179,   423,     0,
     835,     0,     0,     0,     0,     0,     0,   223,     0,     0,
       0,     0,   547,   548,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   121,     0,
     182,     0,     0,    84,   324,   121,    86,    87,     0,    88,
     183,    90,     0,     0,  1695,     0,   232,   232,     0,     0,
       0,     0,     0,   165,   328,   232,     0,     0,     0,     0,
     424,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   217,     0,   329,     0,
       0,   433,   434,   435,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   165,   165,     0,     0,     0,   218,
     436,   165,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,     0,   459,     0,     0,     0,     0,
       0,     0,     0,   217,   836,     0,     0,   460,     0,   165,
       0,     0,     0,     0,   218,  1718,     0,     0,     0,   836,
     836,   836,   836,   836,     0,     0,     0,   836,   218,   218,
    1051,  1052,  1053,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   217,     0,   217,     0,     0,  1054,
       0,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,   217,   835,     0,   165,     0,   219,   219,
       0,     0,   233,   165,     0,     0,  1077,   232,   232,   835,
     835,   835,   835,   835,     0,     0,     0,   835,     0,  1780,
      60,    61,    62,    63,    64,   423,     0,     0,     0,   218,
       0,    70,   467,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   232,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   836,     0,     0,     0,
     217,     0,     0,     0,     0,     0,     0,   468,  1289,   469,
       0,     0,     0,   232,     0,   217,   217,     0,     0,     0,
       0,     0,   470,     0,   471,     0,     0,   424,     0,   232,
     232,     0,     0,   929,     0,     0,     0,     0,     0,   232,
       0,     0,     0,     0,     0,   232,     0,     0,   929,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   232,     0,
       0,     0,     0,     0,     0,     0,   835,     0,     0,   232,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1232,     0,     0,     0,     0,     0,   232,     0,     0,
       0,   232,     0,     0,     0,     0,     0,     0,   836,     0,
       0,     0,     0,   836,   836,   836,   836,   836,   836,   836,
     836,   836,   836,   836,   836,   836,   836,   836,   836,   836,
     836,   836,   836,   836,   836,   836,   836,   836,   836,   836,
     836,     0,     0,     0,   217,   217,     0,   219,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   232,     0,
       0,   232,     0,   232,     0,     0,   836,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   835,     0,
     232,     0,     0,   835,   835,   835,   835,   835,   835,   835,
     835,   835,   835,   835,   835,   835,   835,   835,   835,   835,
     835,   835,   835,   835,   835,   835,   835,   835,   835,   835,
     835,     0,     0,     0,     0,     0,   433,   434,   435,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   436,   835,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,     0,
     459,     0,     0,   333,     0,     0,   232,     0,   232,     0,
     836,     0,   460,     0,     0,     0,   219,     0,   217,     0,
       0,     0,     0,     0,     0,   219,   836,     0,   836,     0,
       0,     0,   219,     0,     0,   232,     0,     0,   232,   219,
       0,     0,     0,     0,   836,     0,     0,     0,     0,     0,
     219,     0,     0,     0,     0,     0,     0,     0,   232,     0,
       0,     0,     0,   217,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   217,   217,     0,
     835,     0,     0,     0,     0,     0,     0,     0,     0,   232,
       0,     0,     0,   232,     0,     0,   835,     0,   835,     0,
       0,   433,   434,   435,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   835,     0,     0,     0,     0,     0,
     436,   233,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,     0,   459,     0,     0,     0,     0,
     902,     0,   232,   232,     0,   232,     0,   460,   217,     0,
       0,     0,   219,     0,   433,   434,   435,     0,     0,     0,
       0,     0,     0,     0,     0,   836,     0,     0,     0,   836,
       0,   836,     0,   436,   836,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,     0,
       0,     0,     0,     0,     0,     0,     0,   839,     0,     0,
     460,   333,     0,   333,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   232,     0,
     232,     0,     0,     0,     0,   835,   232,     0,     0,   835,
       0,   835,     0,     0,   835,     0,     0,     0,     0,     0,
       0,   836,     0,   232,   232,     0,     0,   232,     0,   839,
       0,     0,     0,     0,   232,     0,    36,     0,   333,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   934,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   232,     0,   503,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   835,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   232,   232,     0,     0,   219,   836,   836,     0,   232,
       0,   232,     0,   836,     0,     0,    86,    87,   973,    88,
     183,    90,   333,     0,     0,   333,     0,     0,   488,   489,
       0,     0,     0,   232,     0,   232,     0,     0,     0,     0,
       0,   232,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   689,     0,
       0,     0,   219,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   835,   835,     0,     0,
       0,     0,     0,   835,     0,   232,     0,   433,   434,   435,
       0,   232,     0,   232,   490,   491,     0,     0,     0,     0,
       0,     0,     0,   219,     0,   219,   436,     0,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
       0,   459,   219,   839,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   460,     0,     0,     0,     0,   839,   839,
     839,   839,   839,     0,     0,     0,   839,     0,     0,     0,
     333,   820,     0,   836,   837,   787,     0,     0,     0,     0,
       0,     0,     0,     0,   836,     0,     0,     0,     0,     0,
     836,     0,  1092,     0,   836,     0,     0,     0,     0,     0,
       0,     0,   232,     0,     0,     0,     0,     0,     0,   219,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   232,
       0,     0,  1110,     0,   219,   219,   837,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   232,     0,  1110,
       0,     0,     0,   835,     0,     0,     0,     0,   219,     0,
       0,     0,     0,     0,   835,     0,   836,     0,     0,     0,
     835,     0,     0,     0,   835,     0,     0,     0,     0,     0,
       0,     0,   333,   333,     0,   839,     0,     0,  1151,     0,
       0,   333,     0,   433,   434,   435,   232,     0,     0,     0,
       0,   977,     0,     0,     0,     0,     0,     0,     0,     0,
     233,     0,   436,  1359,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   835,   459,     0,     0,
       0,     0,     0,     0,     0,   232,     0,     0,     0,   460,
       0,     0,     0,   219,   219,     0,     0,     0,     0,     0,
       0,   232,     0,     0,     0,     0,     0,     0,     0,     0,
     232,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   232,     0,   839,     0,   219,
       0,     0,   839,   839,   839,   839,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   839,   839,
     839,   839,   839,   839,   839,   839,   839,   839,   839,   839,
     503,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,     0,     0,     0,     0,     0,     0,     0,
     837,     0,     0,     0,     0,   839,   221,   221,     0,     0,
     237,     0,     0,   333,   333,   837,   837,   837,   837,   837,
       0,     0,     0,   837,     0,     0,     0,   433,   434,   435,
     488,   489,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1360,   436,   219,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
       0,   459,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   460,     0,     0,     0,   219,     0,     0,
       0,     0,   219,     0,     0,   333,   490,   491,     0,     0,
       0,     0,     0,     0,     0,     0,   219,   219,     0,   839,
       0,   333,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   333,   839,     0,   839,     0,     0,
       0,     0,   837,   433,   434,   435,     0,     0,     0,     0,
       0,     0,     0,   839,     0,     0,     0,     0,     0,     0,
       0,     0,   436,   333,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,     0,   459,     0,     0,
       0,     0,     0,     0,  1388,     0,     0,   219,     0,   460,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   221,     0,     0,     0,     0,
       0,     0,     0,     0,   333,     0,     0,   333,     0,   820,
       0,  1102,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   837,     0,     0,     0,     0,   837,
     837,   837,   837,   837,   837,   837,   837,   837,   837,   837,
     837,   837,   837,   837,   837,   837,   837,   837,   837,   837,
     837,   837,   837,   837,   837,   837,   837,     0,     0,     0,
       0,     0,     0,     0,   839,   219,     0,     0,   839,     0,
     839,   435,     0,   839,     0,     0,     0,     0,     0,     0,
       0,     0,   837,  1475,     0,     0,  1488,     0,   436,     0,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   333,   459,   333,     0,     0,  1158,     0,     0,
       0,     0,     0,     0,   221,   460,     0,     0,     0,     0,
       0,     0,     0,   221,     0,   219,     0,     0,     0,     0,
     221,   333,     0,     0,   333,     0,     0,   221,     0,     0,
     839,     0,     0,     0,     0,     0,     0,     0,   237,     0,
    1547,  1548,     0,     0,     0,     0,     0,     0,     0,     0,
    1488,     0,     0,   818,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   837,     0,     0,     0,
       0,     0,     0,     0,     0,   333,     0,     0,     0,   333,
       0,     0,   837,     0,   837,     0,   503,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,     0,
     837,     0,    36,     0,     0,     0,     0,     0,     0,   237,
       0,   819,     0,     0,     0,   839,   839,     0,     0,     0,
       0,     0,   839,    48,  1693,     0,     0,     0,     0,     0,
       0,     0,  1488,   433,   434,   435,   488,   489,   333,   333,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     221,     0,   436,     0,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   182,   459,     0,    84,
       0,     0,    86,    87,     0,    88,   183,    90,     0,   460,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   490,   491,     0,   840,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,   333,     0,   333,     0,     0,     0,
       0,   837,     0,     0,     0,   837,     0,   837,     0,     0,
     837,     0,     0,     0,     0,     0,     0,     0,     0,   333,
       0,     0,     0,     0,     0,     0,     0,   840,     0,   841,
     333,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   839,   882,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   839,     0,     0,     0,     0,     0,   839,
       0,     0,     0,   839,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   875,   459,     0,     0,     0,     0,   837,     0,     0,
       0,     0,     0,     0,   460,     0,     0,  1169,     0,     0,
       0,     0,     0,   221,     0,   333,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   839,   459,     0,     0,   333,
       0,   333,     0,     0,  1806,     0,     0,   333,   460,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1475,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     221,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   837,   837,     0,     0,     0,     0,     0,   837,
       0,     0,     0,   433,   434,   435,     0,   333,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   221,   436,   221,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,     0,   459,     0,     0,
     221,   840,     0,     0,   433,   434,   435,     0,     0,   460,
       0,     0,     0,     0,     0,     0,   840,   840,   840,   840,
     840,     0,     0,   436,   840,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,     0,
       0,     0,     0,     0,     0,  1016,     0,     0,   333,     0,
     460,     0,   272,     0,     0,     0,     0,   221,     0,     0,
    1038,  1039,  1040,  1041,     0,   333,     0,     0,  1048,     0,
       0,     0,   221,   221,     0,     0,     0,     0,     0,   274,
       0,     0,     0,  1739,     0,     0,     0,     0,     0,   837,
       0,     0,     0,     0,     0,     0,   237,     0,     0,     0,
     837,    36,     0,     0,     0,     0,   837,     0,     0,     0,
     837,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,   840,     0,     0,     0,     0,     0,     0,
     553,     0,   333,     0,     0,     0,     0,  1192,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   237,     0,
       0,     0,     0,     0,     0,     0,     0,   547,   548,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   837,     0,     0,   182,     0,  1148,    84,   324,
       0,    86,    87,     0,    88,   183,    90,     0,  1511,     0,
       0,   221,   221,     0,     0,     0,     0,     0,     0,   328,
       0,     0,     0,     0,     0,     0,   333,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,   333,     0,   329,     0,   840,     0,   237,     0,     0,
     840,   840,   840,   840,   840,   840,   840,   840,   840,   840,
     840,   840,   840,   840,   840,   840,   840,   840,   840,   840,
     840,   840,   840,   840,   840,   840,   840,   840,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   840,  1235,  1238,  1239,  1240,  1242,  1243,
    1244,  1245,  1246,  1247,  1248,  1249,  1250,  1251,  1252,  1253,
    1254,  1255,  1256,  1257,  1258,  1259,  1260,  1261,  1262,  1263,
    1264,  1265,     0,     0,     0,     0,     0,   433,   434,   435,
       0,     0,     0,     0,     0,   221,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   436,  1277,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
       0,   459,     0,     0,     0,   237,     0,     0,     0,     0,
     221,     0,     0,   460,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   221,   221,     0,   840,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   840,     0,   840,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   840,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1354,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1077,     0,     0,  1369,     0,  1370,
       0,     0,     0,     0,     0,   221,   433,   434,   435,     0,
       0,     0,     0,     0,     0,  1380,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   436,     0,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,     0,
     459,  1512,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,   460,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   395,
      12,     0,     0,     0,     0,     0,     0,   740,     0,     0,
       0,     0,   840,   237,     0,     0,   840,     0,   840,    14,
      15,   840,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,  1459,     0,     0,     0,
    1461,    48,  1462,     0,     0,  1463,     0,     0,     0,    53,
       0,     0,     0,   237,     0,     0,     0,    60,    61,    62,
     178,   179,   180,     0,     0,    67,    68,     0,   840,     0,
       0,     0,     0,     0,     0,   181,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,   461,    80,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,  1542,    97,    98,   338,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,     0,     0,     0,   116,   117,     0,
     118,   119,     0,   840,   840,     0,     0,     0,     0,     0,
     840,   433,   434,   435,     0,     0,     0,     0,     0,  1698,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     436,     0,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,     0,   459,     0,  1686,  1687,     0,
       0,     0,     0,     0,  1692,     0,     0,   460,   433,   434,
     435,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   436,     0,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,     0,   459,   433,   434,   435,     0,     0,     0,     0,
       0,     0,     0,     0,   460,     0,     0,     0,     0,     0,
       0,     0,   436,     0,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,     0,   459,     0,     0,
     840,     0,     0,     0,     0,     0,     0,     0,     0,   460,
       0,   840,     0,     0,     0,     0,     0,   840,     0,     0,
       0,   840,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,     0,   544,     0,  1781,     0,    10,     0,     0,
       0,     0,     0,     0,  1748,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,  1758,     0,     0,     0,     0,
       0,  1763,     0,     0,     0,  1765,     0,     0,     0,     0,
      13,    14,    15,   840,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
     546,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,     0,
       0,    41,    42,    43,    44,     0,    45,     0,    46,     0,
      47,     0,     0,    48,    49,     0,     0,  1799,    50,    51,
      52,    53,    54,    55,    56,   564,    57,    58,    59,    60,
      61,    62,    63,    64,    65,     0,    66,    67,    68,    69,
      70,    71,     0,     0,     0,     0,     0,    72,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,    79,     0,
       0,    80,     0,     0,     0,     0,    81,    82,    83,    84,
      85,     0,    86,    87,     0,    88,    89,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,    95,     0,    96,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1118,   116,
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
       0,   113,     0,   114,   115,  1290,   116,   117,     0,   118,
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
     110,   111,   112,     0,     0,   113,     0,   114,   115,   668,
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
       0,     0,   113,     0,   114,   115,  1091,   116,   117,     0,
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
       0,   114,   115,  1132,   116,   117,     0,   118,   119,     5,
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
    1198,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,  1200,    45,     0,    46,     0,    47,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,  1355,     0,    48,
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
     115,  1466,   116,   117,     0,   118,   119,     5,     6,     7,
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
     111,   112,     0,     0,   113,     0,   114,   115,  1689,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,  1734,    47,     0,     0,
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
     114,   115,  1769,   116,   117,     0,   118,   119,     5,     6,
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
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1770,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,  1773,    46,     0,    47,     0,
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
       0,   182,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   183,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1790,   116,   117,     0,   118,   119,     5,
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
    1846,   116,   117,     0,   118,   119,     5,     6,     7,     8,
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
     112,     0,     0,   113,     0,   114,   115,  1853,   116,   117,
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
     113,     0,   114,   115,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   530,     0,     0,
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
       0,    11,    12,     0,   805,     0,     0,     0,     0,     0,
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
       0,  1018,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,  1537,     0,
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
       0,     0,    11,    12,     0,  1681,     0,     0,     0,     0,
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
      97,    98,   338,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   184,
       0,   339,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,     0,   459,
       0,   683,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   460,     0,    14,    15,     0,     0,     0,     0,    16,
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
       0,   684,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   184,     0,     0,     0,
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
     112,     0,     0,   184,     0,     0,   800,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1059,  1060,  1061,  1062,  1063,  1064,
    1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,
    1075,  1076,     0,     0,     0,  1145,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1077,     0,    14,    15,     0,
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
       0,    88,   183,    90,     0,  1146,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     184,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   395,    12,     0,     0,     0,     0,
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
     108,   109,   110,   111,   112,     0,     0,   113,     0,   433,
     434,   435,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   436,     0,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,     0,   459,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,   460,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
       0,    41,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,   195,     0,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   178,   179,   180,     0,     0,    67,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,  1084,  1085,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   184,     0,     0,     0,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10, -1013, -1013, -1013, -1013,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,     0,   459,     0,     0,   225,     0,     0,     0,
       0,     0,     0,     0,     0,   460,     0,     0,    14,    15,
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
       0,   184,     0,   433,   434,   435,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   436,     0,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,     0,   459,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,   460,
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
       0,     0,  1735,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   184,     0,
     258,   434,   435,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   436,
       0,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,     0,   459,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   460,     0,    16,     0,
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
     110,   111,   112,     0,     0,   184,     0,   261,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   395,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   433,   434,   435,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   436,     0,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     460,     0,    16,     0,    17,    18,    19,    20,    21,    22,
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
       0,     0,  1545,     0,     0,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   184,
     528,     0,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   696,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
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
     109,   110,   111,   112,     0,     0,   184,     0,     0,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,  1060,  1061,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,
    1073,  1074,  1075,  1076,     0,     0,     0,     0,   740,     0,
       0,     0,     0,     0,     0,     0,     0,  1077,     0,     0,
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
     112,     0,     0,   184,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10, -1013, -1013, -1013, -1013,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,     0,     0,     0,     0,   781,     0,     0,     0,     0,
       0,     0,     0,     0,  1077,     0,     0,    14,    15,     0,
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
     184,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   783,     0,     0,     0,     0,     0,     0,     0,
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
     108,   109,   110,   111,   112,     0,     0,   184,     0,     0,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1190,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
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
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   184,     0,   433,   434,   435,   116,
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
       0,     0,     0,     0,     0,     0,    60,    61,    62,   178,
     179,   180,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,    93,     0,  1382,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   184,     0,   433,   434,   435,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   436,   951,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,     0,   459,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,   460,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,   630,    38,     0,
       0,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,    60,    61,    62,   178,   179,   180,     0,
       0,    67,    68,     0,     0,     0,     0,     0,     0,     0,
       0,   181,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,    80,     0,     0,     0,     0,
     182,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   184,     0,
       0,     0,     0,   116,   117,     0,   118,   119,   263,   264,
       0,   265,   266,  1052,  1053,   267,   268,   269,   270,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1054,   271,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,     0,     0,     0,     0,   273,
       0,     0,     0,     0,     0,     0,     0,     0,  1077,     0,
       0,     0,     0,   275,   276,   277,   278,   279,   280,   281,
       0,     0,     0,    36,     0,   212,     0,     0,     0,     0,
       0,     0,     0,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,    48,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,     0,     0,
       0,   728,   317,   318,   319,     0,     0,     0,   320,   557,
     558,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   263,   264,     0,   265,   266,     0,   559,   267,   268,
     269,   270,     0,    86,    87,     0,    88,   183,    90,   325,
       0,   326,     0,     0,   327,   271,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   273,     0,     0,     0,     0,     0,   729,     0,
     116,     0,     0,     0,     0,     0,   275,   276,   277,   278,
     279,   280,   281,     0,     0,     0,    36,     0,   212,     0,
       0,     0,     0,     0,     0,     0,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,    48,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,     0,     0,     0,   316,   317,   318,   319,     0,     0,
       0,   320,   557,   558,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   263,   264,     0,   265,   266,     0,
     559,   267,   268,   269,   270,     0,    86,    87,     0,    88,
     183,    90,   325,     0,   326,     0,     0,   327,   271,     0,
     272,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   273,     0,   274,     0,     0,
       0,   729,     0,   116,     0,     0,     0,     0,     0,   275,
     276,   277,   278,   279,   280,   281,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
      48,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,     0,     0,     0,     0,   317,   318,
     319,    36,     0,     0,   320,   321,   322,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,   323,     0,     0,    84,   324,     0,    86,
      87,     0,    88,   183,    90,   325,     0,   326,     0,     0,
     327,     0,     0,     0,     0,     0,     0,   328,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,   263,
     264,   329,   265,   266,     0,  1661,   267,   268,   269,   270,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
       0,     0,     0,   271,     0,   272,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     273,     0,   274,   948,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   275,   276,   277,   278,   279,   280,
     281,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,    48,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
       0,     0,     0,   317,   318,   319,    36,     0,     0,   320,
     321,   322,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,   323,     0,
       0,    84,   324,     0,    86,    87,     0,    88,   183,    90,
     325,     0,   326,     0,     0,   327,     0,     0,     0,     0,
       0,     0,   328,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,   263,   264,   329,   265,   266,     0,
    1730,   267,   268,   269,   270,     0,    86,    87,     0,    88,
     183,    90,     0,     0,     0,     0,     0,     0,   271,     0,
     272,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   273,     0,   274,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   275,
     276,   277,   278,   279,   280,   281,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
      48,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,     0,     0,     0,   316,   317,   318,
     319,     0,     0,     0,   320,   321,   322,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   323,     0,     0,    84,   324,     0,    86,
      87,     0,    88,   183,    90,   325,     0,   326,     0,     0,
     327,     0,     0,     0,     0,     0,     0,   328,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,   263,
     264,   329,   265,   266,     0,     0,   267,   268,   269,   270,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   271,   436,   272,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,     0,   459,
     273,     0,   274,     0,     0,     0,     0,     0,     0,     0,
       0,   460,     0,     0,   275,   276,   277,   278,   279,   280,
     281,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,    48,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
       0,     0,     0,   317,   318,   319,     0,     0,     0,   320,
     321,   322,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   323,     0,
       0,    84,   324,     0,    86,    87,     0,    88,   183,    90,
     325,     0,   326,     0,     0,   327,     0,     0,     0,     0,
       0,     0,   328,  1470,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,   263,   264,   329,   265,   266,     0,
       0,   267,   268,   269,   270,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   271,  1054,
     272,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,     0,     0,   273,     0,   274,     0,     0,
       0,     0,     0,     0,     0,     0,  1077,     0,     0,   275,
     276,   277,   278,   279,   280,   281,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
      48,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,     0,     0,     0,     0,   317,   318,
     319,     0,     0,     0,   320,   321,   322,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,   323,     0,     0,    84,   324,     0,    86,
      87,     0,    88,   183,    90,   325,     0,   326,     0,    48,
     327,     0,     0,     0,     0,     0,     0,   328,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,   329,  1562,  1563,  1564,  1565,  1566,     0,     0,  1567,
    1568,  1569,  1570,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   182,     0,     0,    84,  1571,  1572,    86,    87,
       0,    88,   183,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1573,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,  1574,  1575,  1576,
    1577,  1578,  1579,  1580,  1737,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1581,  1582,  1583,
    1584,  1585,  1586,  1587,  1588,  1589,  1590,  1591,    48,  1592,
    1593,  1594,  1595,  1596,  1597,  1598,  1599,  1600,  1601,  1602,
    1603,  1604,  1605,  1606,  1607,  1608,  1609,  1610,  1611,  1612,
    1613,  1614,  1615,  1616,  1617,  1618,  1619,  1620,  1621,     0,
       0,     0,  1622,  1623,     0,  1624,  1625,  1626,  1627,  1628,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1629,  1630,  1631,     0,     0,     0,    86,    87,     0,
      88,   183,    90,  1632,     0,  1633,  1634,     0,  1635,     0,
       0,     0,     0,     0,     0,  1636,  1637,     0,  1638,     0,
    1639,  1640,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   263,   264,     0,   265,
     266,     0,  1053,   267,   268,   269,   270,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1054,
     271,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,     0,     0,     0,     0,   273,     0,     0,
       0,     0,     0,     0,     0,     0,  1077,     0,     0,     0,
       0,   275,   276,   277,   278,   279,   280,   281,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,    48,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,     0,     0,     0,   316,
     317,   318,   319,     0,     0,     0,   320,   557,   558,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   263,
     264,     0,   265,   266,     0,   559,   267,   268,   269,   270,
       0,    86,    87,     0,    88,   183,    90,   325,     0,   326,
       0,     0,   327,   271,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     273,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   275,   276,   277,   278,   279,   280,
     281,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,    48,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
       0,     0,  1233,   317,   318,   319,     0,     0,     0,   320,
     557,   558,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   263,   264,     0,   265,   266,     0,   559,   267,
     268,   269,   270,     0,    86,    87,     0,    88,   183,    90,
     325,     0,   326,     0,     0,   327,   271,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   273,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   275,   276,   277,
     278,   279,   280,   281,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,    48,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,     0,     0,     0,     0,   317,   318,   319,     0,
       0,     0,   320,   557,   558,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   559,     0,     0,     0,     0,     0,    86,    87,     0,
      88,   183,    90,   325,     0,   326,     0,     0,   327,     0,
       0,     0,     0,     0,     0,     0,     0,   433,   434,   435,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   436,     0,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
       0,   459,   433,   434,   435,     0,     0,     0,     0,     0,
       0,     0,     0,   460,     0,     0,     0,     0,     0,     0,
       0,   436,     0,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,     0,   459,   433,   434,   435,
       0,     0,     0,     0,     0,     0,     0,     0,   460,     0,
       0,     0,     0,     0,     0,     0,   436,     0,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
       0,   459,   433,   434,   435,     0,     0,     0,     0,     0,
       0,     0,     0,   460,     0,     0,     0,     0,     0,     0,
     272,   436,     0,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,     0,   459,   274,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   460,   588,
       0,     0,     0,     0,     0,     0,   272,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,   274,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   773,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   547,   548,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,   272,     0,   182,     0,     0,    84,   324,   797,    86,
      87,     0,    88,   183,    90,     0,  1033,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   328,   274,     0,
       0,   547,   548,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,  1088,   182,
      36,   329,    84,   324,     0,    86,    87,     0,    88,   183,
      90,     0,  1366,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,   328,     0,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,   329,     0,     0,
       0,     0,     0,     0,     0,     0,   547,   548,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,   182,   212,     0,    84,   324,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,   328,     0,
       0,     0,  1241,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   826,
     827,     0,   329,     0,     0,   828,     0,   829,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   830,
       0,     0,     0,     0,     0,     0,     0,    33,    34,    35,
      36,   757,     0,    86,    87,     0,    88,   183,    90,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,     0,     0,     0,   758,     0,
     116,     0,     0,     0,     0,     0,   831,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   215,
       0,     0,     0,     0,   182,    82,    83,    84,   832,     0,
      86,    87,     0,    88,   183,    90,     0,  1012,     0,    92,
       0,     0,     0,     0,     0,     0,     0,     0,   833,     0,
       0,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    28,
       0,     0,   834,     0,     0,     0,     0,    33,    34,    35,
      36,     0,   212,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,     0,     0,     0,  1478,
       0,     0,     0,     0,     0,  1013,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   215,
       0,     0,     0,     0,   182,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   183,    90,     0,     0,     0,    92,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,    97,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    48,
       0,     0,   216,     0,     0,     0,     0,   116,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   826,   827,     0,
       0,  1479,     0,   828,     0,   829,     0,     0,     0,     0,
       0,     0,     0,     0,  1480,  1481,     0,   830,     0,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,     0,
       0,     0,   182,     0,     0,    84,  1482,   213,    86,    87,
       0,    88,  1483,    90,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,  1172,  1173,  1174,
      36,     0,     0,     0,   831,     0,    74,    75,    76,    77,
      78,     0,     0,    36,     0,   212,     0,   215,     0,     0,
       0,    48,   182,    82,    83,    84,   832,     0,    86,    87,
       0,    88,   183,    90,    48,     0,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,   833,   979,   980,     0,
       0,    97,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   981,     0,     0,
     834,     0,     0,     0,     0,   982,   983,   984,    36,     0,
      86,    87,     0,    88,   183,    90,     0,   985,     0,     0,
       0,   757,     0,    86,    87,     0,    88,   183,    90,    48,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   986,     0,     0,     0,   792,     0,
     116,     0,     0,     0,     0,     0,     0,   987,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   988,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    28,     0,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,     0,
     212,     0,     0,     0,     0,     0,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     214,   459,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   460,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     0,   182,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   183,    90,     0,     0,     0,    92,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    97,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,    28,     0,
     216,     0,     0,   601,     0,   116,    33,    34,    35,    36,
       0,   212,     0,     0,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   214,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   621,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,     0,     0,     0,    92,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    97,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
      28,   216,   968,     0,     0,     0,   116,     0,    33,    34,
      35,    36,     0,   212,     0,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1077,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
     215,     0,     0,     0,     0,   182,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   183,    90,     0,     0,     0,
      92,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,    28,     0,   216,     0,     0,     0,     0,   116,    33,
      34,    35,    36,     0,   212,     0,     0,     0,     0,     0,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1113,    73,     0,
      74,    75,    76,    77,    78,     0,     0,     0,     0,     0,
       0,   215,     0,     0,     0,     0,   182,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    92,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,    28,     0,   216,     0,     0,     0,     0,   116,
      33,    34,    35,    36,     0,   212,     0,     0,     0,     0,
       0,     0,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
       0,     0,    36,     0,   896,   897,     0,     0,     0,    73,
       0,    74,    75,    76,    77,    78,     0,     0,    36,     0,
       0,     0,   215,    48,     0,     0,     0,   182,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   183,    90,    48,
       0,     0,    92,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   216,    33,    34,    35,    36,
     116,   212,    86,    87,     0,    88,   183,    90,   213,     0,
       0,     0,     0,     0,     0,   380,     0,     0,    86,    87,
      48,    88,   183,    90,     0,     0,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   230,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    74,    75,    76,
      77,    78,     0,   381,    36,     0,     0,     0,   215,     0,
       0,     0,     0,   182,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   183,    90,    48,     0,     0,    92,     0,
       0,     0,     0,   347,   348,     0,     0,     0,     0,     0,
       0,     0,    97,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,   231,    33,    34,    35,    36,   116,   212,     0,     0,
       0,     0,     0,     0,   644,     0,     0,     0,     0,     0,
       0,   349,     0,     0,    86,    87,    48,    88,   183,    90,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   214,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,   182,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,    92,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   433,   434,   435,   645,     0,     0,
       0,     0,   116,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   436,     0,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,   433,
     434,   435,     0,     0,     0,     0,     0,     0,     0,     0,
     460,     0,     0,     0,     0,     0,     0,     0,   436,     0,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,     0,   459,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   460,   433,   434,   435,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   436,   506,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,     0,
     459,   433,   434,   435,     0,     0,     0,     0,     0,     0,
       0,     0,   460,     0,     0,     0,     0,     0,     0,     0,
     436,   515,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,     0,   459,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   460,   433,   434,
     435,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   436,   954,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,     0,   459,  1051,  1052,  1053,     0,     0,     0,     0,
       0,     0,     0,     0,   460,     0,     0,     0,     0,     0,
       0,     0,  1054,   998,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1077,
    1051,  1052,  1053,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1054,
    1319,  1055,  1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,
    1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,
    1074,  1075,  1076,     0,     0,  1051,  1052,  1053,     0,     0,
       0,     0,     0,     0,     0,     0,  1077,     0,     0,     0,
       0,     0,     0,     0,  1054,  1223,  1055,  1056,  1057,  1058,
    1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,
    1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1077,  1051,  1052,  1053,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1054,  1376,  1055,  1056,  1057,  1058,  1059,  1060,  1061,
    1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1077,     0,
       0,    36,     0,     0,     0,     0,  1646,  1458,    86,    87,
    1647,    88,   183,    90,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,  1492,    36,     0,
    1493,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,  1544,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    86,    87,    48,    88,   183,    90,     0,     0,     0,
       0,  1479,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,  1480,  1481,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      36,     0,   182,  1493,    48,    84,    85,     0,    86,    87,
       0,    88,  1483,    90,     0,     0,   182,     0,     0,    84,
      85,    48,    86,    87,     0,    88,   183,    90,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   420,     0,    86,    87,    36,    88,   183,    90,     0,
       0,     0,     0,     0,     0,     0,     0,   589,     0,     0,
      86,    87,    36,    88,   183,    90,    48,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,    48,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   593,   459,     0,    86,    87,     0,    88,   183,
      90,     0,     0,     0,     0,   460,     0,     0,     0,   349,
       0,     0,    86,    87,     0,    88,   183,    90,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   433,   434,   435,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,   809,   436,     0,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,     0,   459,   433,
     434,   435,     0,     0,     0,     0,     0,     0,     0,     0,
     460,     0,     0,     0,     0,     0,     0,     0,   436,     0,
     437,   438,   439,   440,   441,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   810,   459,  1051,  1052,  1053,     0,     0,     0,
       0,     0,     0,     0,     0,   460,     0,     0,     0,     0,
       0,     0,     0,  1054,  1381,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1051,  1052,  1053,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1077,     0,     0,     0,     0,     0,  1054,     0,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1077,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1077,
     438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,     0,   459,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   460,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1077
};

static const yytype_int16 yycheck[] =
{
       5,     6,   135,     8,     9,    10,    11,    12,   161,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   113,    54,    28,    29,    91,   186,   686,   500,    95,
      96,   927,    32,   232,     4,   228,    55,    42,     4,  1135,
       4,     4,   388,   166,    44,    50,   161,    52,   388,    49,
      55,   113,    57,    42,   187,   656,    54,  1122,   388,   637,
     916,    30,   242,   459,   496,   497,   492,   657,   134,   113,
     815,    30,     4,   775,    79,    30,    30,   492,   113,  1011,
     737,   947,     9,    30,     9,    30,     4,   530,     9,     9,
      79,    67,   524,   808,  1131,     9,    67,   963,     9,     9,
     526,     9,     9,    14,    14,     9,    14,     9,   113,     9,
       9,   526,     9,     9,     9,     9,     9,     9,    46,     9,
     243,     9,   184,     9,     9,     9,     9,    67,    46,    33,
       9,     9,     9,     9,     9,   151,   111,     9,    67,    51,
     184,     4,   585,    67,    67,    80,   157,    80,  1675,   184,
      67,    67,   130,   131,   216,   101,   102,   103,   118,   130,
     131,    35,   200,   119,   151,    46,   126,    80,   151,   231,
     126,    86,   216,    86,   116,   151,    46,   523,    46,   184,
       4,   216,    46,    86,   200,   172,   191,   231,   200,   172,
     108,  1047,   203,   373,   169,   113,    67,   115,   116,   117,
     118,   119,   120,   121,  1731,    67,    80,    67,    67,    67,
      67,   216,    67,   130,   131,   658,   203,    35,   151,    67,
      67,    35,   200,     0,   200,    67,   231,   203,   151,    67,
     165,    67,   203,   148,   151,    67,   149,   150,   200,   244,
     158,   159,   247,   161,   156,   148,    58,    80,   173,   254,
     255,  1316,     8,   173,    80,   203,   203,   202,  1323,    80,
    1325,   172,    80,   203,   182,   425,    80,    80,    51,    81,
     198,   200,    84,  1205,   173,   202,  1021,  1314,  1023,   203,
     203,   202,   203,   200,   200,   248,   204,   203,   202,   252,
     156,   202,   202,   151,   202,   202,   209,   201,   161,   201,
    1166,   173,   202,   365,   201,   337,   202,   202,   202,   202,
     202,   135,   202,  1390,   202,   201,   509,   202,   202,   202,
     201,   365,   201,   201,   201,   201,   201,   905,   198,  1185,
     365,   201,   203,   201,    80,    35,    35,   201,   200,   337,
     200,   474,   502,   203,   203,   203,   203,   165,   203,    26,
      27,   165,   200,   419,   944,   203,   203,   800,   200,   364,
     365,   203,   805,   187,    98,   203,   371,   203,   200,   201,
    1435,   376,   205,   156,    80,   364,   197,   203,   157,   205,
      80,    80,   203,   475,   416,   172,   172,  1424,   171,  1426,
     395,   170,   205,   102,   103,   102,   103,    54,   403,  1476,
     101,   102,   103,   469,   470,   471,   472,   130,   131,    66,
     415,    80,    98,   200,   200,    67,    35,    30,   416,   165,
     892,   155,   200,  1500,   203,  1502,    47,    48,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   647,   460,   475,   462,   463,   165,
     465,    80,   394,  1120,     4,   165,   165,   200,   466,   155,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   663,  1186,   665,  1201,  1524,   493,   494,
     459,   496,   497,   498,   499,   204,   165,   204,   503,   155,
     459,   506,   172,     4,   459,   459,   200,   933,   130,   131,
     515,    51,   517,   532,    54,   513,    98,  1373,   933,   524,
      80,   674,   200,    80,   466,   172,    86,   532,   960,   534,
     200,    71,   731,  1134,   727,   347,   348,   349,    67,   402,
      98,    80,   117,   118,   119,    46,    98,    86,   225,    89,
     492,    91,    80,   200,   523,    95,    96,   200,    86,   674,
    1150,   200,   567,  1153,   763,   570,   571,   572,   380,    80,
     916,   513,   968,   155,   537,  1018,   916,    98,   402,   739,
     172,   523,   204,   645,   526,    80,   916,  1443,   200,   149,
     150,    86,   149,   150,   134,   200,   601,   155,   200,   779,
     780,   130,   131,   155,   200,   785,   786,   108,   200,   148,
     149,   150,   113,   200,   115,   116,   117,   118,   119,   120,
     121,   149,   150,    72,    73,    29,   692,    80,   155,   822,
    1332,   202,  1334,    86,   155,   151,   835,   148,   149,   150,
     645,  1678,     4,    47,   843,    80,    50,    35,   208,   473,
      67,    86,     4,   148,   149,   150,   172,   158,   159,   199,
     161,   202,   785,   151,    26,    27,   202,  1324,   115,   116,
     117,   118,   119,   120,   463,   202,   203,   354,   202,   684,
     202,   182,    72,    73,   200,   202,   363,   203,    67,    14,
     202,   696,    29,   370,    46,   203,   149,   150,   128,   129,
     377,  1047,  1303,   204,   493,    30,   200,  1047,   248,   498,
      47,   388,   252,    50,   149,   150,   256,  1047,    50,    51,
      52,  1817,    54,    48,   729,   657,   200,  1159,  1318,   115,
     116,   117,  1204,   107,    66,   182,  1832,    67,  1170,  1441,
     151,   115,   116,   117,   118,   119,   120,   155,  1405,    98,
      99,   113,   200,   758,    45,   202,   108,    50,    51,    52,
     202,   113,    66,   115,   116,   117,   118,   119,   120,   121,
     202,   203,   200,    66,  1811,   202,   203,   589,  1705,  1706,
     172,   593,   151,   183,   184,   185,   598,   792,   200,  1826,
     190,   191,  1701,  1702,   194,   195,     9,   337,   824,   825,
     993,   374,   207,   808,   804,   378,   158,   159,   182,   161,
      54,   674,   151,  1285,   115,   116,   117,   118,   119,   120,
    1477,     8,   184,   500,   151,   200,   202,   200,   151,  1531,
     182,  1274,   405,    14,   407,   408,   409,   410,   151,  1185,
     845,    80,   202,   202,  1037,  1185,  1436,   126,   126,    14,
     813,  1044,   204,   201,   216,  1185,   115,   116,   117,   118,
     119,   120,   402,   225,   172,    14,    98,   126,   127,   231,
     201,   206,   412,   201,   107,   201,   416,   201,   200,   419,
     200,   182,   200,     9,   148,   201,   248,   201,  1320,  1785,
     252,   201,   201,    90,     9,   202,   901,    14,   172,     9,
     200,   967,  1345,   186,    80,   164,    80,    80,  1804,   189,
     915,   200,   202,     9,   202,     9,  1812,    80,  1361,   201,
     201,   919,   128,   182,   201,   200,   466,   467,   468,   469,
     470,   471,   472,   202,   904,   798,   941,    67,   904,   201,
     904,   904,  1141,    30,   129,   171,   951,   151,   132,   954,
       9,   956,   492,   201,    14,   960,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,   151,
      54,   198,   904,   513,   798,     9,     9,   919,  1680,   173,
     201,     9,    66,    14,   128,   207,   526,  1180,   207,   931,
    1422,   933,   354,   998,   204,   927,   859,   537,     9,   968,
      14,   363,  1445,   365,  1004,   207,   683,   207,   370,   968,
     201,  1454,   944,   968,   968,   377,   556,   200,    26,    27,
     151,   201,  1465,  1115,   201,    98,    87,  1373,   272,   132,
     274,   202,   202,  1373,     9,   859,   200,  1230,   151,   201,
     402,   904,  1005,  1373,  1237,   151,   200,   151,  1046,   186,
     203,  1049,   864,   186,    14,     9,   868,   999,    80,    14,
     600,   203,   925,   740,   202,    14,  1725,   203,   207,    47,
      48,    49,    50,    51,    52,   203,  1081,  1082,  1083,    14,
     904,   201,  1087,  1088,   202,   329,   626,   627,    66,   198,
      30,    30,   200,   200,  1537,    14,  1115,  1443,   200,   200,
      14,   925,    49,  1443,   781,   200,   783,   200,     9,   201,
    1115,   202,   202,  1443,   200,   132,    75,    76,    77,    14,
      47,    48,    49,    50,    51,    52,   132,    54,    87,     9,
     201,   207,     4,   810,    66,     9,    80,   200,   500,    66,
       9,  1146,   132,  1802,  1007,   200,  1009,    14,  1341,  1119,
     202,    80,   692,  1119,  1159,  1119,  1119,   201,   203,   200,
     200,   203,   201,   132,   202,  1170,  1171,   203,     9,   413,
     207,    87,   416,    30,    46,   537,   148,   136,   137,   138,
     139,   140,    74,  1007,   202,  1009,   201,  1119,   147,   202,
     173,   132,    30,  1193,   153,   154,  1201,   201,   201,   132,
     877,     9,   201,   204,     9,   201,  1211,   204,   167,   203,
      14,   200,    80,   201,   201,   892,   893,   225,  1150,   201,
     200,  1153,   181,   202,   132,   201,   203,  1670,   201,  1672,
     201,     9,    30,  1196,   202,   775,   108,   777,  1681,   916,
     201,   113,   108,   115,   116,   117,   118,   119,   120,   121,
      26,    27,   202,  1406,   202,   201,  1119,   160,   798,   156,
     203,    14,    80,   113,   202,   201,   203,   201,  1401,   132,
     201,   132,   812,   813,    14,   203,   172,   202,    80,    14,
      14,    80,   203,   201,   201,  1728,   158,   159,   200,   161,
    1295,   202,   132,   202,  1299,  1119,  1301,    14,    14,    14,
     202,     9,    56,   203,  1309,   204,    80,   551,   172,   200,
     182,    80,     9,   202,  1319,  1320,    80,   111,    98,   859,
     151,   683,    33,    98,   163,   865,    14,   200,   173,   869,
     870,   201,   204,   169,  1011,  1012,    80,   202,  1498,   166,
     200,     4,   201,     9,    80,  1208,   354,   202,   201,   889,
     201,  1283,    14,    80,   203,   363,    14,   365,    80,    14,
    1292,    80,   370,    14,   904,   864,    80,   598,   868,   377,
    1047,  1793,   472,   966,   907,   961,  1358,  1382,   740,   919,
     469,  1808,   467,    46,  1208,   925,  1318,  1830,  1536,   603,
    1804,   931,  1202,   933,  1837,  1527,  1560,  1478,  1644,  1396,
    1836,  1824,  1656,  1523,  1470,   475,  1392,  1090,   574,   653,
     654,  1087,   856,   574,  1419,  1123,  1168,  1422,   662,   781,
    1283,   783,  1045,  1181,   980,  1182,   995,   967,   931,  1292,
     416,   824,  1385,  1760,  1106,  1030,   798,  1078,    -1,   979,
     980,   981,    -1,    -1,    -1,   108,    -1,   371,   810,   225,
     113,   813,   115,   116,   117,   118,   119,   120,   121,   999,
      -1,    -1,    -1,    -1,    -1,  1005,    -1,  1007,  1145,  1009,
      -1,    -1,    -1,    -1,    -1,  1473,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1451,  1029,
      -1,    -1,   500,  1425,    -1,   158,   159,   859,   161,  1431,
    1660,  1433,    -1,  1696,  1436,    -1,  1046,    -1,  1185,  1049,
    1515,    -1,    -1,  1190,    -1,   877,    -1,    -1,    -1,   182,
      -1,  1384,    -1,    -1,    -1,    -1,    -1,  1204,  1205,    -1,
     892,   893,    -1,    -1,    -1,    -1,    -1,    -1,  1078,    -1,
    1545,   204,   904,  1406,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1095,    -1,    -1,  1098,    -1,
    1384,    -1,  1425,   925,    -1,    -1,    -1,    -1,  1431,    -1,
    1433,    -1,  1535,  1536,    -1,    -1,  1400,    -1,   354,  1119,
     824,   825,    -1,  1446,    -1,    -1,    -1,   363,    -1,    -1,
       4,    -1,  1455,    -1,   370,    -1,    -1,  1529,    -1,    -1,
      -1,   377,    -1,    -1,    -1,    -1,    -1,    -1,  1285,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,  1446,  1655,    -1,    -1,  1779,    -1,    -1,    -1,
      -1,  1455,    46,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,  1182,  1005,    -1,  1007,  1186,  1009,    -1,  1011,
    1012,    -1,    -1,  1719,    -1,    -1,  1196,  1655,    -1,    -1,
    1665,    -1,   906,    -1,  1797,    -1,  1529,    -1,  1208,    -1,
      -1,  1534,    -1,    -1,    -1,   683,    -1,  1540,   922,    -1,
      -1,    -1,  1506,  1546,    -1,    -1,    -1,    -1,    -1,    26,
      27,   935,    -1,    30,   108,    -1,  1373,    -1,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,    -1,    -1,
    1534,    -1,    -1,    -1,    -1,    -1,  1540,    54,    -1,    -1,
     964,    -1,  1546,    -1,   500,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   740,    -1,    -1,    -1,    -1,    -1,  1278,    -1,
      -1,    -1,    -1,    -1,   158,   159,    -1,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1443,    -1,   182,    -1,
      -1,    -1,    -1,   781,    -1,   783,    -1,    -1,    -1,    -1,
      -1,  1025,    -1,  1145,  1028,    -1,    -1,    -1,    -1,    -1,
     204,    -1,  1332,    -1,  1334,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   810,    -1,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,  1685,    -1,    -1,    -1,    -1,    -1,  1190,    -1,
      -1,    -1,    -1,    -1,  1196,    -1,    -1,  1842,    -1,    -1,
      -1,    -1,  1204,  1205,  1384,  1850,  1208,    -1,    -1,    -1,
    1390,  1856,    -1,  1785,  1859,    -1,  1396,    -1,    64,    65,
      -1,  1685,    -1,  1726,  1727,    -1,    -1,    -1,    -1,   877,
    1733,    -1,  1804,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1812,    -1,    -1,    -1,   892,   893,    -1,    -1,   225,  1133,
      -1,  1135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1441,  1726,  1727,    -1,    -1,  1446,   683,  1771,  1733,
      -1,  1451,    -1,    -1,    -1,  1455,  1779,    -1,  1162,    -1,
      -1,  1165,    -1,  1285,   130,   131,    -1,    -1,    -1,    -1,
    1470,    -1,    -1,  1473,    -1,   272,  1476,   274,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1486,  1771,    -1,    -1,
      -1,    -1,    -1,  1493,    -1,    -1,    -1,    -1,    -1,    -1,
    1500,    -1,  1502,    -1,   740,    -1,    -1,    -1,  1508,    -1,
      -1,    -1,  1216,    -1,    -1,  1838,  1220,    -1,    -1,    -1,
      -1,    -1,  1845,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1531,   329,    -1,  1534,  1535,  1536,    -1,    -1,    -1,
    1540,    -1,    -1,  1011,  1012,   781,  1546,   783,    -1,    -1,
      -1,    -1,    -1,    -1,  1838,    -1,    -1,   354,    -1,    -1,
      -1,  1845,  1384,    -1,    -1,    -1,   363,    -1,    -1,    -1,
      -1,    -1,    -1,   370,   810,  1279,  1280,    -1,    -1,    -1,
     377,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   388,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   413,    -1,    -1,   416,
      -1,    -1,    -1,    -1,  1446,    -1,    -1,    -1,    -1,  1451,
      -1,    -1,    -1,  1455,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   877,    64,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1655,   892,   893,    -1,    -1,
      -1,  1365,   459,  1367,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1676,  1145,    -1,    -1,
    1680,    -1,    -1,    -1,    -1,  1685,    -1,    -1,   555,    -1,
      -1,    -1,    -1,    -1,  1694,    -1,    -1,  1401,    -1,    -1,
      -1,  1701,  1702,   500,    -1,  1705,  1706,    -1,   130,   131,
      -1,    -1,  1534,  1535,  1536,    -1,    -1,    29,  1540,  1719,
      -1,    -1,  1190,    -1,  1546,    -1,  1726,  1727,    -1,    -1,
      -1,    -1,    -1,  1733,    -1,    -1,  1204,  1205,    -1,    -1,
     607,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   551,   552,    -1,    -1,   555,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      -1,  1771,    -1,    -1,    -1,  1011,  1012,    -1,  1778,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,  1796,    -1,    -1,    -1,
      -1,    -1,    -1,   115,   116,   117,   118,   119,   120,    -1,
     607,    -1,    -1,    -1,    -1,    -1,    -1,  1285,    -1,    -1,
      -1,    -1,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1838,    -1,
     152,    -1,    -1,   155,   156,  1845,   158,   159,    -1,   161,
     162,   163,    -1,    -1,  1558,    -1,   653,   654,    -1,    -1,
      -1,    -1,    -1,  1685,   176,   662,    -1,    -1,    -1,    -1,
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,   683,    -1,   200,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1726,  1727,    -1,    -1,    -1,  1145,
      29,  1733,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   740,   811,    -1,    -1,    66,    -1,  1771,
      -1,    -1,    -1,    -1,  1190,  1659,    -1,    -1,    -1,   826,
     827,   828,   829,   830,    -1,    -1,    -1,   834,  1204,  1205,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   781,    -1,   783,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,   810,   811,    -1,  1838,    -1,    26,    27,
      -1,    -1,    30,  1845,    -1,    -1,    66,   824,   825,   826,
     827,   828,   829,   830,    -1,    -1,    -1,   834,    -1,  1743,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,  1285,
      -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   860,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   943,    -1,    -1,    -1,
     877,    -1,    -1,    -1,    -1,    -1,    -1,   162,   207,   164,
      -1,    -1,    -1,   890,    -1,   892,   893,    -1,    -1,    -1,
      -1,    -1,   177,    -1,   179,    -1,    -1,   182,    -1,   906,
     907,    -1,    -1,  1817,    -1,    -1,    -1,    -1,    -1,   916,
      -1,    -1,    -1,    -1,    -1,   922,    -1,    -1,  1832,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   935,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   943,    -1,    -1,   946,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   201,    -1,    -1,    -1,    -1,    -1,   964,    -1,    -1,
      -1,   968,    -1,    -1,    -1,    -1,    -1,    -1,  1045,    -1,
      -1,    -1,    -1,  1050,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,    -1,    -1,    -1,  1011,  1012,    -1,   225,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1025,    -1,
      -1,  1028,    -1,  1030,    -1,    -1,  1103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1045,    -1,
    1047,    -1,    -1,  1050,  1051,  1052,  1053,  1054,  1055,  1056,
    1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,
    1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,  1103,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    54,    -1,    -1,  1133,    -1,  1135,    -1,
    1207,    -1,    66,    -1,    -1,    -1,   354,    -1,  1145,    -1,
      -1,    -1,    -1,    -1,    -1,   363,  1223,    -1,  1225,    -1,
      -1,    -1,   370,    -1,    -1,  1162,    -1,    -1,  1165,   377,
      -1,    -1,    -1,    -1,  1241,    -1,    -1,    -1,    -1,    -1,
     388,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1185,    -1,
      -1,    -1,    -1,  1190,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1204,  1205,    -1,
    1207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1216,
      -1,    -1,    -1,  1220,    -1,    -1,  1223,    -1,  1225,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1241,    -1,    -1,    -1,    -1,    -1,
      29,   459,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
     204,    -1,  1279,  1280,    -1,  1282,    -1,    66,  1285,    -1,
      -1,    -1,   500,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1372,    -1,    -1,    -1,  1376,
      -1,  1378,    -1,    29,  1381,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   555,    -1,    -1,
      66,   272,    -1,   274,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1365,    -1,
    1367,    -1,    -1,    -1,    -1,  1372,  1373,    -1,    -1,  1376,
      -1,  1378,    -1,    -1,  1381,    -1,    -1,    -1,    -1,    -1,
      -1,  1458,    -1,  1390,  1391,    -1,    -1,  1394,    -1,   607,
      -1,    -1,    -1,    -1,  1401,    -1,    78,    -1,   329,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   204,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1443,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,  1458,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1468,  1469,    -1,    -1,   683,  1543,  1544,    -1,  1476,
      -1,  1478,    -1,  1550,    -1,    -1,   158,   159,   204,   161,
     162,   163,   413,    -1,    -1,   416,    -1,    -1,    64,    65,
      -1,    -1,    -1,  1500,    -1,  1502,    -1,    -1,    -1,    -1,
      -1,  1508,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,    -1,   200,    -1,
      -1,    -1,   740,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1543,  1544,    -1,    -1,
      -1,    -1,    -1,  1550,    -1,  1552,    -1,    10,    11,    12,
      -1,  1558,    -1,  1560,   130,   131,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   781,    -1,   783,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,   810,   811,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,   826,   827,
     828,   829,   830,    -1,    -1,    -1,   834,    -1,    -1,    -1,
     551,   552,    -1,  1700,   555,   201,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1711,    -1,    -1,    -1,    -1,    -1,
    1717,    -1,   860,    -1,  1721,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1659,    -1,    -1,    -1,    -1,    -1,    -1,   877,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1676,
      -1,    -1,   890,    -1,   892,   893,   607,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1694,    -1,   907,
      -1,    -1,    -1,  1700,    -1,    -1,    -1,    -1,   916,    -1,
      -1,    -1,    -1,    -1,  1711,    -1,  1783,    -1,    -1,    -1,
    1717,    -1,    -1,    -1,  1721,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   653,   654,    -1,   943,    -1,    -1,   946,    -1,
      -1,   662,    -1,    10,    11,    12,  1743,    -1,    -1,    -1,
      -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     968,    -1,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,  1783,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1792,    -1,    -1,    -1,    66,
      -1,    -1,    -1,  1011,  1012,    -1,    -1,    -1,    -1,    -1,
      -1,  1808,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1817,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1832,    -1,  1045,    -1,  1047,
      -1,    -1,  1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,
    1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     811,    -1,    -1,    -1,    -1,  1103,    26,    27,    -1,    -1,
      30,    -1,    -1,   824,   825,   826,   827,   828,   829,   830,
      -1,    -1,    -1,   834,    -1,    -1,    -1,    10,    11,    12,
      64,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   202,    29,  1145,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,  1185,    -1,    -1,
      -1,    -1,  1190,    -1,    -1,   906,   130,   131,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1204,  1205,    -1,  1207,
      -1,   922,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   935,  1223,    -1,  1225,    -1,    -1,
      -1,    -1,   943,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1241,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   964,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,  1282,    -1,    -1,  1285,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   225,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1025,    -1,    -1,  1028,    -1,  1030,
      -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1045,    -1,    -1,    -1,    -1,  1050,
    1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,  1070,
    1071,  1072,  1073,  1074,  1075,  1076,  1077,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1372,  1373,    -1,    -1,  1376,    -1,
    1378,    12,    -1,  1381,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1103,  1391,    -1,    -1,  1394,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,  1133,    54,  1135,    -1,    -1,   204,    -1,    -1,
      -1,    -1,    -1,    -1,   354,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   363,    -1,  1443,    -1,    -1,    -1,    -1,
     370,  1162,    -1,    -1,  1165,    -1,    -1,   377,    -1,    -1,
    1458,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   388,    -1,
    1468,  1469,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1478,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1207,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1216,    -1,    -1,    -1,  1220,
      -1,    -1,  1223,    -1,  1225,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
    1241,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,   459,
      -1,    87,    -1,    -1,    -1,  1543,  1544,    -1,    -1,    -1,
      -1,    -1,  1550,    99,  1552,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1560,    10,    11,    12,    64,    65,  1279,  1280,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     500,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,   152,    54,    -1,   155,
      -1,    -1,   158,   159,    -1,   161,   162,   163,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,   555,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,    -1,  1365,    -1,  1367,    -1,    -1,    -1,
      -1,  1372,    -1,    -1,    -1,  1376,    -1,  1378,    -1,    -1,
    1381,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1390,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   607,    -1,   555,
    1401,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1700,   201,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1711,    -1,    -1,    -1,    -1,    -1,  1717,
      -1,    -1,    -1,  1721,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,   607,    54,    -1,    -1,    -1,    -1,  1458,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,   204,    -1,    -1,
      -1,    -1,    -1,   683,    -1,  1476,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,  1783,    54,    -1,    -1,  1500,
      -1,  1502,    -1,    -1,  1792,    -1,    -1,  1508,    66,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1808,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     740,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1543,  1544,    -1,    -1,    -1,    -1,    -1,  1550,
      -1,    -1,    -1,    10,    11,    12,    -1,  1558,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   781,    29,   783,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
     810,   811,    -1,    -1,    10,    11,    12,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,   826,   827,   828,   829,
     830,    -1,    -1,    29,   834,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,   811,    -1,    -1,  1659,    -1,
      66,    -1,    29,    -1,    -1,    -1,    -1,   877,    -1,    -1,
     826,   827,   828,   829,    -1,  1676,    -1,    -1,   834,    -1,
      -1,    -1,   892,   893,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    -1,  1694,    -1,    -1,    -1,    -1,    -1,  1700,
      -1,    -1,    -1,    -1,    -1,    -1,   916,    -1,    -1,    -1,
    1711,    78,    -1,    -1,    -1,    -1,  1717,    -1,    -1,    -1,
    1721,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,   943,    -1,    -1,    -1,    -1,    -1,    -1,
     107,    -1,  1743,    -1,    -1,    -1,    -1,   204,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   968,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1783,    -1,    -1,   152,    -1,   943,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,   204,    -1,
      -1,  1011,  1012,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,    -1,  1817,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,  1832,    -1,   200,    -1,  1045,    -1,  1047,    -1,    -1,
    1050,  1051,  1052,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060,  1061,  1062,  1063,  1064,  1065,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1103,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,  1058,  1059,  1060,  1061,  1062,  1063,  1064,  1065,
    1066,  1067,  1068,  1069,  1070,  1071,  1072,  1073,  1074,  1075,
    1076,  1077,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,  1145,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,  1103,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,  1185,    -1,    -1,    -1,    -1,
    1190,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1204,  1205,    -1,  1207,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1223,    -1,  1225,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1241,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,  1207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,  1223,    -1,  1225,
      -1,    -1,    -1,    -1,    -1,  1285,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,  1241,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,   204,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    66,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,  1372,  1373,    -1,    -1,  1376,    -1,  1378,    47,
      48,  1381,    -1,    -1,    -1,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      68,    69,    70,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    -1,  1372,    -1,    -1,    -1,
    1376,    99,  1378,    -1,    -1,  1381,    -1,    -1,    -1,   107,
      -1,    -1,    -1,  1443,    -1,    -1,    -1,   115,   116,   117,
     118,   119,   120,    -1,    -1,   123,   124,    -1,  1458,    -1,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,   202,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,  1458,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,    -1,
     208,   209,    -1,  1543,  1544,    -1,    -1,    -1,    -1,    -1,
    1550,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,  1559,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,  1543,  1544,    -1,
      -1,    -1,    -1,    -1,  1550,    -1,    -1,    66,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
    1700,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      -1,  1711,    -1,    -1,    -1,    -1,    -1,  1717,    -1,    -1,
      -1,  1721,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    -1,   202,    -1,  1745,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,  1700,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,  1711,    -1,    -1,    -1,    -1,
      -1,  1717,    -1,    -1,    -1,  1721,    -1,    -1,    -1,    -1,
      46,    47,    48,  1783,    -1,    -1,    -1,    53,    -1,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
     202,    67,    68,    69,    70,    71,    -1,    -1,    -1,    75,
      76,    77,    78,    79,    80,    -1,    82,    83,    -1,    -1,
      -1,    87,    88,    89,    90,    -1,    92,    -1,    94,    -1,
      96,    -1,    -1,    99,   100,    -1,    -1,  1783,   104,   105,
     106,   107,   108,   109,   110,   202,   112,   113,   114,   115,
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
      87,    88,    89,    90,    91,    92,    -1,    94,    -1,    96,
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
      89,    90,    -1,    92,    -1,    94,    95,    96,    -1,    -1,
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
      88,    89,    90,    -1,    92,    93,    94,    -1,    96,    -1,
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
     195,   196,   197,    -1,    -1,   200,    -1,   202,    -1,    -1,
     205,   206,    -1,   208,   209,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   200,    -1,    10,    11,    12,   205,   206,    -1,
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
      -1,    -1,   188,    -1,    -1,   176,    -1,    -1,    -1,    -1,
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
       7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    -1,    -1,    -1,    35,    -1,
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
      -1,    -1,    -1,    -1,   181,   182,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,   206,
      -1,   208,   209,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
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
     193,   194,   195,   196,   197,    -1,    -1,   200,    -1,    -1,
      -1,    -1,   205,   206,    -1,   208,   209,     3,     4,     5,
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
      -1,   200,    -1,    10,    11,    12,   205,   206,    -1,   208,
     209,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    30,    31,    32,    33,    34,    35,    36,
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
     182,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   205,   206,    -1,   208,   209,     3,     4,
      -1,     6,     7,    11,    12,    10,    11,    12,    13,    -1,
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
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
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
     122,    -1,    -1,    -1,   126,   127,   128,   129,    -1,    -1,
      -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
     152,    10,    11,    12,    13,    -1,   158,   159,    -1,   161,
     162,   163,   164,    -1,   166,    -1,    -1,   169,    27,    -1,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    54,    -1,    56,    -1,    -1,
      -1,   203,    -1,   205,    -1,    -1,    -1,    -1,    -1,    68,
      69,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,    -1,   127,   128,
     129,    78,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,   152,    -1,    -1,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,   164,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,     3,
       4,   200,     6,     7,    -1,   204,    10,    11,    12,    13,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      54,    -1,    56,   200,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,    -1,   127,   128,   129,    78,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,   152,    -1,
      -1,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
     164,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,     3,     4,   200,     6,     7,    -1,
     204,    10,    11,    12,    13,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    54,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      69,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,   126,   127,   128,
     129,    -1,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,    -1,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,   164,    -1,   166,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,     3,
       4,   200,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    29,    29,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    54,
      54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    68,    69,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,    -1,   127,   128,   129,    -1,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,
      -1,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
     164,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,    -1,   176,   177,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,     3,     4,   200,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    29,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    54,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    68,
      69,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,    -1,   127,   128,
     129,    -1,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    -1,   152,    -1,    -1,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,   164,    -1,   166,    -1,    99,
     169,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
      -1,   200,     3,     4,     5,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   152,    -1,    -1,   155,    27,    28,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    54,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    68,    69,    70,
      71,    72,    73,    74,   204,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,   133,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   152,   153,   154,    -1,    -1,    -1,   158,   159,    -1,
     161,   162,   163,   164,    -1,   166,   167,    -1,   169,    -1,
      -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,    -1,
     181,   182,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,     3,     4,    -1,     6,
       7,    -1,    12,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      27,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    -1,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    68,    69,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,   126,
     127,   128,   129,    -1,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,   152,    10,    11,    12,    13,
      -1,   158,   159,    -1,   161,   162,   163,   164,    -1,   166,
      -1,    -1,   169,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
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
     121,   122,    -1,    -1,    -1,    -1,   127,   128,   129,    -1,
      -1,    -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   152,    -1,    -1,    -1,    -1,    -1,   158,   159,    -1,
     161,   162,   163,   164,    -1,   166,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    29,    -1,    31,    32,
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
      29,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,   202,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,    29,    -1,   152,    -1,    -1,   155,   156,   201,   158,
     159,    -1,   161,   162,   163,    -1,   165,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    56,    -1,
      -1,   134,   135,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   196,   152,
      78,   200,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,   165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,   152,    80,    -1,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    30,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    47,
      48,    -1,   200,    -1,    -1,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,   156,    -1,   158,   159,    -1,   161,   162,   163,    87,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,
     205,    -1,    -1,    -1,    -1,    -1,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    35,    -1,   167,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    67,
      -1,    -1,   200,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    99,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    -1,
      -1,   121,    -1,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   134,   135,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    -1,
      -1,    -1,   152,    -1,    -1,   155,   156,    87,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    75,    76,    77,
      78,    -1,    -1,    -1,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    78,    -1,    80,    -1,   147,    -1,    -1,
      -1,    99,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    99,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   176,    47,    48,    -1,
      -1,   181,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    67,    -1,    -1,
     200,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    87,    -1,    -1,
      -1,   156,    -1,   158,   159,    -1,   161,   162,   163,    99,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   134,    -1,    -1,    -1,   203,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
     120,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    67,    -1,
     200,    -1,    -1,   203,    -1,   205,    75,    76,    77,    78,
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
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
      67,   200,    69,    -1,    -1,    -1,   205,    -1,    75,    76,
      77,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,   134,    -1,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   181,    -1,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    67,    -1,   200,    -1,    -1,    -1,    -1,   205,    75,
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
     196,    -1,    67,    -1,   200,    -1,    -1,    -1,    -1,   205,
      75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    80,    81,    -1,    -1,    -1,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    78,    -1,
      -1,    -1,   147,    99,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    99,
      -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,    -1,   200,    75,    76,    77,    78,
     205,    80,   158,   159,    -1,   161,   162,   163,    87,    -1,
      -1,    -1,    -1,    -1,    -1,   155,    -1,    -1,   158,   159,
      99,   161,   162,   163,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   120,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   136,   137,   138,
     139,   140,    -1,   203,    78,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    99,    -1,    -1,   167,    -1,
      -1,    -1,    -1,   107,   108,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
      -1,   200,    75,    76,    77,    78,   205,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      -1,   155,    -1,    -1,   158,   159,    99,   161,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    10,    11,    12,   200,    -1,    -1,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   132,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   132,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   132,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   132,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     132,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   132,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    10,    11,    12,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   132,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    78,    -1,    -1,    -1,    -1,   156,   132,   158,   159,
     160,   161,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   124,    78,    -1,
     200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    -1,   132,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   158,   159,    99,   161,   162,   163,    -1,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,   134,   135,    -1,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      78,    -1,   152,   200,    99,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,    -1,    -1,   152,    -1,    -1,   155,
     156,    99,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   156,    -1,   158,   159,    78,   161,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   155,    -1,    -1,
     158,   159,    78,   161,   162,   163,    99,    -1,    -1,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    99,    -1,    -1,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,   155,    54,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,   155,
      -1,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
      -1,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    10,    11,    12,    -1,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    28,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    98,    54,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    -1,    -1,
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
     120,   133,   152,   162,   200,   217,   250,   329,   348,   445,
     348,   200,   348,   348,   348,   104,   348,   348,   431,   432,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,    80,    87,   120,   147,   200,   227,   367,   403,
     406,   411,   445,   448,   445,    35,   348,   459,   460,   348,
     120,   200,   227,   403,   404,   405,   407,   411,   442,   443,
     444,   452,   456,   457,   200,   339,   408,   200,   339,   355,
     340,   348,   236,   339,   200,   200,   200,   339,   202,   348,
     217,   202,   348,     3,     4,     6,     7,    10,    11,    12,
      13,    27,    29,    54,    56,    68,    69,    70,    71,    72,
      73,    74,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   126,   127,   128,   129,
     133,   134,   135,   152,   156,   164,   166,   169,   176,   200,
     217,   218,   219,   230,   473,   488,   489,   491,   183,   202,
     345,   348,   372,   374,   203,   243,   348,   107,   108,   155,
     220,   223,   226,    80,   205,   295,   296,   119,   126,   118,
     126,    80,   297,   200,   200,   200,   200,   217,   267,   476,
     200,   200,   340,    80,    86,   148,   149,   150,   465,   466,
     155,   203,   226,   226,   217,   268,   476,   156,   200,   476,
     476,    80,   197,   203,   357,    27,   338,   342,   348,   349,
     445,   449,   232,   203,   454,    86,   409,   465,    86,   465,
     465,    30,   155,   172,   477,   200,     9,   202,    35,   249,
     156,   266,   476,   120,   182,   250,   330,   202,   202,   202,
     202,   202,   202,    10,    11,    12,    29,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    54,
      66,   202,    67,    67,   202,   203,   151,   127,   162,   164,
     177,   179,   269,   328,   329,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    64,    65,
     130,   131,   435,    67,   203,   440,   200,   200,    67,   203,
     200,   249,   250,    14,   348,   202,   132,    45,   217,   430,
      86,   338,   349,   151,   445,   132,   207,     9,   416,   338,
     349,   445,   477,   151,   200,   410,   435,   440,   201,   348,
      30,   234,     8,   360,     9,   202,   234,   235,   340,   341,
     348,   217,   281,   238,   202,   202,   202,   134,   135,   491,
     491,   172,   200,   107,   491,    14,   151,   134,   135,   152,
     217,   219,    80,   202,   202,   202,   183,   184,   185,   190,
     191,   194,   195,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   387,   388,   389,   244,   111,   169,   202,   155,
     221,   224,   226,   155,   222,   225,   226,   226,     9,   202,
      98,   203,   445,     9,   202,   126,   126,    14,     9,   202,
     445,   469,   340,   338,   349,   445,   448,   449,   201,   172,
     261,   133,   445,   458,   459,   202,    67,   435,   148,   466,
      79,   348,   445,    86,   148,   466,   226,   216,   202,   203,
     256,   264,   393,   395,    87,   200,   361,   362,   364,   406,
     451,   453,   470,    14,    98,   471,   356,   358,   359,   291,
     292,   433,   434,   201,   201,   201,   201,   201,   204,   233,
     234,   251,   258,   263,   433,   348,   206,   208,   209,   217,
     478,   479,   491,    35,   165,   293,   294,   348,   473,   200,
     476,   259,   249,   348,   348,   348,    30,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     407,   348,   348,   455,   455,   348,   461,   462,   126,   203,
     218,   219,   454,   267,   217,   268,   476,   476,   266,   250,
      35,   342,   345,   348,   372,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   156,   203,   217,
     436,   437,   438,   439,   454,   455,   348,   293,   293,   455,
     348,   458,   249,   201,   348,   200,   429,     9,   416,   201,
     201,    35,   348,    35,   348,   201,   201,   201,   452,   453,
     454,   293,   203,   217,   436,   437,   454,   201,   232,   285,
     203,   345,   348,   348,    90,    30,   234,   279,   202,    28,
      98,    14,     9,   201,    30,   203,   282,   491,    29,    87,
     230,   485,   486,   487,   200,     9,    47,    48,    53,    55,
      67,   134,   156,   176,   200,   227,   228,   230,   369,   403,
     411,   412,   413,   217,   490,   186,    80,   348,    80,    80,
     348,   384,   385,   348,   348,   377,   387,   189,   390,   232,
     200,   242,   226,   202,     9,    98,   226,   202,     9,    98,
      98,   223,   217,   348,   296,   412,    80,     9,   201,   201,
     201,   201,   201,   201,   201,   202,    47,    48,   483,   484,
     128,   272,   200,     9,   201,   201,    80,    81,   217,   467,
     217,    67,   204,   204,   213,   215,    30,   129,   271,   171,
      51,   156,   171,   397,   349,   132,     9,   416,   201,   151,
     491,   491,    14,   360,   291,   232,   198,     9,   417,   491,
     492,   435,   440,   435,   204,     9,   416,   173,   445,   348,
     201,     9,   417,    14,   352,   252,   128,   270,   200,   476,
     348,    30,   207,   207,   132,   204,     9,   416,   348,   477,
     200,   262,   257,   265,    14,   471,   260,   249,    69,   445,
     348,   477,   207,   204,   201,   201,   207,   204,   201,    47,
      48,    67,    75,    76,    77,    87,   134,   147,   176,   217,
     419,   421,   422,   425,   428,   217,   445,   445,   132,   435,
     440,   201,   348,   286,    72,    73,   287,   232,   339,   232,
     341,    98,    35,   133,   276,   445,   412,   217,    30,   234,
     280,   202,   283,   202,   283,     9,   173,    87,   132,   151,
       9,   416,   201,   165,   478,   479,   480,   478,   412,   412,
     412,   412,   412,   415,   418,   200,   151,   200,   412,   151,
     203,    10,    11,    12,    29,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    66,   151,   477,
     348,   186,   186,    14,   192,   193,   386,     9,   196,   390,
      80,   204,   403,   203,   246,    98,   224,   217,    98,   225,
     217,   217,   204,    14,   445,   202,     9,   173,   217,   273,
     403,   203,   458,   133,   445,    14,   207,   348,   204,   213,
     491,   273,   203,   396,    14,   201,   348,   361,   454,   202,
     491,   198,   204,    30,   481,   434,    35,    80,   165,   436,
     437,   439,   436,   437,   491,    35,   165,   348,   412,   291,
     200,   403,   271,   353,   253,   348,   348,   348,   204,   200,
     293,   272,    30,   271,   491,    14,   270,   476,   407,   204,
     200,    14,    75,    76,    77,   217,   420,   420,   422,   423,
     424,    49,   200,    86,   148,   200,     9,   416,   201,   429,
      35,   348,   204,    72,    73,   288,   339,   234,   204,   202,
      91,   202,   276,   445,   200,   132,   275,    14,   232,   283,
     101,   102,   103,   283,   204,   491,   132,   491,   217,   485,
       9,   201,   416,   132,   207,     9,   416,   415,   218,   361,
     363,   365,   201,   126,   218,   412,   463,   464,   412,   412,
     412,    30,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   412,   490,   348,   348,   348,
     385,   348,   375,    80,   247,   217,   217,   412,   484,    98,
      99,   482,     9,   301,   201,   200,   342,   345,   348,   207,
     204,   471,   301,   157,   170,   203,   392,   399,   157,   203,
     398,   132,   202,   481,   491,   360,   492,    80,   165,    14,
      80,   477,   445,   348,   201,   291,   203,   291,   200,   132,
     200,   293,   201,   203,   491,   203,   202,   491,   271,   254,
     410,   293,   132,   207,     9,   416,   421,   423,   148,   361,
     426,   427,   422,   445,   339,    30,    74,   234,   202,   341,
     275,   458,   276,   201,   412,    97,   101,   202,   348,    30,
     202,   284,   204,   173,   491,   132,   165,    30,   201,   412,
     412,   201,   132,     9,   416,   201,   132,   204,     9,   416,
     412,    30,   187,   201,   232,   217,   491,   491,   403,     4,
     108,   113,   119,   121,   158,   159,   161,   204,   302,   327,
     328,   329,   334,   335,   336,   337,   433,   458,   204,   203,
     204,    51,   348,   348,   348,   360,    35,    80,   165,    14,
      80,   348,   200,   481,   201,   301,   201,   291,   348,   293,
     201,   301,   471,   301,   202,   203,   200,   201,   422,   422,
     201,   132,   201,     9,   416,    30,   232,   202,   201,   201,
     201,   239,   202,   202,   284,   232,   491,   491,   132,   412,
     361,   412,   412,   412,   348,   203,   204,   482,   128,   129,
     177,   218,   474,   491,   274,   403,   108,   337,    29,   121,
     134,   135,   156,   162,   311,   312,   313,   314,   403,   160,
     319,   320,   124,   200,   217,   321,   322,   303,   250,   491,
       9,   202,     9,   202,   202,   471,   328,   201,   298,   156,
     394,   204,   204,    80,   165,    14,    80,   348,   293,   113,
     350,   481,   204,   481,   201,   201,   204,   203,   204,   301,
     291,   132,   422,   361,   232,   237,   240,    30,   234,   278,
     232,   201,   412,   132,   132,   188,   232,   403,   403,   476,
      14,   218,     9,   202,   203,   474,   471,   314,   172,   203,
       9,   202,     3,     4,     5,     6,     7,    10,    11,    12,
      13,    27,    28,    54,    68,    69,    70,    71,    72,    73,
      74,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   133,   134,   136,   137,   138,   139,   140,   152,
     153,   154,   164,   166,   167,   169,   176,   177,   179,   181,
     182,   217,   400,   401,     9,   202,   156,   160,   217,   322,
     323,   324,   202,    80,   333,   249,   304,   474,   474,    14,
     250,   204,   299,   300,   474,    14,    80,   348,   201,   200,
     203,   202,   203,   325,   350,   481,   298,   204,   201,   422,
     132,    30,   234,   277,   278,   232,   412,   412,   348,   204,
     202,   202,   412,   403,   307,   491,   315,   316,   411,   312,
      14,    30,    48,   317,   320,     9,    33,   201,    29,    47,
      50,    14,     9,   202,   219,   475,   333,    14,   491,   249,
     202,    14,   348,    35,    80,   391,   232,   232,   203,   325,
     204,   481,   422,   232,    95,   189,   245,   204,   217,   230,
     308,   309,   310,     9,   173,     9,   416,   204,   412,   401,
     401,    56,   318,   323,   323,    29,    47,    50,   412,    80,
     172,   200,   202,   412,   476,   412,    80,     9,   417,   204,
     204,   232,   325,    93,   202,    80,   111,   241,   151,    98,
     491,   411,   163,    14,   483,   305,   200,    35,    80,   201,
     204,   202,   200,   169,   248,   217,   328,   329,   173,   412,
     173,   289,   290,   434,   306,    80,   403,   246,   166,   217,
     202,   201,     9,   417,   115,   116,   117,   331,   332,   289,
      80,   274,   202,   481,   434,   492,   201,   201,   202,   202,
     203,   326,   331,    35,    80,   165,   481,   203,   232,   492,
      80,   165,    14,    80,   326,   232,   204,    35,    80,   165,
      14,    80,   348,   204,    80,   165,    14,    80,   348,    14,
      80,   348,   348
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
#line 876 "hphp.y"
    { ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 894 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 895 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 897 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 913 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->onStatementListStart((yyval));;}
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
#line 966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1022 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1033 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getRange(),
                                                     &(yyval));;}
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
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1061 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1080 "hphp.y"
    { (yyval).reset();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1086 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1092 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1093 "hphp.y"
    { (yyval).reset();;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1097 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1098 "hphp.y"
    { (yyval).reset();;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1102 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1108 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1114 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1121 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1127 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1140 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1148 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1152 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1156 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1160 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1166 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1169 "hphp.y"
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
#line 1184 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
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
#line 1201 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1212 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1230 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { (yyval).reset();;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { (yyval).reset();;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1271 "hphp.y"
    { (yyval).reset();;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1274 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { (yyval).reset();;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1325 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1333 "hphp.y"
    { (yyval).reset();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { (yyval).reset();;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { (yyval).reset();;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1343 "hphp.y"
    { (yyval).reset();;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { (yyval).reset();;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { (yyval).reset();;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { (yyval).reset();;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1371 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1390 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1395 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1428 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1453 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { (yyval).reset();;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1481 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1484 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { (yyval).reset();;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { (yyval).reset();;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { (yyval).reset();;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval).reset();;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { (yyval).reset();;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval).reset();;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval).reset();;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval).reset();;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval).reset();;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
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
#line 1888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { (yyval).reset();;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
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
#line 2018 "hphp.y"
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
#line 2026 "hphp.y"
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
#line 2034 "hphp.y"
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
#line 2043 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
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
#line 2059 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
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
#line 2077 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
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
#line 2097 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
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
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
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
#line 2301 "hphp.y"
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
#line 2312 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval).reset();;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval).reset();;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
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
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
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
#line 2433 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (3)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval).reset();;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { (yyval).reset();;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval).reset();;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { (yyval).reset();;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SPACESHIP);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval).reset();;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval).reset();;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval).reset();;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval).reset();;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2654 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { (yyval).reset();;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2717 "hphp.y"
    { (yyval).reset();;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { (yyval).reset();;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
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
#line 2813 "hphp.y"
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
#line 2828 "hphp.y"
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
#line 2840 "hphp.y"
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
#line 2856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
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
#line 2876 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
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
#line 2887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
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
#line 2905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2908 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2912 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2985 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { (yyval).reset();;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
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
#line 3021 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
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

  case 913:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { (yyval).reset();;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3079 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3087 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3088 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3093 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3097 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3102 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3107 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3112 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3114 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
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

  case 949:

/* Line 1455 of yacc.c  */
#line 3126 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3133 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3138 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3139 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
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
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 960:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 961:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3145 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3148 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3158 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3160 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3174 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3179 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3183 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3188 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3194 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3195 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3199 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3206 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3210 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3220 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 984:

/* Line 1455 of yacc.c  */
#line 3227 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 985:

/* Line 1455 of yacc.c  */
#line 3228 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 986:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 987:

/* Line 1455 of yacc.c  */
#line 3235 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 3241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 989:

/* Line 1455 of yacc.c  */
#line 3246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 990:

/* Line 1455 of yacc.c  */
#line 3247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 991:

/* Line 1455 of yacc.c  */
#line 3248 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 992:

/* Line 1455 of yacc.c  */
#line 3249 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 993:

/* Line 1455 of yacc.c  */
#line 3253 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 994:

/* Line 1455 of yacc.c  */
#line 3254 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 997:

/* Line 1455 of yacc.c  */
#line 3264 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 998:

/* Line 1455 of yacc.c  */
#line 3266 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 999:

/* Line 1455 of yacc.c  */
#line 3270 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 1000:

/* Line 1455 of yacc.c  */
#line 3273 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 1001:

/* Line 1455 of yacc.c  */
#line 3277 "hphp.y"
    {;}
    break;

  case 1002:

/* Line 1455 of yacc.c  */
#line 3278 "hphp.y"
    {;}
    break;

  case 1003:

/* Line 1455 of yacc.c  */
#line 3279 "hphp.y"
    {;}
    break;

  case 1004:

/* Line 1455 of yacc.c  */
#line 3285 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 1005:

/* Line 1455 of yacc.c  */
#line 3290 "hphp.y"
    {
                                     /* should not reach here as
                                      * optional shape fields are not
                                      * supported in strict mode */
                                     validate_shape_keyname((yyvsp[(2) - (4)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));
                                   ;}
    break;

  case 1006:

/* Line 1455 of yacc.c  */
#line 3301 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 1007:

/* Line 1455 of yacc.c  */
#line 3306 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1008:

/* Line 1455 of yacc.c  */
#line 3307 "hphp.y"
    { ;}
    break;

  case 1009:

/* Line 1455 of yacc.c  */
#line 3312 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 1010:

/* Line 1455 of yacc.c  */
#line 3313 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 1011:

/* Line 1455 of yacc.c  */
#line 3319 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 1012:

/* Line 1455 of yacc.c  */
#line 3324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1013:

/* Line 1455 of yacc.c  */
#line 3329 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 1014:

/* Line 1455 of yacc.c  */
#line 3333 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 1015:

/* Line 1455 of yacc.c  */
#line 3340 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1016:

/* Line 1455 of yacc.c  */
#line 3343 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 1017:

/* Line 1455 of yacc.c  */
#line 3346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1018:

/* Line 1455 of yacc.c  */
#line 3347 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1019:

/* Line 1455 of yacc.c  */
#line 3350 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 1020:

/* Line 1455 of yacc.c  */
#line 3353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1021:

/* Line 1455 of yacc.c  */
#line 3356 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 1022:

/* Line 1455 of yacc.c  */
#line 3360 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 1023:

/* Line 1455 of yacc.c  */
#line 3363 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 1024:

/* Line 1455 of yacc.c  */
#line 3366 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 1025:

/* Line 1455 of yacc.c  */
#line 3372 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 1026:

/* Line 1455 of yacc.c  */
#line 3378 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 1027:

/* Line 1455 of yacc.c  */
#line 3386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 1028:

/* Line 1455 of yacc.c  */
#line 3387 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 14553 "hphp.7.tab.cpp"
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
#line 3390 "hphp.y"

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

