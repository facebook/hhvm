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
#define YYLAST   16412

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  196
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  265
/* YYNRULES -- Number of rules.  */
#define YYNRULES  983
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1793

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
    2693,  2697,  2701,  2703,  2705,  2707,  2709,  2711,  2715,  2719,
    2724,  2729,  2733,  2735,  2737,  2745,  2755,  2763,  2770,  2779,
    2781,  2786,  2791,  2793,  2795,  2800,  2803,  2805,  2806,  2808,
    2810,  2812,  2816,  2820,  2824,  2825,  2827,  2829,  2833,  2837,
    2840,  2844,  2851,  2852,  2854,  2859,  2862,  2863,  2869,  2873,
    2877,  2879,  2886,  2891,  2896,  2899,  2902,  2903,  2909,  2913,
    2917,  2919,  2922,  2923,  2929,  2933,  2937,  2939,  2942,  2945,
    2947,  2950,  2952,  2957,  2961,  2965,  2972,  2976,  2978,  2980,
    2982,  2987,  2992,  2997,  3002,  3007,  3012,  3015,  3018,  3023,
    3026,  3029,  3031,  3035,  3039,  3043,  3044,  3047,  3053,  3060,
    3067,  3075,  3077,  3080,  3082,  3085,  3087,  3092,  3094,  3099,
    3103,  3104,  3106,  3110,  3113,  3117,  3119,  3121,  3122,  3123,
    3126,  3129,  3132,  3137,  3140,  3146,  3150,  3152,  3154,  3155,
    3159,  3164,  3170,  3174,  3176,  3179,  3180,  3185,  3187,  3191,
    3194,  3197,  3200,  3202,  3204,  3206,  3208,  3212,  3217,  3224,
    3226,  3235,  3242,  3244
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     197,     0,    -1,    -1,   198,   199,    -1,   199,   200,    -1,
      -1,   219,    -1,   236,    -1,   243,    -1,   240,    -1,   250,
      -1,   440,    -1,   125,   186,   187,   188,    -1,   152,   212,
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
      -1,   213,   445,    -1,   213,   445,    -1,   216,     9,   441,
      14,   380,    -1,   108,   441,    14,   380,    -1,   217,   218,
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
     188,    -1,   122,   186,   437,   187,   188,    -1,   188,    -1,
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
      -1,   107,    -1,    -1,   235,   234,   444,   237,   186,   278,
     187,   449,   312,    -1,    -1,   316,   235,   234,   444,   238,
     186,   278,   187,   449,   312,    -1,    -1,   401,   315,   235,
     234,   444,   239,   186,   278,   187,   449,   312,    -1,    -1,
     162,   203,   241,    30,   459,   439,   189,   285,   190,    -1,
      -1,   401,   162,   203,   242,    30,   459,   439,   189,   285,
     190,    -1,    -1,   256,   253,   244,   257,   258,   189,   288,
     190,    -1,    -1,   401,   256,   253,   245,   257,   258,   189,
     288,   190,    -1,    -1,   127,   254,   246,   259,   189,   288,
     190,    -1,    -1,   401,   127,   254,   247,   259,   189,   288,
     190,    -1,    -1,   126,   249,   378,   257,   258,   189,   288,
     190,    -1,    -1,   164,   255,   251,   258,   189,   288,   190,
      -1,    -1,   401,   164,   255,   252,   258,   189,   288,   190,
      -1,   444,    -1,   156,    -1,   444,    -1,   444,    -1,   126,
      -1,   119,   126,    -1,   119,   118,   126,    -1,   118,   119,
     126,    -1,   118,   126,    -1,   128,   371,    -1,    -1,   129,
     260,    -1,    -1,   128,   260,    -1,    -1,   371,    -1,   260,
       9,   371,    -1,   371,    -1,   261,     9,   371,    -1,   132,
     263,    -1,    -1,   413,    -1,    35,   413,    -1,   133,   186,
     426,   187,    -1,   219,    -1,    30,   217,    93,   188,    -1,
     219,    -1,    30,   217,    95,   188,    -1,   219,    -1,    30,
     217,    91,   188,    -1,   219,    -1,    30,   217,    97,   188,
      -1,   203,    14,   380,    -1,   268,     9,   203,    14,   380,
      -1,   189,   270,   190,    -1,   189,   188,   270,   190,    -1,
      30,   270,   101,   188,    -1,    30,   188,   270,   101,   188,
      -1,   270,   102,   335,   271,   217,    -1,   270,   103,   271,
     217,    -1,    -1,    30,    -1,   188,    -1,   272,    72,   326,
     219,    -1,    -1,   273,    72,   326,    30,   217,    -1,    -1,
      73,   219,    -1,    -1,    73,    30,   217,    -1,    -1,   277,
       9,   402,   318,   460,   165,    80,    -1,   277,     9,   402,
     318,   460,    35,   165,    80,    -1,   277,     9,   402,   318,
     460,   165,    -1,   277,   385,    -1,   402,   318,   460,   165,
      80,    -1,   402,   318,   460,    35,   165,    80,    -1,   402,
     318,   460,   165,    -1,    -1,   402,   318,   460,    80,    -1,
     402,   318,   460,    35,    80,    -1,   402,   318,   460,    35,
      80,    14,   335,    -1,   402,   318,   460,    80,    14,   335,
      -1,   277,     9,   402,   318,   460,    80,    -1,   277,     9,
     402,   318,   460,    35,    80,    -1,   277,     9,   402,   318,
     460,    35,    80,    14,   335,    -1,   277,     9,   402,   318,
     460,    80,    14,   335,    -1,   279,     9,   402,   460,   165,
      80,    -1,   279,     9,   402,   460,    35,   165,    80,    -1,
     279,     9,   402,   460,   165,    -1,   279,   385,    -1,   402,
     460,   165,    80,    -1,   402,   460,    35,   165,    80,    -1,
     402,   460,   165,    -1,    -1,   402,   460,    80,    -1,   402,
     460,    35,    80,    -1,   402,   460,    35,    80,    14,   335,
      -1,   402,   460,    80,    14,   335,    -1,   279,     9,   402,
     460,    80,    -1,   279,     9,   402,   460,    35,    80,    -1,
     279,     9,   402,   460,    35,    80,    14,   335,    -1,   279,
       9,   402,   460,    80,    14,   335,    -1,   281,   385,    -1,
      -1,   335,    -1,    35,   413,    -1,   165,   335,    -1,   281,
       9,   335,    -1,   281,     9,   165,   335,    -1,   281,     9,
      35,   413,    -1,   282,     9,   283,    -1,   283,    -1,    80,
      -1,   191,   413,    -1,   191,   189,   335,   190,    -1,   284,
       9,    80,    -1,   284,     9,    80,    14,   380,    -1,    80,
      -1,    80,    14,   380,    -1,   285,   286,    -1,    -1,   287,
     188,    -1,   442,    14,   380,    -1,   288,   289,    -1,    -1,
      -1,   314,   290,   320,   188,    -1,    -1,   316,   459,   291,
     320,   188,    -1,   321,   188,    -1,   322,   188,    -1,   323,
     188,    -1,    -1,   315,   235,   234,   443,   186,   292,   276,
     187,   449,   313,    -1,    -1,   401,   315,   235,   234,   444,
     186,   293,   276,   187,   449,   313,    -1,   158,   298,   188,
      -1,   159,   306,   188,    -1,   161,   308,   188,    -1,     4,
     128,   371,   188,    -1,     4,   129,   371,   188,    -1,   113,
     261,   188,    -1,   113,   261,   189,   294,   190,    -1,   294,
     295,    -1,   294,   296,    -1,    -1,   215,   151,   203,   166,
     261,   188,    -1,   297,    98,   315,   203,   188,    -1,   297,
      98,   316,   188,    -1,   215,   151,   203,    -1,   203,    -1,
     299,    -1,   298,     9,   299,    -1,   300,   368,   304,   305,
      -1,   156,    -1,    29,   301,    -1,   301,    -1,   134,    -1,
     134,   172,   459,   173,    -1,   134,   172,   459,     9,   459,
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
      14,   380,    -1,   321,     9,   442,    14,   380,    -1,   108,
     442,    14,   380,    -1,   322,     9,   442,    -1,   119,   108,
     442,    -1,   119,   324,   439,    -1,   324,   439,    14,   459,
      -1,   108,   177,   444,    -1,   186,   325,   187,    -1,    69,
     375,   378,    -1,    69,   248,    -1,    68,   335,    -1,   360,
      -1,   355,    -1,   186,   335,   187,    -1,   327,     9,   335,
      -1,   335,    -1,   327,    -1,    -1,    27,    -1,    27,   335,
      -1,    27,   335,   132,   335,    -1,   186,   329,   187,    -1,
     413,    14,   329,    -1,   133,   186,   426,   187,    14,   329,
      -1,    28,   335,    -1,   413,    14,   332,    -1,   133,   186,
     426,   187,    14,   332,    -1,   336,    -1,   413,    -1,   325,
      -1,   417,    -1,   416,    -1,   133,   186,   426,   187,    14,
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
      30,   335,    -1,   436,    -1,    63,   335,    -1,    62,   335,
      -1,    61,   335,    -1,    60,   335,    -1,    59,   335,    -1,
      58,   335,    -1,    57,   335,    -1,    70,   376,    -1,    56,
     335,    -1,   382,    -1,   354,    -1,   353,    -1,   192,   377,
     192,    -1,    13,   335,    -1,   357,    -1,   113,   186,   359,
     385,   187,    -1,    -1,    -1,   235,   234,   186,   339,   278,
     187,   449,   337,   189,   217,   190,    -1,    -1,   316,   235,
     234,   186,   340,   278,   187,   449,   337,   189,   217,   190,
      -1,    -1,   182,    80,   342,   347,    -1,    -1,   182,   183,
     343,   278,   184,   449,   347,    -1,    -1,   182,   189,   344,
     217,   190,    -1,    -1,    80,   345,   347,    -1,    -1,   183,
     346,   278,   184,   449,   347,    -1,     8,   335,    -1,     8,
     332,    -1,     8,   189,   217,   190,    -1,    87,    -1,   438,
      -1,   349,     9,   348,   132,   335,    -1,   348,   132,   335,
      -1,   350,     9,   348,   132,   380,    -1,   348,   132,   380,
      -1,   349,   384,    -1,    -1,   350,   384,    -1,    -1,   176,
     186,   351,   187,    -1,   134,   186,   427,   187,    -1,    67,
     427,   193,    -1,   371,   189,   429,   190,    -1,   371,   189,
     431,   190,    -1,   357,    67,   423,   193,    -1,   358,    67,
     423,   193,    -1,   354,    -1,   438,    -1,   416,    -1,    87,
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
     186,   336,   187,    -1,   372,    -1,   373,   151,   422,    -1,
     372,    -1,   419,    -1,   374,   151,   422,    -1,   371,    -1,
     120,    -1,   424,    -1,   186,   187,    -1,   326,    -1,    -1,
      -1,    86,    -1,   433,    -1,   186,   280,   187,    -1,    -1,
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
      79,    -1,   438,    -1,   379,    -1,   194,   433,   194,    -1,
     195,   433,   195,    -1,   147,   433,   148,    -1,   386,   384,
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
     422,    -1,   203,    -1,   189,   335,   190,    -1,   406,    -1,
     422,    -1,    67,   423,   193,    -1,   189,   335,   190,    -1,
     414,   408,    -1,   186,   325,   187,   408,    -1,   425,   408,
      -1,   186,   325,   187,   408,    -1,   186,   325,   187,   403,
     405,    -1,   186,   336,   187,   403,   405,    -1,   186,   325,
     187,   403,   404,    -1,   186,   336,   187,   403,   404,    -1,
     420,    -1,   370,    -1,   418,    -1,   419,    -1,   409,    -1,
     411,    -1,   413,   403,   405,    -1,   374,   151,   422,    -1,
     415,   186,   280,   187,    -1,   416,   186,   280,   187,    -1,
     186,   413,   187,    -1,   370,    -1,   418,    -1,   419,    -1,
     409,    -1,   413,   403,   405,    -1,   412,    -1,   415,   186,
     280,   187,    -1,   186,   413,   187,    -1,   374,   151,   422,
      -1,   420,    -1,   409,    -1,   370,    -1,   354,    -1,   379,
      -1,   186,   413,   187,    -1,   186,   336,   187,    -1,   416,
     186,   280,   187,    -1,   415,   186,   280,   187,    -1,   186,
     417,   187,    -1,   338,    -1,   341,    -1,   413,   403,   407,
     445,   186,   280,   187,    -1,   186,   325,   187,   403,   407,
     445,   186,   280,   187,    -1,   374,   151,   205,   445,   186,
     280,   187,    -1,   374,   151,   422,   186,   280,   187,    -1,
     374,   151,   189,   335,   190,   186,   280,   187,    -1,   421,
      -1,   421,    67,   423,   193,    -1,   421,   189,   335,   190,
      -1,   422,    -1,    80,    -1,   191,   189,   335,   190,    -1,
     191,   422,    -1,   335,    -1,    -1,   420,    -1,   410,    -1,
     411,    -1,   424,   403,   405,    -1,   373,   151,   420,    -1,
     186,   413,   187,    -1,    -1,   410,    -1,   412,    -1,   424,
     403,   404,    -1,   186,   413,   187,    -1,   426,     9,    -1,
     426,     9,   413,    -1,   426,     9,   133,   186,   426,   187,
      -1,    -1,   413,    -1,   133,   186,   426,   187,    -1,   428,
     384,    -1,    -1,   428,     9,   335,   132,   335,    -1,   428,
       9,   335,    -1,   335,   132,   335,    -1,   335,    -1,   428,
       9,   335,   132,    35,   413,    -1,   428,     9,    35,   413,
      -1,   335,   132,    35,   413,    -1,    35,   413,    -1,   430,
     384,    -1,    -1,   430,     9,   335,   132,   335,    -1,   430,
       9,   335,    -1,   335,   132,   335,    -1,   335,    -1,   432,
     384,    -1,    -1,   432,     9,   380,   132,   380,    -1,   432,
       9,   380,    -1,   380,   132,   380,    -1,   380,    -1,   433,
     434,    -1,   433,    86,    -1,   434,    -1,    86,   434,    -1,
      80,    -1,    80,    67,   435,   193,    -1,    80,   403,   203,
      -1,   149,   335,   190,    -1,   149,    79,    67,   335,   193,
     190,    -1,   150,   413,   190,    -1,   203,    -1,    81,    -1,
      80,    -1,   123,   186,   327,   187,    -1,   124,   186,   413,
     187,    -1,   124,   186,   336,   187,    -1,   124,   186,   417,
     187,    -1,   124,   186,   416,   187,    -1,   124,   186,   325,
     187,    -1,     7,   335,    -1,     6,   335,    -1,     5,   186,
     335,   187,    -1,     4,   335,    -1,     3,   335,    -1,   413,
      -1,   437,     9,   413,    -1,   374,   151,   204,    -1,   374,
     151,   126,    -1,    -1,    98,   459,    -1,   177,   444,    14,
     459,   188,    -1,   401,   177,   444,    14,   459,   188,    -1,
     179,   444,   439,    14,   459,   188,    -1,   401,   179,   444,
     439,    14,   459,   188,    -1,   205,    -1,   459,   205,    -1,
     204,    -1,   459,   204,    -1,   205,    -1,   205,   172,   451,
     173,    -1,   203,    -1,   203,   172,   451,   173,    -1,   172,
     447,   173,    -1,    -1,   459,    -1,   446,     9,   459,    -1,
     446,   384,    -1,   446,     9,   165,    -1,   447,    -1,   165,
      -1,    -1,    -1,    30,   459,    -1,    98,   459,    -1,    99,
     459,    -1,   451,     9,   452,   203,    -1,   452,   203,    -1,
     451,     9,   452,   203,   450,    -1,   452,   203,   450,    -1,
      47,    -1,    48,    -1,    -1,    87,   132,   459,    -1,    29,
      87,   132,   459,    -1,   215,   151,   203,   132,   459,    -1,
     454,     9,   453,    -1,   453,    -1,   454,   384,    -1,    -1,
     176,   186,   455,   187,    -1,   215,    -1,   203,   151,   458,
      -1,   203,   445,    -1,    29,   459,    -1,    56,   459,    -1,
     215,    -1,   134,    -1,   135,    -1,   456,    -1,   457,   151,
     458,    -1,   134,   172,   459,   173,    -1,   134,   172,   459,
       9,   459,   173,    -1,   156,    -1,   186,   107,   186,   448,
     187,    30,   459,   187,    -1,   186,   459,     9,   446,   384,
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
    2618,  2622,  2623,  2627,  2628,  2629,  2633,  2638,  2643,  2644,
    2648,  2653,  2658,  2659,  2663,  2664,  2669,  2671,  2676,  2687,
    2701,  2713,  2728,  2729,  2730,  2731,  2732,  2733,  2734,  2744,
    2753,  2755,  2757,  2761,  2762,  2763,  2764,  2765,  2781,  2782,
    2784,  2786,  2793,  2794,  2795,  2796,  2797,  2798,  2799,  2800,
    2802,  2807,  2811,  2812,  2816,  2819,  2826,  2830,  2839,  2846,
    2854,  2856,  2857,  2861,  2862,  2864,  2869,  2870,  2881,  2882,
    2883,  2884,  2895,  2898,  2901,  2902,  2903,  2904,  2915,  2919,
    2920,  2921,  2923,  2924,  2925,  2929,  2931,  2934,  2936,  2937,
    2938,  2939,  2942,  2944,  2945,  2949,  2951,  2954,  2956,  2957,
    2958,  2962,  2964,  2967,  2970,  2972,  2974,  2978,  2979,  2981,
    2982,  2988,  2989,  2991,  3001,  3003,  3005,  3008,  3009,  3010,
    3014,  3015,  3016,  3017,  3018,  3019,  3020,  3021,  3022,  3023,
    3024,  3028,  3029,  3033,  3035,  3043,  3045,  3049,  3053,  3058,
    3062,  3070,  3071,  3075,  3076,  3082,  3083,  3092,  3093,  3101,
    3104,  3108,  3111,  3116,  3121,  3123,  3124,  3125,  3129,  3130,
    3134,  3135,  3138,  3141,  3143,  3147,  3153,  3154,  3155,  3159,
    3163,  3173,  3181,  3183,  3187,  3189,  3194,  3200,  3203,  3208,
    3216,  3219,  3222,  3223,  3226,  3229,  3230,  3235,  3238,  3242,
    3246,  3252,  3262,  3263
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
     414,   414,   415,   415,   415,   415,   415,   415,   415,   415,
     415,   416,   417,   417,   418,   418,   419,   419,   419,   420,
     421,   421,   421,   422,   422,   422,   423,   423,   424,   424,
     424,   424,   424,   424,   425,   425,   425,   425,   425,   426,
     426,   426,   426,   426,   426,   427,   427,   428,   428,   428,
     428,   428,   428,   428,   428,   429,   429,   430,   430,   430,
     430,   431,   431,   432,   432,   432,   432,   433,   433,   433,
     433,   434,   434,   434,   434,   434,   434,   435,   435,   435,
     436,   436,   436,   436,   436,   436,   436,   436,   436,   436,
     436,   437,   437,   438,   438,   439,   439,   440,   440,   440,
     440,   441,   441,   442,   442,   443,   443,   444,   444,   445,
     445,   446,   446,   447,   448,   448,   448,   448,   449,   449,
     450,   450,   451,   451,   451,   451,   452,   452,   452,   453,
     453,   453,   454,   454,   455,   455,   456,   457,   458,   458,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   460,   460
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
       3,     3,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     1,     1,     7,     9,     7,     6,     8,     1,
       4,     4,     1,     1,     4,     2,     1,     0,     1,     1,
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
       0,   423,     0,   786,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   876,     0,
     864,   668,     0,   674,   675,   676,    22,   733,   853,   150,
     151,   677,     0,   131,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   182,     0,     0,     0,     0,     0,     0,
     389,   390,   391,   394,   393,   392,     0,     0,     0,     0,
     211,     0,     0,     0,   681,   683,   684,   678,   679,     0,
       0,     0,   685,   680,     0,   652,    24,    25,    26,    28,
      27,     0,   682,     0,     0,     0,     0,   686,   395,   521,
       0,   149,   121,     0,   669,     0,     0,     4,   111,   113,
     732,     0,   651,     0,     6,   181,     7,     9,     8,    10,
       0,     0,   387,   434,     0,     0,     0,     0,     0,     0,
       0,   432,   842,   843,   503,   502,   417,   506,     0,   416,
     813,   653,   660,     0,   735,   501,   386,   816,   817,   828,
     433,     0,     0,   436,   435,   814,   815,   812,   849,   852,
     491,   734,    11,   394,   393,   392,     0,     0,    28,     0,
     111,   181,     0,   920,   433,   919,     0,   917,   916,   505,
       0,   424,   429,     0,     0,   474,   475,   476,   477,   500,
     498,   497,   496,   495,   494,   493,   492,   853,   677,   655,
       0,     0,   940,   835,   653,     0,   654,   456,     0,   454,
       0,   880,     0,   742,   415,   664,   201,     0,   940,   414,
     663,   658,     0,   673,   654,   859,   860,   866,   858,   665,
       0,     0,   667,   499,     0,     0,     0,     0,   420,     0,
     129,   422,     0,     0,   135,   137,     0,     0,   139,     0,
      69,    68,    63,    62,    54,    55,    46,    66,    77,     0,
      49,     0,    61,    53,    59,    79,    72,    71,    44,    67,
      86,    87,    45,    82,    42,    83,    43,    84,    41,    88,
      76,    80,    85,    73,    74,    48,    75,    78,    40,    70,
      56,    89,    64,    57,    47,    39,    38,    37,    36,    35,
      34,    58,    90,    92,    51,    32,    33,    60,   973,   974,
      52,   979,    31,    50,    81,     0,     0,   111,    91,   931,
     972,     0,   975,     0,     0,   141,     0,     0,   172,     0,
       0,     0,     0,     0,     0,    94,    99,   300,     0,     0,
     299,     0,   215,     0,   212,   305,     0,     0,     0,     0,
       0,   937,   197,   209,   872,   876,     0,   901,     0,   688,
       0,     0,     0,   899,     0,    16,     0,   115,   189,   203,
     210,   558,   533,     0,   925,   513,   515,   517,   790,   423,
     434,     0,     0,   432,   433,   435,     0,     0,   855,   670,
       0,   671,     0,     0,     0,   171,     0,     0,   117,   291,
       0,    21,   180,     0,   208,   193,   207,   392,   395,   181,
     388,   164,   165,   166,   167,   168,   170,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   864,     0,   163,   857,   857,   886,     0,
       0,     0,     0,     0,     0,     0,     0,   385,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   455,   453,   791,   792,     0,   857,     0,   804,   291,
     291,   857,     0,   872,     0,   181,     0,     0,   143,     0,
     788,   783,   742,     0,   434,   432,     0,   884,     0,   538,
     741,   875,   673,   434,   432,   433,   117,     0,   291,   413,
       0,   806,   666,     0,   121,   251,     0,   520,     0,   146,
       0,     0,   421,     0,     0,     0,     0,     0,   138,   162,
     140,   973,   974,   970,   971,     0,   965,     0,     0,     0,
       0,    65,    30,    52,    29,   932,   169,   142,   121,     0,
     159,   161,     0,     0,    96,   103,     0,     0,    98,   107,
     100,     0,    18,     0,     0,   301,     0,   144,   214,   213,
       0,     0,   145,   921,     0,     0,   434,   432,   433,   436,
     435,     0,   958,   221,     0,   873,     0,     0,   147,     0,
       0,   687,   900,   733,     0,     0,   898,   738,   897,   114,
       5,    13,    14,     0,   219,     0,     0,   526,     0,     0,
     742,     0,     0,   661,   656,   527,     0,     0,     0,     0,
     790,   121,     0,   744,   789,   983,   412,   426,   488,   822,
     841,   126,   120,   122,   123,   124,   125,   386,     0,   504,
     736,   737,   112,   742,     0,   941,     0,     0,     0,   744,
     292,     0,   509,   183,   217,     0,   459,   461,   460,     0,
       0,   457,   458,   462,   464,   463,   479,   478,   481,   480,
     482,   484,   486,   485,   483,   473,   472,   466,   467,   465,
     468,   469,   471,   487,   470,   856,     0,     0,   890,     0,
     742,   924,     0,   923,   940,   819,   199,   191,   205,     0,
     925,   195,   181,     0,   427,   430,   438,   452,   451,   450,
     449,   448,   447,   446,   445,   444,   443,   442,   441,   794,
       0,   793,   796,   818,   800,   940,   797,     0,     0,     0,
       0,     0,     0,     0,     0,   918,   425,   781,   785,   741,
     787,     0,   657,     0,   879,     0,   878,   217,     0,   657,
     863,   862,   849,   852,     0,     0,   793,   796,   861,   797,
     418,   253,   255,   121,   524,   523,   419,     0,   121,   235,
     130,   422,     0,     0,     0,     0,     0,   247,   247,   136,
       0,     0,     0,     0,   963,   742,     0,   947,     0,     0,
       0,     0,     0,   740,     0,   652,     0,     0,   690,   651,
     695,     0,   689,   119,   694,   940,   976,     0,     0,     0,
     104,     0,    19,     0,   108,     0,    20,     0,     0,    93,
     101,     0,   298,   306,   303,     0,     0,   910,   915,   912,
     911,   914,   913,    12,   956,   957,     0,     0,     0,     0,
     872,   869,     0,   537,   909,   908,   907,     0,   903,     0,
     904,   906,     0,     5,     0,     0,     0,   552,   553,   561,
     560,     0,   432,     0,   741,   532,   536,     0,     0,   926,
       0,   514,     0,     0,   948,   790,   277,   982,     0,     0,
     805,     0,   854,   741,   943,   939,   293,   294,   650,   743,
     290,     0,   790,     0,     0,   219,   511,   185,   490,     0,
     541,   542,     0,   539,   741,   885,     0,     0,   291,   221,
       0,   219,     0,     0,   217,     0,   864,   439,     0,     0,
     802,   803,   820,   821,   850,   851,     0,     0,     0,   769,
     749,   750,   751,   758,     0,     0,     0,   762,   760,   761,
     775,   742,     0,   783,   883,   882,     0,   219,     0,   807,
     672,     0,   257,     0,     0,   127,     0,     0,     0,     0,
       0,     0,     0,   227,   228,   239,     0,   121,   237,   156,
     247,     0,   247,     0,     0,   977,     0,     0,     0,   741,
     964,   966,   946,   742,   945,     0,   742,   716,   717,   714,
     715,   748,     0,   742,   740,     0,   535,     0,     0,   892,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   969,   173,
       0,   176,   160,     0,    95,   105,     0,    97,   109,   102,
     302,     0,   922,   148,   958,   938,   953,   220,   222,   312,
       0,     0,   870,     0,   902,     0,    17,     0,   925,   218,
     312,     0,     0,   657,   529,     0,   662,   927,     0,   948,
     518,     0,     0,   983,     0,   282,   280,   796,   808,   940,
     796,   809,   942,     0,     0,   295,   118,     0,   790,   216,
       0,   790,     0,   489,   889,   888,     0,   291,     0,     0,
       0,     0,     0,     0,   219,   187,   673,   795,   291,     0,
     754,   755,   756,   757,   763,   764,   773,     0,   742,     0,
     769,     0,   753,   777,   741,   780,   782,   784,     0,   877,
       0,   795,     0,     0,     0,     0,   254,   525,   132,     0,
     422,   227,   229,   872,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   241,     0,     0,   959,     0,   962,   741,
       0,     0,     0,   692,   741,   739,     0,   730,     0,   742,
       0,   696,   731,   729,   896,     0,   742,   699,   701,   700,
       0,     0,   697,   698,   702,   704,   703,   719,   718,   721,
     720,   722,   724,   726,   725,   723,   712,   711,   706,   707,
     705,   708,   709,   710,   713,   968,     0,   121,   106,   110,
     304,     0,     0,     0,   955,     0,   386,   874,   872,   428,
     431,   437,     0,    15,     0,   386,   564,     0,     0,   566,
     559,   562,     0,   557,     0,   929,     0,   949,   522,     0,
     283,     0,     0,   278,     0,   297,   296,   948,     0,   312,
       0,   790,     0,   291,     0,   847,   312,   925,   312,   928,
       0,     0,     0,   440,     0,     0,   766,   741,   768,   759,
       0,   752,     0,     0,   742,   774,   881,   312,     0,   121,
       0,   250,   236,     0,     0,     0,   226,   152,   240,     0,
       0,   243,     0,   248,   249,   121,   242,   978,   960,     0,
     944,     0,   981,   747,   746,   691,     0,   741,   534,   693,
       0,   540,   741,   891,   728,     0,     0,     0,   952,   950,
     951,   223,     0,     0,     0,   393,   384,     0,     0,     0,
     198,   311,   313,     0,   383,     0,     0,     0,   925,   386,
       0,   905,   308,   204,   555,     0,     0,   528,   516,     0,
     286,   276,     0,   279,   285,   291,   508,   948,   386,   948,
       0,   887,     0,   846,   386,     0,   386,   930,   312,   790,
     844,   772,   771,   765,     0,   767,   741,   776,   386,   121,
     256,   128,   133,   154,   230,     0,   238,   244,   121,   246,
     961,     0,     0,   531,     0,   895,   894,   727,   121,   177,
     954,     0,     0,     0,   933,     0,     0,     0,   224,     0,
     925,     0,   349,   345,   351,   652,    28,     0,   339,     0,
     344,   348,   361,     0,   359,   364,     0,   363,     0,   362,
       0,   181,   315,     0,   317,     0,   318,   319,     0,     0,
     871,     0,   556,   554,   565,   563,   287,     0,     0,   274,
     284,     0,     0,     0,     0,   194,   508,   948,   848,   200,
     308,   206,   386,     0,     0,   779,     0,   202,   252,     0,
       0,   121,   233,   153,   245,   980,   745,     0,     0,     0,
       0,     0,   411,     0,   934,     0,   329,   333,   408,   409,
     343,     0,     0,     0,   324,   616,   615,   612,   614,   613,
     633,   635,   634,   604,   575,   576,   594,   610,   609,   571,
     581,   582,   584,   583,   603,   587,   585,   586,   588,   589,
     590,   591,   592,   593,   595,   596,   597,   598,   599,   600,
     602,   601,   572,   573,   574,   577,   578,   580,   618,   619,
     628,   627,   626,   625,   624,   623,   611,   630,   620,   621,
     622,   605,   606,   607,   608,   631,   632,   636,   638,   637,
     639,   640,   617,   642,   641,   644,   646,   645,   579,   649,
     647,   648,   643,   629,   570,   356,   567,     0,   325,   377,
     378,   376,   369,     0,   370,   326,   403,     0,     0,     0,
       0,   407,     0,   181,   190,   307,     0,     0,     0,   275,
     289,   845,     0,   121,   379,   121,   184,     0,     0,     0,
     196,   948,   770,     0,   121,   231,   134,   155,     0,   530,
     893,   175,   327,   328,   406,   225,     0,     0,   742,     0,
     352,   340,     0,     0,     0,   358,   360,     0,     0,   365,
     372,   373,   371,     0,     0,   314,   935,     0,     0,     0,
     410,     0,   309,     0,   288,     0,   550,   744,     0,     0,
     121,   186,   192,     0,   778,     0,     0,   157,   330,   111,
       0,   331,   332,     0,     0,   346,   741,   354,   350,   355,
     568,   569,     0,   341,   374,   375,   367,   368,   366,   404,
     401,   958,   320,   316,   405,     0,   310,   551,   743,     0,
     510,   380,     0,   188,     0,   234,     0,   179,     0,   386,
       0,   353,   357,     0,     0,   790,   322,     0,   548,   507,
     512,   232,     0,     0,   158,   337,     0,   385,   347,   402,
     936,     0,   744,   397,   790,   549,     0,   178,     0,     0,
     336,   948,   790,   261,   398,   399,   400,   983,   396,     0,
       0,     0,   335,     0,   397,     0,   948,     0,   334,   381,
     121,   321,   983,     0,   266,   264,     0,   121,     0,     0,
     267,     0,     0,   262,   323,     0,   382,     0,   270,   260,
       0,   263,   269,   174,   271,     0,     0,   258,   268,     0,
     259,   273,   272
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   107,   853,   600,   170,  1404,   694,
     334,   553,   557,   335,   554,   558,   109,   110,   111,   112,
     113,   386,   632,   633,   521,   239,  1469,   527,  1385,  1470,
    1707,   809,   329,   548,  1667,  1032,  1207,  1724,   403,   171,
     634,   893,  1092,  1262,   117,   603,   910,   635,   654,   914,
     583,   909,   219,   502,   636,   604,   911,   405,   352,   369,
     120,   895,   856,   839,  1047,  1407,  1145,   963,  1616,  1473,
     770,   969,   526,   779,   971,  1295,   762,   952,   955,  1134,
    1731,  1732,   622,   623,   648,   649,   339,   340,   346,  1441,
    1595,  1596,  1216,  1331,  1430,  1589,  1715,  1734,  1626,  1671,
    1672,  1673,  1417,  1418,  1419,  1420,  1628,  1629,  1635,  1683,
    1423,  1424,  1428,  1582,  1583,  1584,  1606,  1761,  1332,  1333,
     172,   122,  1747,  1748,  1587,  1335,  1336,  1337,  1338,   123,
     232,   522,   523,   124,   125,   126,   127,   128,   129,   130,
     131,  1453,   132,   892,  1091,   133,   619,   620,   621,   236,
     378,   517,   609,   610,  1169,   611,  1170,   134,   135,   136,
     800,   137,   138,  1657,   139,   605,  1443,   606,  1061,   861,
    1233,  1230,  1575,  1576,   140,   141,   142,   222,   143,   223,
     233,   390,   509,   144,   991,   804,   145,   992,   884,   876,
     993,   938,  1114,   939,  1116,  1117,  1118,   941,  1273,  1274,
     942,   738,   492,   183,   184,   637,   625,   475,  1077,  1078,
     724,   725,   880,   147,   225,   148,   149,   174,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   686,   229,   230,
     586,   212,   213,   689,   690,  1175,  1176,   362,   363,   847,
     160,   574,   161,   618,   162,   321,  1597,  1647,   353,   398,
     643,   644,   985,  1072,  1214,   836,   837,   784,   785,   786,
     322,   323,   806,  1406,   878
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1458
static const yytype_int16 yypact[] =
{
   -1458,   177, -1458, -1458,  5182, 12709, 12709,    -4, 12709, 12709,
   12709, 10586, 12709, -1458, 12709, 12709, 12709, 12709, 12709, 12709,
   12709, 12709, 12709, 12709, 12709, 12709, 15265, 15265, 10779, 12709,
   13948,     1,     6, -1458, -1458, -1458, -1458, -1458,   186, -1458,
   -1458,   130, 12709, -1458,     6,    16,    55,    71, -1458,     6,
   10972,  1739, 11165, -1458, 13611,  9621,    46, 12709,   951,   310,
   -1458, -1458, -1458,   239,    57,    69,   183,   187,   202,   225,
   -1458,  1739,   316,   342, -1458, -1458, -1458, -1458, -1458, 12709,
     621,   876, -1458, -1458,  1739, -1458, -1458, -1458, -1458,  1739,
   -1458,  1739, -1458,   362,   353,  1739,  1739, -1458,   393, -1458,
   11358, -1458, -1458,   390,   726,   783,   783, -1458,   185,   240,
      12,   358, -1458,    85, -1458,   520, -1458, -1458, -1458, -1458,
    1112,   503, -1458, -1458,   401,   416,   420,   423,   426,   428,
   10957, -1458, -1458, -1458, -1458,   147, -1458,   564,   579, -1458,
     160,   490, -1458,   530,    25, -1458,  3028,   165, -1458, -1458,
    1377,   152,   500,   149, -1458,   158,    92,   513,   191, -1458,
   -1458,   622, -1458, -1458, -1458,   553,   537,   571, -1458, 12709,
   -1458,   520,   503, 15955,  2991, 15955, 12709, 15955, 15955, 16203,
     560,  4672, 16203,   709,  1739,   699,   699,   472,   699,   699,
     699,   699,   699,   699,   699,   699,   699, -1458, -1458, -1458,
      56, 12709,   596, -1458, -1458,   633,   603,   540,   605,   540,
   15265, 14841,   589,   784, -1458,   553, -1458, 12709,   596, -1458,
     657, -1458,   660,   649, -1458,   163, -1458, -1458, -1458,   540,
     152, 11551, -1458, -1458, 12709,  8463,   809,    87, 15955,  9428,
   -1458, 12709, 12709,  1739, -1458, -1458, 11536,   651, -1458, 12694,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,  3078,
   -1458,  3078, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,    76,    78,
     571, -1458, -1458, -1458, -1458,   655,  2181,    84, -1458, -1458,
     686,   834, -1458,   704, 14406, -1458,   674, 14214, -1458,    59,
   14259,  1414,  1785,  1739,    98, -1458,    48, -1458,  3601,   101,
   -1458,   740, -1458,   744, -1458,   863,   102, 15265, 12709, 12709,
     692,   711, -1458, -1458, 14982, 10779,   104,    91,   418, -1458,
   12902, 15265,   631, -1458,  1739, -1458,    35,   240, -1458, -1458,
   -1458, -1458, 15656,   874,   793, -1458, -1458, -1458,    66, 12709,
     706,   714, 15955,   715,  1722,   724,  5375, 12709, -1458,   434,
     721,   785,   434,   527,   477, -1458,  1739,  3078,   712,  9814,
   13611, -1458, -1458,   642, -1458, -1458, -1458, -1458, -1458,   520,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, 12709, 12709, 12709,
   11744, 12709, 12709, 12709, 12709, 12709, 12709, 12709, 12709, 12709,
   12709, 12709, 12709, 12709, 12709, 12709, 12709, 12709, 12709, 12709,
   12709, 12709, 12709, 15749, 12709, -1458, 12709, 12709, 12709, 13064,
    1739,  1739,  1739,  1739,  1739,  1112,   835,  1399,  4495, 12709,
   12709, 12709, 12709, 12709, 12709, 12709, 12709, 12709, 12709, 12709,
   12709, -1458, -1458, -1458, -1458,   662, 12709, 12709, -1458,  9814,
    9814, 12709, 12709, 14982,   731,   520, 11937,  4724, -1458, 12709,
   -1458,   758,   938,   802,   775,   776, 13197,   540, 12130, -1458,
   12323, -1458,   649,   781,   791,  2002, -1458,   328,  9814, -1458,
     760, -1458, -1458, 14304, -1458, -1458, 10007, -1458, 12709, -1458,
     896,  8656,   979,   801, 15910,   978,    90,    67, -1458, -1458,
   -1458,   824, -1458, -1458, -1458,  3078,  2162,   811,   990, 14889,
    1739, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,   814,
   -1458, -1458,  1739,   108, -1458,   437,  1739,   111, -1458,   444,
     447,  1797, -1458,  1739, 12709,   540,   310, -1458, -1458, -1458,
   14889,   921, -1458,   540,    97,   125,   815,   825,  2201,   334,
     826,   827,   517,   886,   832,   540,   132,   833, -1458,  1341,
    1739, -1458, -1458,   952,  2406,    43, -1458, -1458, -1458,   240,
   -1458, -1458, -1458,   991,   894,   865,   356,   879, 12709,   908,
    1032,   856,   915, -1458,   175, -1458,  3078,  3078,  1031,   809,
      66, -1458,   884,  1038, -1458,  3078,    58, -1458,   439,   171,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458,  2243,  2528, -1458,
   -1458, -1458, -1458,  1063,   904, -1458, 15265, 12709,   891,  1071,
   15955,  1070, -1458, -1458,   963,   668, 11150, 16126, 16203, 12709,
   12887,  3354,  2987,  9794, 10758, 12108, 12301, 12301, 12301, 12301,
    3474,  3474,  3474,  3474,  3474,  2068,  2068,   907,   907,   907,
     472,   472,   472, -1458,   699, 15955,   903,   905, 14958,   917,
    1096,   408, 12709,   458,   596,   167, -1458, -1458, -1458,  1101,
     793, -1458,   520, 15079, -1458, -1458, 16203, 16203, 16203, 16203,
   16203, 16203, 16203, 16203, 16203, 16203, 16203, 16203, 16203, -1458,
   12709,   506, -1458,   179, -1458,   596,   522,   918,  2698,   929,
     930,   926,  2744,   134,   934, -1458, 15955,  2544, -1458,  1739,
   -1458,    58,    31, 15265, 15955, 15265, 15426,   963,    58,   540,
     296, -1458,   175,   971,   940, 12709, -1458,   303, -1458, -1458,
   -1458,  8270,   661, -1458, -1458, 15955, 15955,     6, -1458, -1458,
   -1458, 12709,  1030,  4136, 14889,  1739,  8849,   941,   943, -1458,
     109,  1046,  1002,   989, -1458,  1132,   957,  1895,  3078, 14889,
   14889, 14889, 14889, 14889,   960,   996,   962, 14889,    21,   998,
   -1458,   961, -1458, 16043, -1458,   230, -1458,  5568,  1731,   964,
     460,  1414, -1458,  1739,   479,  1785, -1458,  1739,  1739, -1458,
   -1458,  3248, -1458, 16043,  1138, 15265,   966, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458,   110,  1739,  1731,   969,
   14982, 15172,  1141, -1458, -1458, -1458, -1458,   975, -1458, 12709,
   -1458, -1458,  4790, -1458,  3078,  1731,   980, -1458, -1458, -1458,
   -1458,  1150,   983, 12709, 15656, -1458, -1458, 13064,   984, -1458,
    3078, -1458,   992,  5761,  1143,   153, -1458, -1458,    86,   662,
   -1458,   760, -1458,  3078, -1458, -1458,   540, 15955, -1458, 10200,
   -1458, 14889,    81,   988,  1731,   894, -1458, -1458, 16275, 12709,
   -1458, -1458, 12709, -1458, 12709, -1458,  3446,   993,  9814,   886,
    1148,   894,  3078,  1166,   963,  1739, 15749,   540,  3571,   995,
   -1458, -1458,   173,   997, -1458, -1458,  1171,  1219,  1219,  2544,
   -1458, -1458, -1458,  1137,  1003,    70,  1005, -1458, -1458, -1458,
   -1458,  1185,  1013,   758,   540,   540, 12516,   894,   760, -1458,
   -1458,  3774,   665,     6,  9428, -1458,  5954,  1014,  6147,  1020,
    4136, 15265,  1023,  1078,   540, 16043,  1198, -1458, -1458, -1458,
   -1458,   673, -1458,    65,  3078, -1458,  1082,  3078,  1739,  2162,
   -1458, -1458, -1458,  1207, -1458,  1035,  1063,   677,   677,  1159,
    1159, 15528,  1033,  1221, 14889, 14672, 15656, 14349, 14539, 14889,
   14889, 14889, 14889, 14772, 14889, 14889, 14889, 14889, 14889, 14889,
   14889, 14889, 14889, 14889, 14889, 14889, 14889, 14889, 14889, 14889,
   14889, 14889, 14889, 14889, 14889, 14889, 14889,  1739, -1458, -1458,
    1152, -1458, -1458,  1739, -1458, -1458,  1739, -1458, -1458, -1458,
   -1458, 14889,   540, -1458,   517, -1458,   646,  1224, -1458, -1458,
     135,  1042,   540, 10393, -1458,  2136, -1458,  4989,   793,  1224,
   -1458,   467,   352, -1458, 15955,  1102,  1049, -1458,  1050,  1143,
   -1458,  3078,   809,  3078,    74,  1229,  1164,   305, -1458,   596,
     308, -1458, -1458, 15265, 12709, 15955, 16043,  1059,    81, -1458,
    1060,    81,  1065, 16275, 15955, 15471,  1066,  9814,  1067,  1064,
    3078,  1072,  1074,  3078,   894, -1458,   649,   538,  9814, 12709,
   -1458, -1458, -1458, -1458, -1458, -1458,  1131,  1083,  1255,  1178,
    2544,  1129, -1458, 15656,  2544, -1458, -1458, -1458, 15265, 15955,
    1090, -1458,     6,  1250,  1208,  9428, -1458, -1458, -1458,  1093,
   12709,  1078,   540, 14982,  4136,  1098, 14889,  6340,   713,  1099,
   12709,   127,   105, -1458,  1125,  3078, -1458,  1167, -1458,  2806,
    1270,  1114, 14889, -1458, 14889, -1458,  1117, -1458,  1173,  1297,
    1121, -1458, -1458, -1458, 15573,  1126,  1311, 16085, 16167, 16239,
   14889, 16000, 16346,  4424, 10180, 11916, 12494, 13062, 13062, 13062,
   13062,  3311,  3311,  3311,  3311,  3311,  1495,  1495,   677,   677,
     677,  1159,  1159,  1159,  1159, -1458,  1135, -1458, -1458, -1458,
   16043,  1739,  3078,  3078, -1458,  1731,   532, -1458, 14982, -1458,
   -1458, 16203,  1133, -1458,  1136,   936, -1458,    88, 12709, -1458,
   -1458, -1458, 12709, -1458, 12709, -1458,   809, -1458, -1458,   174,
    1312,  1247, 12709, -1458,  1142,   540, 15955,  1143,  1144, -1458,
    1145,    81, 12709,  9814,  1153, -1458, -1458,   793, -1458, -1458,
    1147,  1154,  1155, -1458,  1158,  2544, -1458,  2544, -1458, -1458,
    1162, -1458,  1214,  1170,  1321, -1458,   540, -1458,  1328, -1458,
    1163, -1458, -1458,  1181,  1183,   136, -1458, -1458, 16043,  1172,
    1184, -1458, 10571, -1458, -1458, -1458, -1458, -1458, -1458,  3078,
   -1458,  3078, -1458, 16043, 15630, -1458, 14889, 15656, -1458, -1458,
   14889, -1458, 14889, -1458, 16311, 14889,  1182,  6533,   646, -1458,
   -1458, -1458,   634, 13749,  1731,  1265, -1458,  1598,  1215,  1097,
   -1458, -1458, -1458,   835,  3894,   113,   114,  1191,   793,  1399,
     144, -1458, -1458, -1458,  1230,  3868,  4390, 15955, -1458,    80,
    1373,  1308, 12709, -1458, 15955,  9814,  1277,  1143,  1194,  1143,
    1222, 15955,  1223, -1458,  1246,  1225,  1370, -1458, -1458,    81,
   -1458, -1458,  1276, -1458,  2544, -1458, 15656, -1458,  1545, -1458,
    8270, -1458, -1458, -1458, -1458,  9042, -1458, -1458, -1458,  8270,
   -1458,  1226, 14889, 16043,  1285, 16043, 15675, 16311, -1458, -1458,
   -1458,  1731,  1731,  1739, -1458,  1404, 14672,    82, -1458, 13749,
     793,  2524, -1458,  1251, -1458,   119,  1235,   121, -1458, 14054,
   -1458, -1458, -1458,   122, -1458, -1458,   597, -1458,  1237, -1458,
    1347,   520, -1458, 13887, -1458, 13887, -1458, -1458,  1415,   835,
   -1458, 13335, -1458, -1458, -1458, -1458,  1416,  1351, 12709, -1458,
   15955,  1248,  1252,  1244,   598, -1458,  1277,  1143, -1458, -1458,
   -1458, -1458,  1810,  1257,  2544, -1458,  1307, -1458,  8270,  9235,
    9042, -1458, -1458, -1458,  8270, -1458, 16043, 14889, 14889,  6726,
    1259,  1261, -1458, 14889, -1458,  1731, -1458, -1458, -1458, -1458,
   -1458,  3078,  1956,  1598, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458,   625, -1458,  1215, -1458, -1458,
   -1458, -1458, -1458,    75,   502, -1458,  1431,   123, 14406,  1347,
    1437, -1458,  3078,   520, -1458, -1458,  1267,  1444, 12709, -1458,
   15955, -1458,   120, -1458, -1458, -1458, -1458,  1271,   598, 13473,
   -1458,  1143, -1458,  2544, -1458, -1458, -1458, -1458,  6919, 16043,
   16043, -1458, -1458, -1458, 16043, -1458,   912,   115,  1450,  1272,
   -1458, -1458, 14889, 14054, 14054,  1407, -1458,   597,   597,   588,
   -1458, -1458, -1458, 14889,  1385, -1458,  1294,  1282,   128, 14889,
   -1458,  1739, -1458, 14889, 15955,  1389, -1458,  1461,  7112,  7305,
   -1458, -1458, -1458,   598, -1458,  7498,  1284,  1362, -1458,  1379,
    1324, -1458, -1458,  1382,  3078, -1458,  1956, -1458, -1458, 16043,
   -1458, -1458,  1318, -1458,  1449, -1458, -1458, -1458, -1458, 16043,
    1479,   517, -1458, -1458, 16043,  1315, 16043, -1458,   151,  1310,
   -1458, -1458,  7691, -1458,  1306, -1458,  1323,  1329,  1739,  1399,
    1337, -1458, -1458, 14889,   141,   106, -1458,  1432, -1458, -1458,
   -1458, -1458,  1731,   964, -1458,  1357,  1739,   789, -1458, 16043,
   -1458,  1340,  1524,   810,   106, -1458,  1454, -1458,  1731,  1353,
   -1458,  1143,   139, -1458, -1458, -1458, -1458,  3078, -1458,  1348,
    1352,   129, -1458,   607,   810,   214,  1143,  1361, -1458, -1458,
   -1458, -1458,  3078,   365,  1537,  1474,   607, -1458,  7884,   326,
    1542,  1478, 12709, -1458, -1458,  8077, -1458,   410,  1549,  1485,
   12709, -1458, 15955, -1458,  1552,  1487, 12709, -1458, 15955, 12709,
   -1458, 15955, 15955
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1458, -1458, -1458,  -523, -1458, -1458, -1458,   209,     0,   -33,
   -1458, -1458, -1458,  1007,   748,   759,    45,  1510, -1458,  2553,
   -1458,  -443, -1458,    26, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458, -1458, -1458,  -145, -1458, -1458,  -140,   131,
      28, -1458, -1458, -1458, -1458, -1458, -1458,    29, -1458, -1458,
   -1458, -1458, -1458, -1458,    44, -1458, -1458,  1130,  1134,  1128,
    -100,  -661,  -807,   678,   735,  -152,   451,  -877, -1458,   133,
   -1458, -1458, -1458, -1458,  -696,   309, -1458, -1458, -1458, -1458,
    -139, -1458,  -584, -1458,  -440, -1458, -1458,  1034, -1458,   137,
   -1458, -1458,  -971, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458,   116, -1458,   195, -1458, -1458, -1458, -1458,
   -1458,    30, -1458,   285,  -805, -1458, -1457,  -150, -1458,  -138,
     332,  -119,  -137, -1458,    32, -1458, -1458, -1458,   294,   -22,
      11,    23,  -671,   -66, -1458, -1458,   -11, -1458, -1458,    -5,
     -36,   164, -1458, -1458, -1458, -1458, -1458, -1458, -1458, -1458,
   -1458,  -562,  -791, -1458, -1458, -1458, -1458, -1458,   378, -1458,
   -1458, -1458, -1458, -1458,   561, -1458, -1458, -1458, -1458, -1458,
   -1458, -1458, -1458,  -761, -1458,  2058,    36, -1458,   859,  -382,
   -1458, -1458,  -473,  3101,  3357, -1458, -1458,   629,  -144,  -624,
   -1458, -1458,   697,   507,  -674,   509, -1458, -1458, -1458, -1458,
   -1458,   687, -1458, -1458, -1458,    68,  -847,  -188,  -395,  -394,
   -1458,   754,  -116, -1458, -1458,    37,    40,   496, -1458, -1458,
      49,   -21, -1458,  -314,     8,  -316,    33,   222, -1458, -1458,
    -441,  1279, -1458, -1458, -1458, -1458, -1458,   861,   614, -1458,
   -1458, -1458,  -313,  -591, -1458,  1236,  -835, -1458,   -69,  -169,
     143,   850, -1458,  -968,   320,   -50,   601,   669, -1458, -1458,
   -1458, -1458,   620,   312, -1011
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -968
static const yytype_int16 yytable[] =
{
     173,   175,   410,   177,   178,   179,   181,   182,   456,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   319,   370,   211,   214,   890,   373,   374,  1073,   747,
     114,   484,   116,   118,   381,   478,   872,   238,   228,   729,
     730,   510,   733,   235,   326,   246,   455,   249,   119,   506,
     327,   406,   330,   410,   318,   240,   614,   871,   613,   615,
     244,   683,  1239,   940,   383,   237,   221,   226,   754,   501,
     227,   761,   146,  1065,   238,   208,   208,   852,   380,   385,
     722,   723,   973,  1141,  1637,   -65,   947,   -30,  1090,  1225,
     -65,  1485,   -30,   -29,   400,   382,   518,   777,   -29,   775,
     959,  1236,   356,   336,  1101,   807,   825,   561,  1638,   913,
     566,   571,    13,   518,   511,   757,   758,   811,   974,  1044,
     815,  1074,  1433,  1435,  1674,   476,   366,    13,  -342,   367,
    1493,  1577,  1644,   383,   518,   115,   388,  1644,  1485,  1344,
    1130,   841,   493,   841,   841,   841,   563,   380,   385,   345,
    1044,  1661,    13,   841,  1240,  1655,  1121,  1293,   589,  -825,
    1446,   473,   474,  -940,   382,   495,  1075,  1150,  1151,   590,
     549,   487,  -940,   473,   474,   343,  -654,     3,   873,   494,
     385,   504,   176,   344,   397,    13,  1717,   231,   473,   474,
     396,   752,   234,   397,  -519,   503,   382,  -546,  -940,    13,
    1656,  -940,   241,   396,   359,  1168,  1703,  1150,  1151,  1349,
    -940,  -836,   382,   108,  -543,   395,  -545,  -838,  1122,   476,
    -655,   473,   474,   601,   602,  -824,   513,  -823,   550,   513,
    -865,  1718,  -826,   851,  -831,   328,   238,   524,  -830,  1241,
    -829,   242,   481,  -661,   371,  1447,  -827,   477,   535,  1763,
    -281,  1076,   409,  1104,  1350,  1153,   778,   243,   481,   208,
     247,   515,  1639,   317,   -65,   520,   -30,  1286,  -281,   655,
    1486,  1487,   -29,   401,  1148,   519,  1152,   776,  1358,  1356,
     351,  -825,   975,  1045,   826,  1364,   562,  1366,  1675,   567,
     572,   545,   588,  -265,  1764,  1296,   812,  1261,   368,   816,
     351,  1434,  1436,   485,   351,   351,  1378,  -342,  1087,  1494,
    1578,  1645,   827,   577,  1730,  1294,  1693,  1758,  -662,   842,
     956,   926,  1217,  1384,   318,   958,  -743,   576,   580,   351,
    1057,  1440,  1272,  -835,   653,   480,   121,  -743,   410,  1351,
    -743,   477,  -656,   238,   382,   734,  -834,  -824,   740,  -823,
     211,  -833,  -865,   908,  -826,   594,  -831,  -837,   341,  -840,
    -830,  1777,  -829,  -868,   482,   342,   324,   319,  -827,   347,
    -867,   575,  -810,   348,   181,  -811,   555,   559,   560,  1765,
     482,  1027,   638,   370,   699,   700,   406,   208,   349,  1454,
     337,  1456,   704,   491,   650,   396,   208,  1462,   579,  1050,
     318,  -545,   397,   208,   203,   203,  1778,   858,   197,   599,
     208,   350,   656,   657,   658,   660,   661,   662,   663,   664,
     665,   666,   667,   668,   669,   670,   671,   672,   673,   674,
     675,   676,   677,   678,   679,   680,   681,   682,   879,   684,
     881,   685,   685,   688,   108,  1770,   624,   705,   108,   693,
    1275,   228,   525,   706,   707,   708,   709,   710,   711,   712,
     713,   714,   715,   716,   717,   718,   865,  1224,  1098,  1283,
     197,   685,   728,   375,   650,   650,   685,   732,   457,   221,
     226,   706,   695,   227,   736,  -868,  1080,  1081,  1405,  1608,
    1784,  1779,  -867,   744,  -810,   746,   318,  -811,   357,   456,
     150,   338,   354,   650,  1248,   764,  -547,  1250,   726,  1231,
    1238,   765,   859,   766,   357,   751,  1394,   115,   371,   103,
     480,   831,   207,   209,  1147,   907,   443,   860,   355,   695,
    1771,  1640,   208,   544,  1106,   813,  1322,   455,   444,   372,
     753,  1232,   817,   759,   399,   818,   905,   769,   614,  1641,
     613,   615,  1642,   879,   881,   402,   919,   357,  1033,   821,
     948,   881,   915,   596,   834,   835,   591,   360,   361,   473,
     474,   533,   862,   534,  1488,  1785,   376,  1036,    13,   387,
     -92,   103,   377,   360,   361,  1466,   897,   702,   203,   411,
    -657,  1371,   396,  1372,   -92,   108,   384,   810,  1590,   396,
    1591,   814,   396,   382,   412,   642,   336,   357,   413,   317,
      53,   414,   351,   596,   415,   396,   416,  1686,    60,    61,
      62,   163,   164,   407,  1226,  -838,   360,   361,   538,   506,
     -91,   446,   949,  1263,   396,  1687,  1028,  1227,  1688,  1632,
    1323,   980,   887,  1663,   -91,  1324,   447,    60,    61,    62,
     163,  1325,   407,  1326,   898,  1633,  1228,  1254,   544,   351,
     697,   351,   351,   351,   351,   384,  1365,  1360,  1264,   687,
     473,   474,   641,  1634,  1348,    36,   360,   361,  -798,   448,
     614,   449,   613,   615,   721,   408,   479,   906,   624,  -544,
    1327,  1328,  -798,  1329,  -801,   208,    48,   384,   727,  -832,
    1465,   357,  1285,   731,  -655,   544,   497,   358,  -801,   645,
    -799,   357,   324,   505,   408,   918,   203,   596,   121,   756,
      36,   640,  1330,   483,  -799,   203,   364,  1023,  1024,  1025,
     108,   150,   203,   953,   954,   150,  1755,  1132,  1133,   203,
      36,    48,   197,  1026,  1212,  1213,    36,  1438,   488,   805,
     951,  1769,   208,  1579,   490,    86,    87,  1580,    88,   168,
      90,    48,  1401,  1402,  1317,   444,   238,    48,   397,   359,
     360,   361,   820,  1753,  1149,  1150,  1151,  1340,   957,   597,
     360,   361,   499,  1426,   496,  1463,  1604,  1605,  1766,  -836,
    1612,   480,   208,   500,   208,  1759,  1760,  1125,   846,   848,
      86,    87,   968,    88,   168,    90,   357,   614,  -653,   613,
     615,   507,   389,  1362,  1290,  1150,  1151,   516,   719,  1489,
      86,    87,   208,    88,   168,    90,    86,    87,   652,    88,
     168,    90,  1684,  1685,   565,   508,  1380,  -967,    36,   529,
     197,   536,  1161,   573,  1055,   578,  1105,   780,   539,  1165,
     585,   720,  1389,   103,   896,   540,   555,   595,  1064,    48,
     559,   203,   546,   357,   351,   357,   568,   693,  1733,   392,
     569,   596,  1680,  1681,   208,   360,   361,   570,   114,   581,
     116,   118,   150,   582,  1085,   205,   205,  1733,   616,   208,
     208,   617,   115,   626,  1093,  1754,   119,  1094,  -116,  1095,
    1066,   627,   628,   650,    60,    61,    62,   163,   164,   407,
    1244,   630,   726,   639,   759,  1451,   719,   652,    86,    87,
     146,    88,   168,    90,   228,  1744,  1745,  1746,   868,   869,
     983,   986,   360,   361,   360,   361,  1468,   877,   115,  1664,
    1322,  1129,    53,   624,   737,  1474,   937,   739,   943,   755,
     591,   103,   221,   226,    36,  1479,   227,   440,   441,   442,
     624,   443,   741,   742,  1135,   391,   393,   394,   748,   457,
     108,   408,   592,   444,  1268,    48,   598,  1740,   749,   585,
    1136,   759,    13,   115,   966,   108,   767,  1219,   518,   771,
      36,   614,   774,   613,   615,  1167,   535,   787,  1173,   788,
     808,   824,   828,   592,   115,   598,   592,   598,   598,   208,
     208,    48,   829,   832,   838,   833,   108,   150,   840,   849,
     843,   854,  1035,   855,   203,  1308,  1038,  1039,  1618,    36,
    -677,   364,  1313,  1699,    86,    87,   857,    88,   168,    90,
     863,   864,  1220,   866,  1323,   870,  1046,   875,  1221,  1324,
      48,    60,    61,    62,   163,  1325,   407,  1326,   331,   332,
     614,   108,   613,   615,   167,   365,   867,    84,   874,   205,
      86,    87,   883,    88,   168,    90,   544,   885,   888,  1246,
     889,   203,   108,   114,   891,   116,   118,   115,   721,   115,
     756,   894,   650,   121,  1327,  1328,   900,  1329,   901,   645,
     645,   119,  1668,   650,  1221,   904,   333,   903,  1743,    86,
      87,   920,    88,   168,    90,   912,   922,   923,   408,   924,
     896,   203,  -659,   203,   351,   146,  1343,   950,   960,   970,
    1377,   972,   208,   976,   977,   238,  1113,  1113,   937,   121,
     978,   979,   886,  1278,   981,  1292,   994,   995,   996,   998,
     999,   203,  1041,  1031,  1043,  1053,   624,   756,  1049,   624,
    1658,  1281,  1659,   108,  1062,   108,  1058,   108,  1054,  1060,
    1063,  1665,  1067,  1071,  1088,    36,  1069,   208,  1100,  1097,
    1103,  1108,  1068,  -839,   121,  1109,  1119,  1157,   115,  1120,
      36,  1123,   208,   208,  1124,  1082,    48,   205,  1322,   917,
    1126,  1439,  1138,   203,   544,   121,   205,   544,  1140,  1143,
    1144,    48,  1146,   205,  1155,   410,  1159,  1702,   203,   203,
     205,  1425,  1160,  1345,  1102,  1026,  1163,  1346,  1218,  1347,
    1164,   612,  1206,  1215,  1234,   908,   805,  1354,  1235,   944,
      13,   945,  1208,  1242,  1243,  1209,  1247,  1361,   650,  1249,
    1322,  1251,  1253,  1256,  1255,    86,    87,   150,    88,   168,
      90,  1258,  1259,  1265,  1267,   933,   108,   208,   404,   964,
      86,    87,   150,    88,   168,    90,  1266,  1271,   115,  1277,
    1279,  1282,  1280,  1426,  1339,  1287,  1154,  1291,   121,  1156,
     121,  1588,    13,  1339,  1110,  1111,  1112,    36,  1297,  1299,
    1301,  1302,  1323,   150,  1305,  1306,  1307,  1324,  1309,    60,
      61,    62,   163,  1325,   407,  1326,  1311,  1768,    48,   624,
    1312,  1042,  1316,  1341,  1775,  1342,  1352,  1353,  1355,   937,
    1376,  1357,  1359,   937,  1482,  1367,   585,  1052,   203,   203,
    1363,  1369,   205,  1368,   108,  1370,  1374,  1450,   150,  1373,
     650,  1381,  1327,  1328,  1323,  1329,   108,  1375,  1379,  1324,
    1386,    60,    61,    62,   163,  1325,   407,  1326,  1382,   150,
    1383,  1398,  1387,  1409,  1322,  1422,   408,    86,    87,  1437,
      88,   168,    90,  1237,  1455,   877,  1442,  1448,  1449,   121,
    1452,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,  1327,  1328,  1484,  1329,  1464,  1457,
    1458,  1472,  1257,  1475,  1460,  1260,    13,  1477,  1483,    36,
    1318,   844,   845,  1491,  1492,  1585,  1339,  1586,   408,  1592,
    1598,  1599,  1339,  1603,  1339,  1601,  1459,   624,  1602,  1613,
      48,   471,   472,  1600,  1611,  1643,  1339,  1622,   115,  1623,
     150,  1649,   150,  1651,   150,  1652,   964,  1142,  1653,  1676,
    1660,   203,  1678,  1682,  1431,  1690,  1691,  1298,  1692,  1697,
    1698,  1082,  1705,  1706,   937,  1708,   937,  -338,  1323,   121,
    1709,  1712,  1638,  1324,  1677,    60,    61,    62,   163,  1325,
     407,  1326,    36,  1713,  1721,  1615,  1472,  1719,  1723,    86,
      87,  1716,    88,   168,    90,   205,   203,   473,   474,  1722,
    1728,   115,  1735,    48,    60,    61,    62,   163,   164,   407,
     115,   203,   203,  1738,  1319,  1320,   108,  1741,  1327,  1328,
    1339,  1329,   317,  1742,  1750,  1756,   202,   202,  1427,  1757,
     218,  1752,  1020,  1021,  1022,  1023,  1024,  1025,  1334,  1322,
    1767,  1772,   408,   150,  1773,  1646,  1780,  1334,  1781,  1034,
    1461,  1026,   205,  1786,   218,  1787,  1789,  1790,   819,   552,
    1593,  1726,    86,    87,  1037,    88,   168,    90,  1737,  1245,
     698,   408,  1695,   937,   696,   701,  1751,  1099,   318,   108,
    1059,    13,  1284,  1654,   108,  1749,   203,  1609,   108,   115,
     822,  1388,   205,  1617,   205,   115,  1490,  1636,   410,  1631,
     115,  1390,   351,  1391,  1429,   544,  1774,  1762,   317,  1410,
    1607,  1648,  1229,  1166,  1276,  1115,  1269,  1411,  1574,  1270,
    1127,   150,   205,  1079,   587,  1581,   651,   984,  1400,   585,
     964,  1714,   317,   150,   317,  1211,  1432,  1205,  1158,   121,
     317,     0,     0,  1323,     0,     0,     0,     0,  1324,     0,
      60,    61,    62,   163,  1325,   407,  1326,     0,     0,     0,
       0,   457,     0,   937,     0,     0,    36,   108,   108,   108,
       0,     0,     0,   108,   205,     0,     0,     0,   108,     0,
    1334,     0,     0,     0,     0,     0,  1334,    48,  1334,   205,
     205,     0,     0,  1327,  1328,     0,  1329,     0,     0,     0,
    1334,     0,   121,     0,   585,     0,     0,     0,     0,  1412,
     202,   121,     0,   612,     0,     0,     0,   408,     0,     0,
       0,     0,  1413,  1414,     0,  1467,   486,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   115,
     167,     0,     0,    84,  1415,     0,    86,    87,     0,    88,
    1416,    90,     0,     0,     0,     0,     0,  1782,     0,   218,
       0,   218,     0,     0,     0,  1788,     0,     0,     0,     0,
       0,  1791,     0,   624,  1792,     0,   471,   472,     0,   115,
     115,     0,     0,     0,  1334,     0,   115,   544,     0,     0,
     121,     0,   624,  1627,     0,     0,   121,     0,     0,    36,
     624,   121,     0,   150,  1322,     0,     0,    36,   317,   205,
     205,     0,   937,     0,     0,     0,   218,   108,     0,     0,
      48,     0,     0,   115,     0,  1669,     0,     0,    48,     0,
       0,     0,  1574,  1574,     0,     0,  1581,  1581,   202,     0,
       0,     0,   473,   474,     0,   612,    13,   202,     0,     0,
     351,     0,     0,    36,   202,     0,     0,   108,   108,     0,
       0,   202,     0,     0,   108,    36,   150,     0,     0,     0,
       0,   150,   218,   167,    48,   150,    84,    85,     0,    86,
      87,     0,    88,   168,    90,     0,    48,    86,    87,   115,
      88,   168,    90,     0,  1650,     0,   115,   218,     0,   629,
     218,   108,     0,     0,     0,     0,     0,  1725,  1323,     0,
       0,     0,     0,  1324,   259,    60,    61,    62,   163,  1325,
     407,  1326,     0,     0,     0,  1739,     0,     0,     0,     0,
     556,     0,   205,    86,    87,     0,    88,   168,    90,     0,
     121,   261,   333,   218,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,   150,   150,   150,     0,  1327,  1328,
     150,  1329,     0,    36,     0,   150,     0,   108,     0,     0,
       0,     0,   612,     0,   108,     0,  1710,   205,     0,     0,
     121,   121,   408,   202,    48,     0,     0,   121,     0,     0,
    1610,     0,   205,   205,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   486,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   531,
     532,    33,    34,    35,   121,     0,     0,     0,     0,     0,
       0,  1727,     0,   198,     0,   218,   218,   167,     0,   798,
      84,   311,     0,    86,    87,     0,    88,   168,    90,   877,
     982,     0,     0,     0,     0,     0,   471,   472,     0,     0,
       0,   315,     0,     0,   877,     0,     0,   205,     0,     0,
     798,   316,     0,     0,   204,   204,     0,     0,   220,     0,
       0,     0,    74,    75,    76,    77,    78,     0,     0,     0,
     121,     0,     0,   200,     0,     0,     0,   121,     0,    82,
      83,     0,     0,     0,   150,   437,   438,   439,   440,   441,
     442,     0,   443,    92,     0,     0,   218,   218,     0,     0,
       0,     0,   473,   474,   444,   218,     0,    97,     0,     0,
       0,     0,     0,     0,     0,     0,   417,   418,   419,     0,
       0,     0,     0,     0,   150,   150,   202,     0,     0,     0,
       0,   150,     0,     0,     0,   420,   612,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   750,
     443,   781,     0,     0,     0,     0,     0,     0,   150,     0,
       0,     0,   444,     0,     0,     0,     0,     0,     0,     0,
     259,     0,     0,   202,     0,   486,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,     0,     0,
       0,     0,     0,     0,     0,   612,     0,   261,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,   782,
       0,     0,     0,   202,     0,   202,     0,     0,     0,    36,
       0,    48,     0,     0,   150,   471,   472,     0,   204,     0,
       0,   150,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,   202,   798,     0,     0,     0,   537,     0,
       0,     0,     0,     0,     0,     0,     0,   218,   218,   798,
     798,   798,   798,   798,     0,     0,     0,   798,     0,     0,
       0,     0,     0,     0,   167,   531,   532,    84,   218,     0,
      86,    87,     0,    88,   168,    90,     0,     0,     0,  1222,
       0,   473,   474,   167,     0,   202,    84,   311,     0,    86,
      87,     0,    88,   168,    90,     0,     0,     0,   218,     0,
     202,   202,     0,     0,     0,     0,     0,   315,    60,    61,
      62,    63,    64,   407,   218,   218,     0,   316,     0,    70,
     450,     0,     0,     0,   218,     0,     0,     0,     0,     0,
     218,     0,     0,     0,     0,     0,     0,     0,   830,     0,
       0,     0,     0,   218,     0,     0,   204,     0,     0,     0,
       0,   798,     0,     0,   218,   204,     0,   452,     0,     0,
       0,     0,   204,     0,     0,     0,   417,   418,   419,   204,
       0,     0,   218,     0,     0,   408,   218,     0,     0,     0,
     204,     0,     0,     0,     0,   420,     0,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,     0,
     443,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     202,   202,   444,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   218,     0,     0,   218,     0,   218,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   220,     0,     0,   798,     0,   218,     0,     0,   798,
     798,   798,   798,   798,   798,   798,   798,   798,   798,   798,
     798,   798,   798,   798,   798,   798,   798,   798,   798,   798,
     798,   798,   798,   798,   798,   798,   798,     0,   417,   418,
     419,   204,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   798,     0,     0,     0,     0,     0,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,   218,   443,   218,     0,     0,     0,     0,     0,     0,
       0,   927,   928,   202,   444,     0,   850,   801,     0,     0,
       0,     0,    36,     0,     0,     0,     0,   320,     0,     0,
     218,   929,     0,   218,     0,     0,     0,     0,     0,   930,
     931,   932,    36,    48,     0,     0,     0,     0,   801,     0,
       0,   933,     0,   218,     0,     0,     0,     0,   202,     0,
       0,     0,     0,    48,     0,  1412,     0,     0,     0,     0,
       0,     0,     0,   202,   202,     0,   798,     0,  1413,  1414,
       0,     0,     0,     0,     0,   218,     0,     0,     0,   218,
       0,     0,   798,     0,   798,     0,   167,     0,   934,    84,
      85,     0,    86,    87,     0,    88,  1416,    90,     0,     0,
     798,   935,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    86,    87,   204,    88,   168,    90,   417,   418,
     419,     0,     0,     0,     0,     0,     0,     0,   882,     0,
     936,     0,   218,   218,     0,   218,     0,   420,   202,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,     0,   443,     0,   417,   418,   419,     0,     0,     0,
       0,   204,     0,     0,   444,     0,     0,     0,     0,     0,
       0,     0,     0,   420,     0,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,     0,   443,     0,
       0,   204,     0,   204,     0,     0,     0,     0,     0,   218,
     444,   218,   320,     0,   320,     0,   798,   218,     0,     0,
     798,     0,   798,     0,     0,   798,     0,     0,     0,     0,
       0,   204,   801,   218,   218,   259,     0,   218,     0,     0,
       0,     0,     0,     0,   218,     0,     0,   801,   801,   801,
     801,   801,     0,     0,     0,   801,     0,     0,     0,     0,
       0,     0,   261,     0,     0,     0,  1030,     0,     0,   320,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   204,    36,     0,   218,     0,   921,     0,
       0,     0,     0,     0,     0,     0,  1048,     0,   204,   204,
       0,     0,   798,     0,     0,    48,     0,     0,     0,     0,
       0,   218,   218,  1048,     0,     0,     0,     0,     0,   218,
       0,   218,   204,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   925,     0,     0,     0,     0,     0,
     531,   532,     0,   218,     0,   218,     0,     0,     0,   801,
     320,   218,  1089,   320,     0,     0,     0,     0,   167,     0,
       0,    84,   311,     0,    86,    87,     0,    88,   168,    90,
       0,  1300,     0,     0,   220,     0,     0,     0,     0,     0,
       0,     0,   315,     0,     0,     0,     0,   798,   798,     0,
       0,     0,   316,   798,     0,   218,     0,     0,     0,     0,
       0,   218,     0,   218,     0,   486,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   204,   204,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
       0,   443,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   801,   444,   204,   471,   472,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,   801,   801,   801,   801,   801,
     801,   801,   801,   801,   801,     0,     0,     0,   320,   783,
       0,     0,   799,     0,     0,     0,     0,     0,     0,   801,
       0,     0,   218,     0,     0,     0,     0,   259,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   218,
       0,   473,   474,   799,     0,     0,     0,   206,   206,     0,
       0,   224,     0,     0,   261,     0,   218,     0,     0,     0,
       0,   204,   798,    60,    61,    62,    63,    64,   407,     0,
       0,     0,     0,   798,    70,   450,    36,     0,     0,   798,
       0,     0,     0,   798,     0,     0,     0,     0,     0,   320,
     320,     0,     0,     0,     0,     0,     0,    48,   320,     0,
       0,   204,     0,     0,   218,     0,   204,     0,     0,     0,
     451,     0,   452,     0,     0,     0,     0,     0,     0,     0,
       0,   204,   204,     0,   801,   453,     0,   454,     0,     0,
     408,     0,   531,   532,     0,     0,     0,     0,     0,     0,
     801,     0,   801,   798,     0,     0,     0,     0,     0,     0,
     167,     0,   218,    84,   311,     0,    86,    87,   801,    88,
     168,    90,     0,     0,     0,     0,     0,     0,   218,     0,
       0,     0,     0,     0,   315,     0,     0,   218,   417,   418,
     419,     0,     0,     0,   316,     0,     0,     0,     0,     0,
       0,     0,   218,  1321,     0,     0,   204,   420,     0,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,     0,   443,     0,     0,     0,     0,     0,     0,     0,
       0,   206,     0,     0,   444,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   799,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     320,   320,   799,   799,   799,   799,   799,     0,     0,     0,
     799,  -968,  -968,  -968,  -968,  -968,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,   801,   204,     0,     0,   801,     0,
     801,     0,     0,   801,     0,     0,     0,  1026,     0,     0,
       0,     0,  1408,     0,     0,  1421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   320,   443,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     444,     0,     0,   320,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   204,     0,   320,     0,  1040,   206,
       0,     0,     0,     0,   799,     0,     0,     0,   206,     0,
     801,     0,     0,     0,     0,   206,   417,   418,   419,  1480,
    1481,     0,   206,     0,     0,   320,     0,     0,     0,  1421,
       0,     0,     0,   224,     0,   420,     0,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,     0,
     443,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   444,     0,  -968,  -968,  -968,  -968,  -968,   435,
     436,   437,   438,   439,   440,   441,   442,   320,   443,     0,
     320,     0,   783,     0,     0,   801,   801,     0,     0,     0,
     444,   801,     0,  1625,   224,     0,     0,   799,     0,     0,
       0,  1421,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
     799,   799,   799,   799,   799,   799,   799,   799,   799,   799,
       0,   417,   418,   419,   206,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   799,     0,     0,     0,     0,     0,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   320,   443,   320,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1096,   444,     0,     0,
     802,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   320,     0,     0,   320,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    28,     0,
       0,   802,     0,     0,     0,     0,    33,    34,    35,    36,
       0,   197,     0,     0,     0,     0,     0,     0,   198,     0,
     801,     0,     0,     0,     0,     0,     0,     0,     0,   799,
      48,   801,     0,     0,     0,     0,     0,   801,   320,     0,
       0,   801,   320,     0,     0,   799,     0,   799,     0,     0,
       0,   199,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   799,     0,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,   206,   200,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,  1107,    88,   168,    90,   320,   320,     0,    92,     0,
       0,   801,     0,     0,     0,     0,     0,     0,     0,     0,
    1736,     0,    97,     0,   417,   418,   419,   201,     0,     0,
     564,     0,   103,     0,     0,     0,  1408,     0,     0,     0,
       0,     0,     0,   420,   206,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,     0,   443,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     444,     0,     0,     0,   206,     0,   206,     0,     0,     0,
       0,     0,   320,     0,   320,     0,     0,     0,     0,   799,
       0,     0,     0,   799,     0,   799,     0,     0,   799,     0,
       0,     0,     0,     0,   206,   802,   320,     0,   417,   418,
     419,     0,     0,     0,     0,     0,     0,   320,     0,     0,
     802,   802,   802,   802,   802,     0,   803,   420,   802,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,     0,   443,   259,     0,     0,   206,   823,     0,     0,
       0,     0,     0,     0,   444,     0,     0,     0,     0,     0,
       0,   206,   206,     0,     0,   799,     0,     0,     0,     0,
     261,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   320,     0,  1131,   224,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   320,     0,   320,     0,
       0,     0,   802,    48,   320,     0,     0,     0,     0,     0,
       0,  -385,     0,     0,     0,     0,     0,     0,     0,    60,
      61,    62,   163,   164,   407,     0,     0,   224,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   531,   532,
     799,   799,     0,     0,     0,     0,   799,     0,     0,     0,
       0,     0,     0,     0,   320,     0,   167,     0,     0,    84,
     311,     0,    86,    87,     0,    88,   168,    90,  1444,     0,
       0,   206,   206,     0,     0,     0,     0,     0,     0,     0,
     315,     0,     0,     0,     0,     0,   408,     0,     0,     0,
     316,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   802,     0,   224,     0,     0,
     802,   802,   802,   802,   802,   802,   802,   802,   802,   802,
     802,   802,   802,   802,   802,   802,   802,   802,   802,   802,
     802,   802,   802,   802,   802,   802,   802,   802,     0,     0,
       0,   965,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   802,     0,     0,   320,   987,   988,   989,   990,
       0,     0,     0,     0,   997,     0,     0,     0,     0,     0,
       0,     0,   320,     0,     0,     0,     0,     0,     0,     0,
       0,   961,     0,     0,     0,     0,     0,     0,     0,  1670,
       0,     0,     0,     0,   206,   799,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   799,     0,     0,     0,
       0,     0,   799,    28,     0,     0,   799,     0,     0,     0,
       0,    33,    34,    35,    36,     0,   197,     0,     0,     0,
       0,     0,     0,   198,   224,     0,     0,   320,     0,   206,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,   206,   206,     0,   802,  1086,     0,
       0,     0,     0,     0,     0,     0,   199,     0,     0,     0,
       0,     0,     0,   802,     0,   802,   799,     0,     0,   962,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,   802,     0,   200,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
     320,     0,     0,    92,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   320,     0,    97,     0,   206,
       0,     0,   201,     0,     0,     0,     0,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1174,  1177,  1178,  1179,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,  1189,  1190,
    1191,  1192,  1193,  1194,  1195,  1196,  1197,  1198,  1199,  1200,
    1201,  1202,  1203,  1204,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1210,     0,
     417,   418,   419,     0,     0,     0,     0,   802,   224,     0,
       0,   802,     0,   802,     0,     0,   802,     0,     0,   420,
       0,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,     0,   443,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   444,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,   224,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1026,     0,     0,   802,     0,     0,     0,     0,     5,     6,
       7,     8,     9,  1288,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1303,
       0,  1304,   379,    12,     0,     0,     0,     0,     0,     0,
     703,     0,     0,     0,     0,     0,     0,  1314,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,   802,   802,
    1445,     0,    41,     0,   802,     0,     0,     0,     0,     0,
       0,     0,     0,  1630,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   163,   164,   165,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   166,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,  1393,     0,    93,     0,  1395,     0,  1396,
       0,    94,  1397,     0,     0,     0,    97,    98,    99,     0,
       0,   100,   417,   418,   419,     0,   103,   104,     0,   105,
     106,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   420,     0,   421,   422,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,     0,   443,     0,     0,     0,
       0,     0,     0,   802,   417,   418,   419,     0,   444,     0,
       0,     0,     0,     0,   802,     0,     0,     0,     0,  1476,
     802,     0,     0,   420,   802,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,  1711,   443,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     444,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   489,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   802,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1619,  1620,    13,    14,    15,     0,
    1624,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,    40,     0,     0,     0,    41,    42,    43,
      44,     0,    45,     0,    46,     0,    47,     0,     0,    48,
      49,     0,     0,     0,    50,    51,    52,    53,    54,    55,
      56,     0,    57,    58,    59,    60,    61,    62,    63,    64,
      65,   735,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,     0,    72,    73,     0,    74,    75,    76,    77,
      78,     0,     0,     0,    79,     0,     0,    80,     0,     0,
       0,     0,    81,    82,    83,    84,    85,     0,    86,    87,
       0,    88,    89,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,    95,     0,    96,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
    1056,   103,   104,     0,   105,   106,     0,     0,     0,  1679,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
    1689,     0,    10,     0,     0,     0,  1694,     0,     0,     0,
    1696,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
    1729,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,    54,    55,    56,
       0,    57,    58,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,    69,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,    81,    82,    83,    84,    85,     0,    86,    87,     0,
      88,    89,    90,    91,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,    95,     0,    96,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,  1223,
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
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,   631,   103,   104,     0,   105,
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
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,    91,     0,     0,    92,     0,     0,    93,     0,
       0,     0,     0,     0,    94,     0,     0,     0,     0,    97,
      98,    99,     0,     0,   100,     0,   101,   102,  1029,   103,
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
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,  1070,   103,   104,     0,   105,   106,     5,     6,     7,
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
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,  1137,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,  1139,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
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
      44,     0,    45,     0,    46,     0,    47,  1289,     0,    48,
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
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,    91,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,  1399,   103,   104,     0,   105,   106,     5,
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
      79,     0,     0,    80,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
      91,     0,     0,    92,     0,     0,    93,     0,     0,     0,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   100,     0,   101,   102,  1621,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,  1666,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,    91,     0,     0,    92,     0,     0,    93,
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
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,    91,     0,     0,    92,
       0,     0,    93,     0,     0,     0,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   100,     0,
     101,   102,  1700,   103,   104,     0,   105,   106,     5,     6,
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
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,    91,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   100,     0,   101,   102,  1701,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,    40,     0,     0,     0,    41,    42,    43,    44,     0,
      45,  1704,    46,     0,    47,     0,     0,    48,    49,     0,
       0,     0,    50,    51,    52,    53,     0,    55,    56,     0,
      57,     0,    59,    60,    61,    62,    63,    64,    65,     0,
      66,    67,    68,     0,    70,    71,     0,     0,     0,     0,
       0,    72,    73,     0,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,    80,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,     0,    88,
     168,    90,    91,     0,     0,    92,     0,     0,    93,     0,
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
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,    91,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   100,     0,   101,
     102,  1720,   103,   104,     0,   105,   106,     5,     6,     7,
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
       0,    80,     0,     0,     0,     0,   167,    82,    83,    84,
      85,     0,    86,    87,     0,    88,   168,    90,    91,     0,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     100,     0,   101,   102,  1776,   103,   104,     0,   105,   106,
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
       0,    79,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,    91,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   100,     0,   101,   102,  1783,   103,   104,
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
       0,     0,   167,    82,    83,    84,    85,     0,    86,    87,
       0,    88,   168,    90,    91,     0,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   100,     0,   101,   102,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,   514,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,   163,   164,    65,     0,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,    79,     0,     0,
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   101,   102,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,   768,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,    11,    12,     0,   967,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,   163,   164,    65,
       0,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,    79,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   100,     0,   101,   102,     0,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,  1471,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    11,    12,     0,  1614,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   163,   164,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   163,
     164,   165,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   166,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,     0,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   169,     0,   325,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,     0,   443,   646,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     444,    14,    15,     0,     0,     0,     0,    16,     0,    17,
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
      85,     0,    86,    87,     0,    88,   168,    90,     0,   647,
       0,    92,     0,     0,    93,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     0,    97,    98,    99,     0,     0,
     169,     0,     0,     0,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   163,   164,   165,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     166,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   169,     0,     0,   763,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,     0,     0,  1083,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1026,    14,    15,     0,
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
       0,    88,   168,    90,     0,  1084,     0,    92,     0,     0,
      93,     0,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    97,    98,    99,     0,     0,   169,     0,     0,     0,
       0,   103,   104,     0,   105,   106,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     379,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   163,   164,   165,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   166,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   100,
       0,   417,   418,   419,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     420,  1293,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,     0,   443,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   444,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
     180,     0,     0,    53,     0,     0,     0,     0,     0,     0,
       0,    60,    61,    62,   163,   164,   165,     0,     0,    67,
      68,     0,     0,     0,     0,     0,     0,     0,     0,   166,
      73,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
       0,     0,     0,    92,     0,     0,    93,     0,     0,  1294,
       0,     0,    94,     0,     0,     0,     0,    97,    98,    99,
       0,     0,   169,     0,     0,     0,     0,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,     0,   443,     0,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   444,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   163,   164,   165,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   166,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   169,     0,   417,   418,   419,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   420,     0,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
       0,   443,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   444,     0,    16,     0,    17,    18,    19,
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
       0,     0,    93,     0,     0,   445,     0,     0,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   169,     0,
     245,   418,   419,   103,   104,     0,   105,   106,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   420,
       0,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,     0,   443,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   444,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   163,   164,   165,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   166,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,     0,     0,    93,     0,     0,     0,     0,
       0,    94,     0,     0,     0,     0,    97,    98,    99,     0,
       0,   169,     0,   248,     0,     0,   103,   104,     0,   105,
     106,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   379,     0,     0,     0,     0,
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
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   163,
     164,   165,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   166,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    80,     0,
       0,     0,     0,   167,    82,    83,    84,    85,     0,    86,
      87,     0,    88,   168,    90,     0,     0,     0,    92,     0,
       0,    93,     0,     0,   528,     0,     0,    94,     0,     0,
       0,     0,    97,    98,    99,     0,     0,   169,   512,     0,
       0,     0,   103,   104,     0,   105,   106,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   659,     0,     0,     0,     0,     0,
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
     169,     0,     0,     0,     0,   103,   104,     0,   105,   106,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,     0,
       0,     0,   703,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1026,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   163,   164,   165,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     166,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,   167,
      82,    83,    84,    85,     0,    86,    87,     0,    88,   168,
      90,     0,     0,     0,    92,     0,     0,    93,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,    97,    98,
      99,     0,     0,   169,     0,     0,     0,     0,   103,   104,
       0,   105,   106,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,     0,   443,     0,     0,   743,     0,     0,     0,     0,
       0,     0,     0,     0,   444,     0,     0,    14,    15,     0,
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
       9,     0,     0,     0,     0,     0,    10,  -968,  -968,  -968,
    -968,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,     0,   443,     0,     0,   745,     0,
       0,     0,     0,     0,     0,     0,     0,   444,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   163,   164,   165,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   166,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,   167,    82,    83,    84,    85,
       0,    86,    87,     0,    88,   168,    90,     0,     0,     0,
      92,     0,     0,    93,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,    97,    98,    99,     0,     0,   169,
       0,     0,     0,     0,   103,   104,     0,   105,   106,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,     0,     0,     0,
       0,  1128,     0,     0,     0,     0,     0,     0,     0,     0,
    1026,     0,     0,    14,    15,     0,     0,     0,     0,    16,
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
       0,     0,   169,     0,   417,   418,   419,   103,   104,     0,
     105,   106,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   420,     0,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,     0,   443,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     444,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   163,   164,   165,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   166,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    80,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,     0,     0,    93,
       0,     0,   530,     0,     0,    94,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   169,     0,   417,   418,   419,
     103,   104,     0,   105,   106,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   420,   899,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
       0,   443,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   444,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,   593,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,     0,     0,    60,    61,    62,
     163,   164,   165,     0,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,   166,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,    80,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,     0,   250,   251,    92,
     252,   253,    93,     0,   254,   255,   256,   257,    94,     0,
       0,     0,     0,    97,    98,    99,     0,     0,   169,     0,
       0,   258,     0,   103,   104,     0,   105,   106,  -968,  -968,
    -968,  -968,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,
    1021,  1022,  1023,  1024,  1025,     0,     0,     0,   260,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1026,     0,
       0,     0,   262,   263,   264,   265,   266,   267,   268,     0,
       0,     0,    36,     0,   197,     0,     0,     0,     0,     0,
       0,     0,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,    48,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,     0,     0,     0,
     691,   304,   305,   306,     0,     0,     0,   307,   541,   542,
     250,   251,     0,   252,   253,     0,     0,   254,   255,   256,
     257,     0,     0,     0,     0,     0,   543,     0,     0,     0,
       0,     0,    86,    87,   258,    88,   168,    90,   312,     0,
     313,     0,     0,   314,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   260,     0,   692,     0,   103,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   262,   263,   264,   265,   266,
     267,   268,     0,     0,     0,    36,     0,   197,     0,     0,
       0,     0,     0,     0,     0,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,    48,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
       0,     0,     0,   303,   304,   305,   306,     0,     0,     0,
     307,   541,   542,     0,     0,     0,     0,     0,   250,   251,
       0,   252,   253,     0,     0,   254,   255,   256,   257,   543,
       0,     0,     0,     0,     0,    86,    87,     0,    88,   168,
      90,   312,   258,   313,   259,     0,   314,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   692,     0,   103,   260,
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
       0,   315,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   316,     0,     0,     0,  1594,     0,   260,     0,   261,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   262,   263,   264,   265,   266,   267,   268,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,    48,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,     0,     0,     0,     0,
     304,   305,   306,     0,     0,     0,   307,   308,   309,     0,
       0,     0,     0,     0,   250,   251,     0,   252,   253,     0,
       0,   254,   255,   256,   257,   310,     0,     0,    84,   311,
       0,    86,    87,     0,    88,   168,    90,   312,   258,   313,
     259,     0,   314,     0,     0,     0,     0,     0,     0,   315,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   316,
       0,     0,     0,  1662,     0,   260,     0,   261,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   262,
     263,   264,   265,   266,   267,   268,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
      48,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,     0,     0,     0,   303,   304,   305,
     306,     0,     0,     0,   307,   308,   309,     0,     0,     0,
       0,     0,   250,   251,     0,   252,   253,     0,     0,   254,
     255,   256,   257,   310,     0,     0,    84,   311,     0,    86,
      87,     0,    88,   168,    90,   312,   258,   313,   259,     0,
     314,     0,     0,     0,     0,     0,     0,   315,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   316,     0,     0,
       0,     0,     0,   260,     0,   261,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   262,   263,   264,
     265,   266,   267,   268,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,    48,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,     0,     0,     0,     0,   304,   305,   306,     0,
       0,     0,   307,   308,   309,     0,     0,     0,     0,     0,
     250,   251,     0,   252,   253,     0,     0,   254,   255,   256,
     257,   310,     0,     0,    84,   311,     0,    86,    87,     0,
      88,   168,    90,   312,   258,   313,   259,     0,   314,     0,
       0,     0,     0,     0,     0,   315,  1403,     0,     0,     0,
       0,     0,     0,     0,     0,   316,     0,     0,     0,     0,
       0,   260,     0,   261,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   262,   263,   264,   265,   266,
     267,   268,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,    48,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
       0,     0,     0,     0,   304,   305,   306,     0,     0,     0,
     307,   308,   309,    33,    34,    35,    36,     0,   197,     0,
       0,     0,     0,     0,     0,   198,     0,     0,     0,   310,
       0,     0,    84,   311,     0,    86,    87,    48,    88,   168,
      90,   312,     0,   313,     0,     0,   314,  1495,  1496,  1497,
    1498,  1499,     0,   315,  1500,  1501,  1502,  1503,   215,     0,
       0,     0,     0,   316,   216,     0,     0,     0,     0,     0,
       0,  1504,  1505,     0,    74,    75,    76,    77,    78,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
     167,    82,    83,    84,    85,     0,    86,    87,  1506,    88,
     168,    90,     0,     0,     0,    92,     0,     0,     0,     0,
       0,     0,  1507,  1508,  1509,  1510,  1511,  1512,  1513,    97,
       0,     0,    36,     0,   217,     0,     0,     0,     0,   103,
       0,     0,  1514,  1515,  1516,  1517,  1518,  1519,  1520,  1521,
    1522,  1523,  1524,    48,  1525,  1526,  1527,  1528,  1529,  1530,
    1531,  1532,  1533,  1534,  1535,  1536,  1537,  1538,  1539,  1540,
    1541,  1542,  1543,  1544,  1545,  1546,  1547,  1548,  1549,  1550,
    1551,  1552,  1553,  1554,     0,     0,     0,  1555,  1556,     0,
    1557,  1558,  1559,  1560,  1561,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1562,  1563,  1564,     0,
       0,     0,    86,    87,     0,    88,   168,    90,  1565,     0,
    1566,  1567,     0,  1568,   417,   418,   419,     0,     0,     0,
    1569,  1570,     0,  1571,     0,  1572,  1573,     0,     0,     0,
       0,     0,     0,   420,     0,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,     0,   443,   417,
     418,   419,     0,     0,     0,     0,     0,     0,     0,     0,
     444,     0,     0,     0,     0,     0,     0,     0,   420,     0,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,     0,   443,   417,   418,   419,     0,     0,     0,
       0,     0,     0,     0,     0,   444,     0,     0,     0,     0,
       0,     0,     0,   420,     0,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,     0,   443,  1000,
    1001,  1002,     0,     0,     0,     0,     0,     0,     0,     0,
     444,     0,     0,     0,     0,     0,     0,     0,  1003,     0,
    1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,   547,     0,     0,     0,     0,     0,     0,   250,
     251,     0,   252,   253,     0,  1026,   254,   255,   256,   257,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   258,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   551,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     260,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   262,   263,   264,   265,   266,   267,
     268,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,   760,     0,     0,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    48,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,     0,
       0,     0,   303,   304,   305,   306,  1171,     0,     0,   307,
     541,   542,   250,   251,     0,   252,   253,     0,     0,   254,
     255,   256,   257,     0,     0,     0,     0,     0,   543,     0,
       0,     0,     0,     0,    86,    87,   258,    88,   168,    90,
     312,     0,   313,     0,     0,   314,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   260,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   262,   263,   264,
     265,   266,   267,   268,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,    48,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,     0,     0,     0,  1172,   304,   305,   306,     0,
       0,     0,   307,   541,   542,   250,   251,     0,   252,   253,
       0,     0,   254,   255,   256,   257,     0,     0,     0,     0,
       0,   543,     0,     0,     0,     0,     0,    86,    87,   258,
      88,   168,    90,   312,     0,   313,     0,     0,   314,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   260,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     262,   263,   264,   265,   266,   267,   268,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,    48,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,     0,     0,     0,     0,   304,
     305,   306,  1180,     0,     0,   307,   541,   542,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   789,
     790,     0,     0,     0,   543,   791,     0,   792,     0,     0,
      86,    87,     0,    88,   168,    90,   312,     0,   313,   793,
       0,   314,     0,     0,     0,     0,     0,    33,    34,    35,
      36,   417,   418,   419,     0,     0,     0,     0,     0,   198,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     420,    48,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,     0,   443,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   794,   444,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,   167,    82,    83,    84,   795,     0,
      86,    87,     0,    88,   168,    90,   789,   790,     0,    92,
       0,     0,   791,     0,   792,     0,     0,     0,   796,     0,
       0,     0,     0,    97,     0,     0,   793,     0,   797,     0,
       0,     0,     0,     0,    33,    34,    35,    36,   417,   418,
     419,     0,     0,   498,     0,     0,   198,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   420,    48,   421,
     422,   423,   424,   425,   426,   427,   428,   429,   430,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   441,
     442,     0,   443,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   794,   444,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
       0,   167,    82,    83,    84,   795,     0,    86,    87,    28,
      88,   168,    90,     0,     0,     0,    92,    33,    34,    35,
      36,     0,   197,     0,     0,   796,     0,     0,     0,   198,
      97,     0,     0,     0,     0,   797,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
     902,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   199,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   584,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,     0,    88,   168,    90,    28,     0,   916,    92,
       0,     0,     0,     0,    33,    34,    35,    36,     0,   197,
       0,     0,     0,    97,     0,     0,   198,     0,   201,     0,
       0,     0,     0,   103,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   199,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,    28,
      88,   168,    90,     0,     0,     0,    92,    33,    34,    35,
      36,     0,   197,     0,     0,     0,     0,     0,     0,   198,
      97,     0,     0,     0,     0,   201,     0,     0,     0,     0,
     103,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   199,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1051,    73,     0,    74,    75,
      76,    77,    78,     0,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,   167,    82,    83,    84,    85,     0,
      86,    87,    28,    88,   168,    90,     0,     0,     0,    92,
      33,    34,    35,    36,     0,   197,     0,     0,     0,     0,
       0,     0,   198,    97,     0,     0,     0,     0,   201,     0,
       0,     0,     0,   103,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   199,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,   200,     0,     0,     0,     0,   167,    82,    83,
      84,    85,     0,    86,    87,     0,    88,   168,    90,     0,
       0,     0,    92,     0,     0,     0,   417,   418,   419,     0,
       0,     0,     0,     0,     0,     0,    97,     0,     0,     0,
       0,   201,     0,     0,     0,   420,   103,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,     0,
     443,   417,   418,   419,     0,     0,     0,     0,     0,     0,
       0,     0,   444,     0,     0,     0,     0,     0,     0,     0,
     420,     0,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,     0,   443,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   444,  1000,  1001,
    1002,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1003,   946,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,
    1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,
    1025,     0,     0,  1000,  1001,  1002,     0,     0,     0,     0,
       0,     0,     0,     0,  1026,     0,     0,     0,     0,     0,
       0,     0,  1003,  1252,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1026,
    1000,  1001,  1002,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1003,
    1162,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,     0,     0,  1000,  1001,  1002,     0,     0,
       0,     0,     0,     0,     0,     0,  1026,     0,     0,     0,
       0,     0,     0,     0,  1003,  1310,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,     0,     0,
       0,    33,    34,    35,    36,     0,   197,     0,     0,     0,
       0,  1026,     0,   607,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,  1392,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   199,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,  1478,   167,    82,
      83,    84,    85,     0,    86,    87,     0,    88,   168,    90,
       0,     0,     0,    92,    33,    34,    35,    36,     0,   197,
       0,     0,     0,     0,     0,     0,   198,    97,     0,     0,
       0,     0,   608,     0,     0,     0,     0,   103,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   215,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
       0,   167,    82,    83,    84,    85,     0,    86,    87,     0,
      88,   168,    90,     0,     0,     0,    92,     0,     0,     0,
     417,   418,   419,     0,     0,     0,     0,     0,     0,     0,
      97,     0,     0,     0,     0,   217,     0,     0,   772,   420,
     103,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,     0,   443,   417,   418,   419,     0,     0,
       0,     0,     0,     0,     0,     0,   444,     0,     0,     0,
       0,     0,     0,     0,   420,     0,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   773,   443,
    1000,  1001,  1002,     0,     0,     0,     0,     0,     0,     0,
       0,   444,     0,     0,     0,     0,     0,     0,     0,  1003,
    1315,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1000,  1001,  1002,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1026,     0,     0,     0,
       0,     0,  1003,     0,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1001,  1002,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1026,
       0,     0,     0,     0,  1003,     0,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,
    1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,   419,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1026,     0,     0,     0,   420,     0,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,  1002,
     443,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   444,     0,     0,     0,  1003,     0,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,
    1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   420,  1026,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
     437,   438,   439,   440,   441,   442,     0,   443,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1003,   444,
    1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
    1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1026,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,     0,   443,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   444,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1026,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1026
};

static const yytype_int16 yycheck[] =
{
       5,     6,   121,     8,     9,    10,    11,    12,   146,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    54,    91,    28,    29,   649,    95,    96,   875,   502,
       4,   171,     4,     4,   100,   151,   620,    42,    30,   479,
     480,   229,   483,    32,    55,    50,   146,    52,     4,   218,
      55,   120,    57,   172,    54,    44,   372,   619,   372,   372,
      49,   443,  1073,   737,   100,    42,    30,    30,   508,   213,
      30,   514,     4,   864,    79,    26,    27,   600,   100,   100,
     475,   475,   778,   960,     9,     9,   747,     9,   895,  1060,
      14,     9,    14,     9,     9,   100,     9,    30,    14,     9,
     771,  1069,    79,    58,   911,   548,     9,     9,    33,   700,
       9,     9,    46,     9,   230,   510,   510,     9,     9,     9,
       9,    35,     9,     9,     9,    67,    81,    46,     9,    84,
       9,     9,     9,   169,     9,     4,   103,     9,     9,    51,
     947,     9,    86,     9,     9,     9,    98,   169,   169,    80,
       9,  1608,    46,     9,    80,    35,    86,    30,    67,    67,
      80,   130,   131,   151,   169,   201,    80,   102,   103,   357,
     111,   176,   151,   130,   131,   118,   151,     0,   621,   201,
     201,   217,   186,   126,   172,    46,    35,   186,   130,   131,
     155,   507,   186,   172,     8,   217,   201,    67,   186,    46,
      80,   189,   186,   155,   148,   996,  1663,   102,   103,    35,
     189,   186,   217,     4,    67,    30,    67,   186,   148,    67,
     151,   130,   131,   188,   189,    67,   231,    67,   169,   234,
      67,    80,    67,   190,    67,   189,   241,   242,    67,   165,
      67,   186,    67,   151,   156,   165,    67,   189,   172,    35,
     184,   165,   121,   914,    80,   190,   189,   186,    67,   210,
      51,   235,   187,    54,   188,   239,   188,  1144,   187,   409,
     188,   189,   188,   188,   970,   188,   972,   187,  1249,  1247,
      71,   189,   173,   173,   187,  1256,   188,  1258,   173,   188,
     188,   324,   188,   187,    80,   190,   188,  1104,    89,   188,
      91,   188,   188,   172,    95,    96,  1277,   188,   892,   188,
     188,   188,   187,   349,   173,   188,   188,   188,   151,   187,
     763,   187,   187,   187,   324,   768,   187,   349,   349,   120,
     853,   187,  1123,   186,   403,   186,     4,   184,   457,   165,
     187,   189,   151,   348,   349,   485,   186,   189,   492,   189,
     355,   186,   189,   186,   189,   360,   189,   186,   119,   186,
     189,    35,   189,    67,   189,   126,    54,   400,   189,   186,
      67,   348,    67,   186,   379,    67,   331,   332,   333,   165,
     189,   151,   387,   452,   453,   454,   455,   338,   186,  1357,
      80,  1359,   458,   184,   399,   155,   347,  1368,   349,   840,
     400,    67,   172,   354,    26,    27,    80,    51,    80,   364,
     361,   186,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   626,   444,
     628,   446,   447,   448,   235,    80,   378,   458,   239,   449,
    1124,   443,   243,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   610,  1058,   908,  1140,
      80,   476,   477,    80,   479,   480,   481,   482,   146,   443,
     443,   486,   449,   443,   489,   189,   881,   881,  1323,  1457,
      80,   165,   189,   498,   189,   500,   496,   189,    80,   637,
       4,   191,   186,   508,  1088,   516,    67,  1091,   475,   157,
    1072,   516,   156,   518,    80,   507,  1307,   386,   156,   191,
     186,   187,    26,    27,   967,   694,    54,   171,   186,   496,
     165,    29,   483,   324,   916,    98,     4,   637,    66,   186,
     507,   189,    98,   510,   186,    98,   690,   521,   864,    47,
     864,   864,    50,   741,   742,    35,   725,    80,    98,   564,
     748,   749,   702,    86,    47,    48,   148,   149,   150,   130,
     131,   259,   608,   261,  1409,   165,   183,    98,    46,   189,
     172,   191,   189,   149,   150,  1376,   655,   456,   210,   188,
     151,  1265,   155,  1267,   186,   386,   100,   552,  1433,   155,
    1435,   556,   155,   608,   188,   396,   561,    80,   188,   400,
     107,   188,   403,    86,   188,   155,   188,    29,   115,   116,
     117,   118,   119,   120,   157,   186,   149,   150,   316,   798,
     172,    67,   748,  1106,   155,    47,   805,   170,    50,    14,
     108,   785,   647,  1611,   186,   113,    67,   115,   116,   117,
     118,   119,   120,   121,   659,    30,   189,  1097,   449,   450,
     451,   452,   453,   454,   455,   169,  1257,  1251,  1108,   447,
     130,   131,   195,    48,  1236,    78,   149,   150,   172,   189,
     996,   151,   996,   996,   475,   182,   186,   692,   620,    67,
     158,   159,   186,   161,   172,   646,    99,   201,   476,   186,
    1374,    80,  1143,   481,   151,   496,   210,    86,   186,   397,
     172,    80,   400,   217,   182,   720,   338,    86,   386,   510,
      78,   194,   190,   186,   186,   347,   155,    50,    51,    52,
     521,   235,   354,    72,    73,   239,  1747,    72,    73,   361,
      78,    99,    80,    66,    98,    99,    78,  1338,   188,   540,
     755,  1762,   703,   156,    45,   158,   159,   160,   161,   162,
     163,    99,   128,   129,  1207,    66,   771,    99,   172,   148,
     149,   150,   563,  1741,   101,   102,   103,  1218,   767,   148,
     149,   150,   193,   186,   151,  1369,   188,   189,  1756,   186,
    1464,   186,   743,     9,   745,   188,   189,   941,   589,   590,
     158,   159,   776,   161,   162,   163,    80,  1123,   151,  1123,
    1123,   151,    86,  1253,   101,   102,   103,     8,   156,  1410,
     158,   159,   773,   161,   162,   163,   158,   159,   186,   161,
     162,   163,  1637,  1638,   338,   186,  1279,   151,    78,   188,
      80,   186,   986,   347,   849,   349,   915,   535,    14,   993,
     354,   189,  1295,   191,   186,   151,   811,   361,   863,    99,
     815,   483,   188,    80,   655,    80,   126,   867,  1715,    86,
     126,    86,  1633,  1634,   825,   149,   150,    14,   852,   187,
     852,   852,   386,   172,   889,    26,    27,  1734,    14,   840,
     841,    98,   761,   187,   899,  1742,   852,   902,   186,   904,
     867,   187,   187,   908,   115,   116,   117,   118,   119,   120,
    1079,   187,   879,   192,   881,  1355,   156,   186,   158,   159,
     852,   161,   162,   163,   916,   115,   116,   117,   616,   617,
     787,   788,   149,   150,   149,   150,  1379,   625,   807,  1613,
       4,   946,   107,   875,   186,  1388,   737,     9,   739,   189,
     148,   191,   916,   916,    78,  1398,   916,    50,    51,    52,
     892,    54,   187,   187,   953,   104,   105,   106,   187,   637,
     761,   182,   358,    66,  1118,    99,   362,   188,   187,   483,
     954,   948,    46,   852,   775,   776,    90,  1053,     9,   188,
      78,  1307,    14,  1307,  1307,   995,   172,   186,   998,     9,
     186,    80,   187,   389,   873,   391,   392,   393,   394,   960,
     961,    99,   187,   187,   128,   188,   807,   521,   186,    67,
     187,    30,   813,   129,   646,  1169,   817,   818,  1471,    78,
     151,   155,  1176,  1657,   158,   159,   171,   161,   162,   163,
     132,     9,  1053,   187,   108,    14,   837,     9,  1053,   113,
      99,   115,   116,   117,   118,   119,   120,   121,   107,   108,
    1376,   852,  1376,  1376,   152,   189,   151,   155,   184,   210,
     158,   159,     9,   161,   162,   163,   867,   173,   187,  1084,
       9,   703,   873,  1057,    14,  1057,  1057,   956,   879,   958,
     881,   128,  1097,   761,   158,   159,   193,   161,   193,   787,
     788,  1057,   190,  1108,  1109,     9,   155,   190,  1732,   158,
     159,   193,   161,   162,   163,    14,   187,   187,   182,   193,
     186,   743,   151,   745,   915,  1057,   190,   187,    98,   188,
    1274,   188,  1083,    87,   132,  1140,   927,   928,   929,   807,
     151,     9,   646,  1132,   187,  1150,   186,   151,   186,   151,
     189,   773,    14,   189,   188,    14,  1088,   948,   189,  1091,
    1603,  1135,  1605,   954,    14,   956,   854,   958,   193,   189,
     187,  1614,   188,    30,   186,    78,   184,  1128,    30,   186,
      14,   186,   870,   186,   852,    14,    49,   978,  1057,   186,
      78,   186,  1143,  1144,     9,   883,    99,   338,     4,   703,
     187,  1339,   188,   825,   995,   873,   347,   998,   188,   186,
     132,    99,    14,   354,   132,  1334,     9,  1660,   840,   841,
     361,   124,   187,  1228,   912,    66,   193,  1232,   186,  1234,
       9,   372,    80,     9,   132,   186,  1027,  1242,   188,   743,
      46,   745,  1033,    14,    80,  1036,   187,  1252,  1253,   189,
       4,   186,   186,   189,   187,   158,   159,   761,   161,   162,
     163,   189,   188,   132,     9,    87,  1057,  1218,   156,   773,
     158,   159,   776,   161,   162,   163,   193,   148,  1147,   189,
      30,   188,    74,   186,  1216,   187,   974,   188,   956,   977,
     958,  1431,    46,  1225,    75,    76,    77,    78,   173,   132,
      30,   187,   108,   807,   187,   132,     9,   113,   187,   115,
     116,   117,   118,   119,   120,   121,   190,  1760,    99,  1251,
       9,   825,   187,   190,  1767,   189,    14,    80,   186,  1120,
       9,   187,   187,  1124,  1403,   188,   840,   841,   960,   961,
     187,   186,   483,   189,  1135,   187,   132,  1352,   852,   187,
    1355,   188,   158,   159,   108,   161,  1147,   187,    30,   113,
     188,   115,   116,   117,   118,   119,   120,   121,   187,   873,
     187,   189,   188,   108,     4,   160,   182,   158,   159,   188,
     161,   162,   163,  1071,   190,  1073,   156,    14,    80,  1057,
     113,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   158,   159,  1406,   161,   132,   187,
     187,  1385,  1100,   187,   189,  1103,    46,   132,    14,    78,
    1211,    80,    81,   172,   189,   188,  1358,    80,   182,    14,
      14,    80,  1364,   189,  1366,   187,   190,  1369,   186,   132,
      99,    64,    65,  1448,   187,    14,  1378,   188,  1317,   188,
     954,    14,   956,  1593,   958,   188,   960,   961,    14,     9,
     189,  1083,   190,    56,  1333,    80,   172,  1155,   186,    80,
       9,  1159,   188,   111,  1265,   151,  1267,    98,   108,  1147,
      98,   163,    33,   113,  1628,   115,   116,   117,   118,   119,
     120,   121,    78,    14,   188,  1469,  1470,   187,   169,   158,
     159,   186,   161,   162,   163,   646,  1128,   130,   131,   186,
     173,  1380,    80,    99,   115,   116,   117,   118,   119,   120,
    1389,  1143,  1144,   166,  1212,  1213,  1317,   187,   158,   159,
    1462,   161,  1323,     9,    80,   187,    26,    27,  1329,   187,
      30,   188,    47,    48,    49,    50,    51,    52,  1216,     4,
     189,    14,   182,  1057,    80,  1588,    14,  1225,    80,   811,
     190,    66,   703,    14,    54,    80,    14,    80,   561,   155,
    1439,  1709,   158,   159,   815,   161,   162,   163,  1723,  1083,
     452,   182,  1651,  1374,   450,   455,  1738,   909,  1588,  1380,
     855,    46,  1141,  1598,  1385,  1734,  1218,  1460,  1389,  1468,
     566,  1292,   743,  1470,   745,  1474,  1411,  1577,  1727,  1493,
    1479,  1299,  1403,  1301,  1329,  1406,  1766,  1754,  1409,  1325,
    1456,  1589,  1061,   994,  1128,   928,  1119,    29,  1419,  1120,
     943,  1135,   773,   879,   355,  1426,   400,   787,  1318,  1143,
    1144,  1691,  1433,  1147,  1435,  1044,  1334,  1027,   979,  1317,
    1441,    -1,    -1,   108,    -1,    -1,    -1,    -1,   113,    -1,
     115,   116,   117,   118,   119,   120,   121,    -1,    -1,    -1,
      -1,  1339,    -1,  1464,    -1,    -1,    78,  1468,  1469,  1470,
      -1,    -1,    -1,  1474,   825,    -1,    -1,    -1,  1479,    -1,
    1358,    -1,    -1,    -1,    -1,    -1,  1364,    99,  1366,   840,
     841,    -1,    -1,   158,   159,    -1,   161,    -1,    -1,    -1,
    1378,    -1,  1380,    -1,  1218,    -1,    -1,    -1,    -1,   121,
     210,  1389,    -1,   864,    -1,    -1,    -1,   182,    -1,    -1,
      -1,    -1,   134,   135,    -1,   190,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,  1618,
     152,    -1,    -1,   155,   156,    -1,   158,   159,    -1,   161,
     162,   163,    -1,    -1,    -1,    -1,    -1,  1772,    -1,   259,
      -1,   261,    -1,    -1,    -1,  1780,    -1,    -1,    -1,    -1,
      -1,  1786,    -1,  1715,  1789,    -1,    64,    65,    -1,  1658,
    1659,    -1,    -1,    -1,  1462,    -1,  1665,  1588,    -1,    -1,
    1468,    -1,  1734,  1491,    -1,    -1,  1474,    -1,    -1,    78,
    1742,  1479,    -1,  1317,     4,    -1,    -1,    78,  1609,   960,
     961,    -1,  1613,    -1,    -1,    -1,   316,  1618,    -1,    -1,
      99,    -1,    -1,  1702,    -1,  1626,    -1,    -1,    99,    -1,
      -1,    -1,  1633,  1634,    -1,    -1,  1637,  1638,   338,    -1,
      -1,    -1,   130,   131,    -1,   996,    46,   347,    -1,    -1,
    1651,    -1,    -1,    78,   354,    -1,    -1,  1658,  1659,    -1,
      -1,   361,    -1,    -1,  1665,    78,  1380,    -1,    -1,    -1,
      -1,  1385,   372,   152,    99,  1389,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    99,   158,   159,  1768,
     161,   162,   163,    -1,  1592,    -1,  1775,   397,    -1,   187,
     400,  1702,    -1,    -1,    -1,    -1,    -1,  1708,   108,    -1,
      -1,    -1,    -1,   113,    29,   115,   116,   117,   118,   119,
     120,   121,    -1,    -1,    -1,  1726,    -1,    -1,    -1,    -1,
     155,    -1,  1083,   158,   159,    -1,   161,   162,   163,    -1,
    1618,    56,   155,   443,    -1,   158,   159,    -1,   161,   162,
     163,    -1,    -1,    -1,  1468,  1469,  1470,    -1,   158,   159,
    1474,   161,    -1,    78,    -1,  1479,    -1,  1768,    -1,    -1,
      -1,    -1,  1123,    -1,  1775,    -1,  1674,  1128,    -1,    -1,
    1658,  1659,   182,   483,    99,    -1,    -1,  1665,    -1,    -1,
     190,    -1,  1143,  1144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   134,
     135,    75,    76,    77,  1702,    -1,    -1,    -1,    -1,    -1,
      -1,  1709,    -1,    87,    -1,   535,   536,   152,    -1,   539,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,  1747,
     165,    -1,    -1,    -1,    -1,    -1,    64,    65,    -1,    -1,
      -1,   176,    -1,    -1,  1762,    -1,    -1,  1218,    -1,    -1,
     570,   186,    -1,    -1,    26,    27,    -1,    -1,    30,    -1,
      -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
    1768,    -1,    -1,   147,    -1,    -1,    -1,  1775,    -1,   153,
     154,    -1,    -1,    -1,  1618,    47,    48,    49,    50,    51,
      52,    -1,    54,   167,    -1,    -1,   616,   617,    -1,    -1,
      -1,    -1,   130,   131,    66,   625,    -1,   181,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,  1658,  1659,   646,    -1,    -1,    -1,
      -1,  1665,    -1,    -1,    -1,    29,  1307,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,   187,
      54,    29,    -1,    -1,    -1,    -1,    -1,    -1,  1702,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    -1,   703,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1376,    -1,    56,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,   743,    -1,   745,    -1,    -1,    -1,    78,
      -1,    99,    -1,    -1,  1768,    64,    65,    -1,   210,    -1,
      -1,  1775,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,   773,   774,    -1,    -1,    -1,   107,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   787,   788,   789,
     790,   791,   792,   793,    -1,    -1,    -1,   797,    -1,    -1,
      -1,    -1,    -1,    -1,   152,   134,   135,   155,   808,    -1,
     158,   159,    -1,   161,   162,   163,    -1,    -1,    -1,   193,
      -1,   130,   131,   152,    -1,   825,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,   838,    -1,
     840,   841,    -1,    -1,    -1,    -1,    -1,   176,   115,   116,
     117,   118,   119,   120,   854,   855,    -1,   186,    -1,   126,
     127,    -1,    -1,    -1,   864,    -1,    -1,    -1,    -1,    -1,
     870,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,   883,    -1,    -1,   338,    -1,    -1,    -1,
      -1,   891,    -1,    -1,   894,   347,    -1,   164,    -1,    -1,
      -1,    -1,   354,    -1,    -1,    -1,    10,    11,    12,   361,
      -1,    -1,   912,    -1,    -1,   182,   916,    -1,    -1,    -1,
     372,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     960,   961,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   974,    -1,    -1,   977,    -1,   979,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   443,    -1,    -1,   994,    -1,   996,    -1,    -1,   999,
    1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,    -1,    10,    11,
      12,   483,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1041,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,  1071,    54,  1073,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    48,  1083,    66,    -1,   190,   539,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    -1,    54,    -1,    -1,
    1100,    67,    -1,  1103,    -1,    -1,    -1,    -1,    -1,    75,
      76,    77,    78,    99,    -1,    -1,    -1,    -1,   570,    -1,
      -1,    87,    -1,  1123,    -1,    -1,    -1,    -1,  1128,    -1,
      -1,    -1,    -1,    99,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1143,  1144,    -1,  1146,    -1,   134,   135,
      -1,    -1,    -1,    -1,    -1,  1155,    -1,    -1,    -1,  1159,
      -1,    -1,  1162,    -1,  1164,    -1,   152,    -1,   134,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,    -1,
    1180,   147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   158,   159,   646,   161,   162,   163,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,
     176,    -1,  1212,  1213,    -1,  1215,    -1,    29,  1218,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,   703,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,   743,    -1,   745,    -1,    -1,    -1,    -1,    -1,  1299,
      66,  1301,   259,    -1,   261,    -1,  1306,  1307,    -1,    -1,
    1310,    -1,  1312,    -1,    -1,  1315,    -1,    -1,    -1,    -1,
      -1,   773,   774,  1323,  1324,    29,    -1,  1327,    -1,    -1,
      -1,    -1,    -1,    -1,  1334,    -1,    -1,   789,   790,   791,
     792,   793,    -1,    -1,    -1,   797,    -1,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    -1,    -1,   808,    -1,    -1,   316,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   825,    78,    -1,  1376,    -1,   190,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   838,    -1,   840,   841,
      -1,    -1,  1392,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,  1401,  1402,   855,    -1,    -1,    -1,    -1,    -1,  1409,
      -1,  1411,   864,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,    -1,    -1,
     134,   135,    -1,  1433,    -1,  1435,    -1,    -1,    -1,   891,
     397,  1441,   894,   400,    -1,    -1,    -1,    -1,   152,    -1,
      -1,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
      -1,   165,    -1,    -1,   916,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,    -1,  1477,  1478,    -1,
      -1,    -1,   186,  1483,    -1,  1485,    -1,    -1,    -1,    -1,
      -1,  1491,    -1,  1493,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   960,   961,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   994,    66,   996,    64,    65,   999,  1000,  1001,
    1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,
    1012,  1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,
    1022,  1023,  1024,  1025,  1026,    -1,    -1,    -1,   535,   536,
      -1,    -1,   539,    -1,    -1,    -1,    -1,    -1,    -1,  1041,
      -1,    -1,  1592,    -1,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1609,
      -1,   130,   131,   570,    -1,    -1,    -1,    26,    27,    -1,
      -1,    30,    -1,    -1,    56,    -1,  1626,    -1,    -1,    -1,
      -1,  1083,  1632,   115,   116,   117,   118,   119,   120,    -1,
      -1,    -1,    -1,  1643,   126,   127,    78,    -1,    -1,  1649,
      -1,    -1,    -1,  1653,    -1,    -1,    -1,    -1,    -1,   616,
     617,    -1,    -1,    -1,    -1,    -1,    -1,    99,   625,    -1,
      -1,  1123,    -1,    -1,  1674,    -1,  1128,    -1,    -1,    -1,
     162,    -1,   164,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1143,  1144,    -1,  1146,   177,    -1,   179,    -1,    -1,
     182,    -1,   134,   135,    -1,    -1,    -1,    -1,    -1,    -1,
    1162,    -1,  1164,  1713,    -1,    -1,    -1,    -1,    -1,    -1,
     152,    -1,  1722,   155,   156,    -1,   158,   159,  1180,   161,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,  1738,    -1,
      -1,    -1,    -1,    -1,   176,    -1,    -1,  1747,    10,    11,
      12,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1762,  1215,    -1,    -1,  1218,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   210,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   774,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     787,   788,   789,   790,   791,   792,   793,    -1,    -1,    -1,
     797,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,  1306,  1307,    -1,    -1,  1310,    -1,
    1312,    -1,    -1,  1315,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,  1324,    -1,    -1,  1327,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,   854,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,   870,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1376,    -1,   883,    -1,   190,   338,
      -1,    -1,    -1,    -1,   891,    -1,    -1,    -1,   347,    -1,
    1392,    -1,    -1,    -1,    -1,   354,    10,    11,    12,  1401,
    1402,    -1,   361,    -1,    -1,   912,    -1,    -1,    -1,  1411,
      -1,    -1,    -1,   372,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,   974,    54,    -1,
     977,    -1,   979,    -1,    -1,  1477,  1478,    -1,    -1,    -1,
      66,  1483,    -1,  1485,   443,    -1,    -1,   994,    -1,    -1,
      -1,  1493,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,
    1017,  1018,  1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,
      -1,    10,    11,    12,   483,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1041,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,  1071,    54,  1073,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   190,    66,    -1,    -1,
     539,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1100,    -1,    -1,  1103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,   570,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,    -1,
    1632,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1146,
      99,  1643,    -1,    -1,    -1,    -1,    -1,  1649,  1155,    -1,
      -1,  1653,  1159,    -1,    -1,  1162,    -1,  1164,    -1,    -1,
      -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1180,    -1,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,   646,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,   190,   161,   162,   163,  1212,  1213,    -1,   167,    -1,
      -1,  1713,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1722,    -1,   181,    -1,    10,    11,    12,   186,    -1,    -1,
     189,    -1,   191,    -1,    -1,    -1,  1738,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   703,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,   743,    -1,   745,    -1,    -1,    -1,
      -1,    -1,  1299,    -1,  1301,    -1,    -1,    -1,    -1,  1306,
      -1,    -1,    -1,  1310,    -1,  1312,    -1,    -1,  1315,    -1,
      -1,    -1,    -1,    -1,   773,   774,  1323,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,  1334,    -1,    -1,
     789,   790,   791,   792,   793,    -1,   539,    29,   797,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    29,    -1,    -1,   825,   570,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,   840,   841,    -1,    -1,  1392,    -1,    -1,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1409,    -1,   190,   864,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1433,    -1,  1435,    -1,
      -1,    -1,   891,    99,  1441,    -1,    -1,    -1,    -1,    -1,
      -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
     116,   117,   118,   119,   120,    -1,    -1,   916,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,   135,
    1477,  1478,    -1,    -1,    -1,    -1,  1483,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1491,    -1,   152,    -1,    -1,   155,
     156,    -1,   158,   159,    -1,   161,   162,   163,   190,    -1,
      -1,   960,   961,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,    -1,   182,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   994,    -1,   996,    -1,    -1,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,  1013,  1014,  1015,  1016,  1017,  1018,
    1019,  1020,  1021,  1022,  1023,  1024,  1025,  1026,    -1,    -1,
      -1,   774,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1041,    -1,    -1,  1592,   789,   790,   791,   792,
      -1,    -1,    -1,    -1,   797,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1609,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1626,
      -1,    -1,    -1,    -1,  1083,  1632,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1643,    -1,    -1,    -1,
      -1,    -1,  1649,    67,    -1,    -1,  1653,    -1,    -1,    -1,
      -1,    75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    87,  1123,    -1,    -1,  1674,    -1,  1128,
      -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1143,  1144,    -1,  1146,   891,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,  1162,    -1,  1164,  1713,    -1,    -1,   133,
     134,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,  1180,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
    1747,    -1,    -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1762,    -1,   181,    -1,  1218,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   999,  1000,  1001,  1002,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1041,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,  1306,  1307,    -1,
      -1,  1310,    -1,  1312,    -1,    -1,  1315,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,  1376,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,  1392,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,  1146,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1162,
      -1,  1164,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,  1180,    -1,    -1,
      -1,    -1,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    80,    -1,    -1,  1477,  1478,
     190,    -1,    87,    -1,  1483,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1492,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     115,   116,   117,   118,   119,   120,    -1,    -1,   123,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,   167,  1306,    -1,   170,    -1,  1310,    -1,  1312,
      -1,   176,  1315,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,   186,    10,    11,    12,    -1,   191,   192,    -1,   194,
     195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    -1,  1632,    10,    11,    12,    -1,    66,    -1,
      -1,    -1,    -1,    -1,  1643,    -1,    -1,    -1,    -1,  1392,
    1649,    -1,    -1,    29,  1653,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,  1676,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,   132,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1713,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1477,  1478,    46,    47,    48,    -1,
    1483,    -1,    -1,    53,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,    -1,    82,    83,    -1,    -1,    -1,    87,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,
     100,    -1,    -1,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   187,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,   139,
     140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,    -1,   158,   159,
      -1,   161,   162,   163,   164,    -1,    -1,   167,    -1,    -1,
     170,    -1,    -1,    -1,    -1,    -1,   176,   177,    -1,   179,
      -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,   189,
     190,   191,   192,    -1,   194,   195,    -1,    -1,    -1,  1632,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
    1643,    -1,    13,    -1,    -1,    -1,  1649,    -1,    -1,    -1,
    1653,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,    -1,
      -1,    -1,    53,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    67,    68,    69,    70,
      71,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
    1713,    82,    83,    -1,    -1,    -1,    87,    88,    89,    90,
      -1,    92,    -1,    94,    -1,    96,    -1,    -1,    99,   100,
      -1,    -1,    -1,   104,   105,   106,   107,   108,   109,   110,
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
      -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,   188,
      -1,    -1,   191,   192,    -1,   194,   195,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    54,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    47,    48,    -1,    -1,    -1,    -1,    53,    -1,    55,
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
     156,    -1,   158,   159,    -1,   161,   162,   163,    -1,   165,
      -1,   167,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,
     176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,    -1,   194,   195,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
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
     183,    -1,    -1,   186,    -1,    -1,   189,    -1,   191,   192,
      -1,   194,   195,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,   181,   182,   183,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,    -1,   194,   195,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
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
      -1,    -1,    -1,   167,    -1,    -1,   170,    -1,    -1,   188,
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,    -1,
     194,   195,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    47,    48,    -1,    -1,
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
     188,    11,    12,   191,   192,    -1,   194,   195,     3,     4,
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
      -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,    -1,
      -1,   186,    -1,   188,    -1,    -1,   191,   192,    -1,   194,
     195,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
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
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   115,   116,   117,   118,
     119,   120,    -1,    -1,   123,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,    -1,    -1,    -1,   167,    -1,
      -1,   170,    -1,    -1,   188,    -1,    -1,   176,    -1,    -1,
      -1,    -1,   181,   182,   183,    -1,    -1,   186,   187,    -1,
      -1,    -1,   191,   192,    -1,   194,   195,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
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
      13,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    47,    48,    -1,    -1,    -1,    -1,
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
      -1,    -1,   176,    -1,    -1,    -1,    -1,   181,   182,   183,
      -1,    -1,   186,    -1,    10,    11,    12,   191,   192,    -1,
     194,   195,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     158,   159,    -1,   161,   162,   163,    -1,     3,     4,   167,
       6,     7,   170,    -1,    10,    11,    12,    13,   176,    -1,
      -1,    -1,    -1,   181,   182,   183,    -1,    -1,   186,    -1,
      -1,    27,    -1,   191,   192,    -1,   194,   195,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    -1,    -1,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,
      -1,    -1,    68,    69,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
     126,   127,   128,   129,    -1,    -1,    -1,   133,   134,   135,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,
      -1,    -1,   158,   159,    27,   161,   162,   163,   164,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    -1,   189,    -1,   191,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,   126,   127,   128,   129,    -1,    -1,    -1,
     133,   134,   135,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,   152,
      -1,    -1,    -1,    -1,    -1,   158,   159,    -1,   161,   162,
     163,   164,    27,   166,    29,    -1,   169,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   189,    -1,   191,    54,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    69,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,    -1,   127,   128,   129,    -1,    -1,    -1,   133,   134,
     135,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,   152,    -1,    -1,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,   164,
      27,   166,    29,    -1,   169,    -1,    -1,    -1,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,   190,    -1,    54,    -1,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    69,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,
     127,   128,   129,    -1,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,   152,    -1,    -1,   155,   156,
      -1,   158,   159,    -1,   161,   162,   163,   164,    27,   166,
      29,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,   190,    -1,    54,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      69,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,   126,   127,   128,
     129,    -1,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,   152,    -1,    -1,   155,   156,    -1,   158,
     159,    -1,   161,   162,   163,   164,    27,   166,    29,    -1,
     169,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,    -1,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,    -1,    -1,    -1,   127,   128,   129,    -1,
      -1,    -1,   133,   134,   135,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,    -1,    10,    11,    12,
      13,   152,    -1,    -1,   155,   156,    -1,   158,   159,    -1,
     161,   162,   163,   164,    27,   166,    29,    -1,   169,    -1,
      -1,    -1,    -1,    -1,    -1,   176,   177,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,    -1,   127,   128,   129,    -1,    -1,    -1,
     133,   134,   135,    75,    76,    77,    78,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,   152,
      -1,    -1,   155,   156,    -1,   158,   159,    99,   161,   162,
     163,   164,    -1,   166,    -1,    -1,   169,     3,     4,     5,
       6,     7,    -1,   176,    10,    11,    12,    13,   120,    -1,
      -1,    -1,    -1,   186,   126,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,    -1,   158,   159,    54,   161,
     162,   163,    -1,    -1,    -1,   167,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    69,    70,    71,    72,    73,    74,   181,
      -1,    -1,    78,    -1,   186,    -1,    -1,    -1,    -1,   191,
      -1,    -1,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,   133,   134,    -1,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   152,   153,   154,    -1,
      -1,    -1,   158,   159,    -1,   161,   162,   163,   164,    -1,
     166,   167,    -1,   169,    10,    11,    12,    -1,    -1,    -1,
     176,   177,    -1,   179,    -1,   181,   182,    -1,    -1,    -1,
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
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,   188,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,    66,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    88,    89,    90,    91,    92,    93,
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
      -1,    -1,    -1,    -1,    -1,    -1,   134,    66,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    47,    48,    -1,   167,
      -1,    -1,    53,    -1,    55,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,   181,    -1,    -1,    67,    -1,   186,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    10,    11,
      12,    -1,    -1,   132,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    99,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,    66,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    67,
     161,   162,   163,    -1,    -1,    -1,   167,    75,    76,    77,
      78,    -1,    80,    -1,    -1,   176,    -1,    -1,    -1,    87,
     181,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    -1,   161,   162,   163,    67,    -1,    69,   167,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    -1,    80,
      -1,    -1,    -1,   181,    -1,    -1,    87,    -1,   186,    -1,
      -1,    -1,    -1,   191,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   134,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    67,
     161,   162,   163,    -1,    -1,    -1,   167,    75,    76,    77,
      78,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    87,
     181,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,    -1,
     158,   159,    67,   161,   162,   163,    -1,    -1,    -1,   167,
      75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    87,   181,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   134,
      -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,    -1,   158,   159,    -1,   161,   162,   163,    -1,
      -1,    -1,   167,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    29,   191,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      54,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   132,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
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
      45,    46,    47,    48,    49,    50,    51,    52,    -1,    -1,
      -1,    75,    76,    77,    78,    -1,    80,    -1,    -1,    -1,
      -1,    66,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,   132,   152,   153,
     154,   155,   156,    -1,   158,   159,    -1,   161,   162,   163,
      -1,    -1,    -1,   167,    75,    76,    77,    78,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    87,   181,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,    -1,   158,   159,    -1,
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
      47,    48,    49,    50,    51,    52,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    66,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    66
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
     436,   438,   440,   118,   119,   120,   133,   152,   162,   186,
     203,   235,   316,   335,   413,   335,   186,   335,   335,   335,
     104,   335,   335,   399,   400,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,    80,    87,   120,
     147,   186,   213,   354,   371,   374,   379,   413,   416,   413,
      35,   335,   427,   428,   335,   120,   126,   186,   213,   248,
     371,   372,   373,   375,   379,   410,   411,   412,   420,   424,
     425,   186,   326,   376,   186,   326,   345,   327,   335,   221,
     326,   186,   186,   186,   326,   188,   335,   203,   188,   335,
       3,     4,     6,     7,    10,    11,    12,    13,    27,    29,
      54,    56,    68,    69,    70,    71,    72,    73,    74,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   126,   127,   128,   129,   133,   134,   135,
     152,   156,   164,   166,   169,   176,   186,   203,   204,   205,
     215,   441,   456,   457,   459,   188,   332,   335,   189,   228,
     335,   107,   108,   155,   206,   209,   212,    80,   191,   282,
     283,   119,   126,   118,   126,    80,   284,   186,   186,   186,
     186,   203,   254,   444,   186,   186,   327,    80,    86,   148,
     149,   150,   433,   434,   155,   189,   212,   212,   203,   255,
     444,   156,   186,   444,   444,    80,   183,   189,   346,    27,
     325,   329,   335,   336,   413,   417,   217,   189,   422,    86,
     377,   433,    86,   433,   433,    30,   155,   172,   445,   186,
       9,   188,    35,   234,   156,   253,   444,   120,   182,   235,
     317,   188,   188,   188,   188,   188,   188,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    54,    66,   188,    67,    67,   189,   151,
     127,   162,   164,   177,   179,   256,   315,   316,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    64,    65,   130,   131,   403,    67,   189,   408,   186,
     186,    67,   189,   186,   234,   235,    14,   335,   188,   132,
      45,   203,   398,    86,   325,   336,   151,   413,   132,   193,
       9,   384,   249,   325,   336,   413,   445,   151,   186,   378,
     403,   408,   187,   335,    30,   219,     8,   347,     9,   188,
     219,   220,   327,   328,   335,   203,   268,   223,   188,   188,
     188,   134,   135,   459,   459,   172,   186,   107,   459,    14,
     151,   134,   135,   152,   203,   205,   188,   188,   229,   111,
     169,   188,   155,   207,   210,   212,   155,   208,   211,   212,
     212,     9,   188,    98,   189,   413,     9,   188,   126,   126,
      14,     9,   188,   413,   437,   327,   325,   336,   413,   416,
     417,   187,   172,   246,   133,   413,   426,   427,   188,    67,
     403,   148,   434,    79,   335,   413,    86,   148,   434,   212,
     202,   188,   189,   241,   251,   361,   363,    87,   186,   348,
     349,   351,   374,   419,   421,   438,    14,    98,   439,   342,
     343,   344,   278,   279,   401,   402,   187,   187,   187,   187,
     187,   190,   218,   219,   236,   243,   250,   401,   335,   192,
     194,   195,   203,   446,   447,   459,    35,   165,   280,   281,
     335,   441,   186,   444,   244,   234,   335,   335,   335,    30,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   375,   335,   335,   423,   423,   335,   429,
     430,   126,   189,   204,   205,   422,   254,   203,   255,   444,
     444,   253,   235,    35,   329,   332,   335,   335,   335,   335,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   156,
     189,   203,   404,   405,   406,   407,   422,   423,   335,   280,
     280,   423,   335,   426,   234,   187,   335,   186,   397,     9,
     384,   187,   187,    35,   335,    35,   335,   378,   187,   187,
     187,   420,   421,   422,   280,   189,   203,   404,   405,   422,
     187,   217,   272,   189,   332,   335,   335,    90,    30,   219,
     266,   188,    28,    98,    14,     9,   187,    30,   189,   269,
     459,    29,    87,   215,   453,   454,   455,   186,     9,    47,
      48,    53,    55,    67,   134,   156,   176,   186,   213,   215,
     356,   371,   379,   380,   381,   203,   458,   217,   186,   227,
     212,     9,   188,    98,   212,     9,   188,    98,    98,   209,
     203,   335,   283,   380,    80,     9,   187,   187,   187,   187,
     187,   187,   187,   188,    47,    48,   451,   452,   128,   259,
     186,     9,   187,   187,    80,    81,   203,   435,   203,    67,
     190,   190,   199,   201,    30,   129,   258,   171,    51,   156,
     171,   365,   336,   132,     9,   384,   187,   151,   459,   459,
      14,   347,   278,   217,   184,     9,   385,   459,   460,   403,
     408,   403,   190,     9,   384,   173,   413,   335,   187,     9,
     385,    14,   339,   237,   128,   257,   186,   444,   335,    30,
     193,   193,   132,   190,     9,   384,   335,   445,   186,   247,
     242,   252,    14,   439,   245,   234,    69,   413,   335,   445,
     193,   190,   187,   187,   193,   190,   187,    47,    48,    67,
      75,    76,    77,    87,   134,   147,   176,   203,   387,   389,
     390,   393,   396,   203,   413,   413,   132,   257,   403,   408,
     187,   335,   273,    72,    73,   274,   217,   326,   217,   328,
      98,    35,   133,   263,   413,   380,   203,    30,   219,   267,
     188,   270,   188,   270,     9,   173,    87,   132,   151,     9,
     384,   187,   165,   446,   447,   448,   446,   380,   380,   380,
     380,   380,   383,   386,   186,   151,   186,   380,   151,   189,
      10,    11,    12,    29,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    66,   151,   445,   190,
     371,   189,   231,    98,   210,   203,    98,   211,   203,   203,
     190,    14,   413,   188,     9,   173,   203,   260,   371,   189,
     426,   133,   413,    14,   193,   335,   190,   199,   459,   260,
     189,   364,    14,   187,   335,   348,   422,   188,   459,   184,
     190,    30,   449,   402,    35,    80,   165,   404,   405,   407,
     404,   405,   459,    35,   165,   335,   380,   278,   186,   371,
     258,   340,   238,   335,   335,   335,   190,   186,   280,   259,
      30,   258,   459,    14,   257,   444,   375,   190,   186,    14,
      75,    76,    77,   203,   388,   388,   390,   391,   392,    49,
     186,    86,   148,   186,     9,   384,   187,   397,    35,   335,
     258,   190,    72,    73,   275,   326,   219,   190,   188,    91,
     188,   263,   413,   186,   132,   262,    14,   217,   270,   101,
     102,   103,   270,   190,   459,   132,   459,   203,   453,     9,
     187,   384,   132,   193,     9,   384,   383,   204,   348,   350,
     352,   187,   126,   204,   380,   431,   432,   380,   380,   380,
      30,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   458,    80,   232,   203,   203,
     380,   452,    98,    99,   450,     9,   288,   187,   186,   329,
     332,   335,   193,   190,   439,   288,   157,   170,   189,   360,
     367,   157,   189,   366,   132,   188,   449,   459,   347,   460,
      80,   165,    14,    80,   445,   413,   335,   187,   278,   189,
     278,   186,   132,   186,   280,   187,   189,   459,   189,   188,
     459,   258,   239,   378,   280,   132,   193,     9,   384,   389,
     391,   148,   348,   394,   395,   390,   413,   189,   326,    30,
      74,   219,   188,   328,   262,   426,   263,   187,   380,    97,
     101,   188,   335,    30,   188,   271,   190,   173,   459,   132,
     165,    30,   187,   380,   380,   187,   132,     9,   384,   187,
     132,   190,     9,   384,   380,    30,   187,   217,   203,   459,
     459,   371,     4,   108,   113,   119,   121,   158,   159,   161,
     190,   289,   314,   315,   316,   321,   322,   323,   324,   401,
     426,   190,   189,   190,    51,   335,   335,   335,   347,    35,
      80,   165,    14,    80,   335,   186,   449,   187,   288,   187,
     278,   335,   280,   187,   288,   439,   288,   188,   189,   186,
     187,   390,   390,   187,   132,   187,     9,   384,   288,    30,
     217,   188,   187,   187,   187,   224,   188,   188,   271,   217,
     459,   459,   132,   380,   348,   380,   380,   380,   189,   190,
     450,   128,   129,   177,   204,   442,   459,   261,   371,   108,
     324,    29,   121,   134,   135,   156,   162,   298,   299,   300,
     301,   371,   160,   306,   307,   124,   186,   203,   308,   309,
     290,   235,   459,     9,   188,     9,   188,   188,   439,   315,
     187,   285,   156,   362,   190,   190,    80,   165,    14,    80,
     335,   280,   113,   337,   449,   190,   449,   187,   187,   190,
     189,   190,   288,   278,   132,   390,   348,   190,   217,   222,
     225,    30,   219,   265,   217,   187,   380,   132,   132,   217,
     371,   371,   444,    14,   204,     9,   188,   189,   442,   439,
     301,   172,   189,     9,   188,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    54,    68,    69,    70,
      71,    72,    73,    74,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   133,   134,   136,   137,   138,
     139,   140,   152,   153,   154,   164,   166,   167,   169,   176,
     177,   179,   181,   182,   203,   368,   369,     9,   188,   156,
     160,   203,   309,   310,   311,   188,    80,   320,   234,   291,
     442,   442,    14,   235,   190,   286,   287,   442,    14,    80,
     335,   187,   186,   189,   188,   189,   312,   337,   449,   285,
     190,   187,   390,   132,    30,   219,   264,   265,   217,   380,
     380,   190,   188,   188,   380,   371,   294,   459,   302,   303,
     379,   299,    14,    30,    48,   304,   307,     9,    33,   187,
      29,    47,    50,    14,     9,   188,   205,   443,   320,    14,
     459,   234,   188,    14,   335,    35,    80,   359,   217,   217,
     189,   312,   190,   449,   390,   217,    95,   230,   190,   203,
     215,   295,   296,   297,     9,   173,     9,   384,   190,   380,
     369,   369,    56,   305,   310,   310,    29,    47,    50,   380,
      80,   172,   186,   188,   380,   444,   380,    80,     9,   385,
     190,   190,   217,   312,    93,   188,   111,   226,   151,    98,
     459,   379,   163,    14,   451,   292,   186,    35,    80,   187,
     190,   188,   186,   169,   233,   203,   315,   316,   173,   380,
     173,   276,   277,   402,   293,    80,   371,   231,   166,   203,
     188,   187,     9,   385,   115,   116,   117,   318,   319,   276,
      80,   261,   188,   449,   402,   460,   187,   187,   188,   188,
     189,   313,   318,    35,    80,   165,   449,   189,   217,   460,
      80,   165,    14,    80,   313,   217,   190,    35,    80,   165,
      14,    80,   335,   190,    80,   165,    14,    80,   335,    14,
      80,   335,   335
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
#line 2638 "hphp.y"
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
#line 2653 "hphp.y"
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
#line 2788 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 2864 "hphp.y"
    { (yyvsp[(1) - (2)]) = 1; _p->onIndirectRef((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { (yyval).reset();;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 861:

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

  case 862:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 867:

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

  case 868:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { (yyval).reset();;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2937 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2983 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 903:

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

  case 904:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
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
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3036 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3050 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(1) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3071 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3092 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3096 "hphp.y"
    { Token t; _p->setTypeVars(t, (yyvsp[(1) - (4)]));
                                         _p->pushTypeScope(); (yyval) = t; ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3124 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3129 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3130 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3142 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3146 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    {;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3154 "hphp.y"
    {;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3155 "hphp.y"
    {;}
    break;

  case 959:

/* Line 1455 of yacc.c  */
#line 3161 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 960:

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

  case 961:

/* Line 1455 of yacc.c  */
#line 3177 "hphp.y"
    { _p->onClsCnsShapeField((yyval), (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 962:

/* Line 1455 of yacc.c  */
#line 3182 "hphp.y"
    { _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 963:

/* Line 1455 of yacc.c  */
#line 3183 "hphp.y"
    { ;}
    break;

  case 964:

/* Line 1455 of yacc.c  */
#line 3188 "hphp.y"
    { _p->onShape((yyval), (yyvsp[(1) - (2)])); ;}
    break;

  case 965:

/* Line 1455 of yacc.c  */
#line 3189 "hphp.y"
    { Token t; t.reset();
                                         _p->onShape((yyval), t); ;}
    break;

  case 966:

/* Line 1455 of yacc.c  */
#line 3195 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);
                                        (yyval).setText("array"); ;}
    break;

  case 967:

/* Line 1455 of yacc.c  */
#line 3200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 968:

/* Line 1455 of yacc.c  */
#line 3205 "hphp.y"
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                        _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 969:

/* Line 1455 of yacc.c  */
#line 3209 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 970:

/* Line 1455 of yacc.c  */
#line 3216 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 971:

/* Line 1455 of yacc.c  */
#line 3219 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 972:

/* Line 1455 of yacc.c  */
#line 3222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 973:

/* Line 1455 of yacc.c  */
#line 3223 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 974:

/* Line 1455 of yacc.c  */
#line 3226 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 975:

/* Line 1455 of yacc.c  */
#line 3229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 976:

/* Line 1455 of yacc.c  */
#line 3232 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 977:

/* Line 1455 of yacc.c  */
#line 3236 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 3239 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 979:

/* Line 1455 of yacc.c  */
#line 3242 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 980:

/* Line 1455 of yacc.c  */
#line 3248 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 981:

/* Line 1455 of yacc.c  */
#line 3254 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 982:

/* Line 1455 of yacc.c  */
#line 3262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 983:

/* Line 1455 of yacc.c  */
#line 3263 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13626 "hphp.7.tab.cpp"
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
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}

