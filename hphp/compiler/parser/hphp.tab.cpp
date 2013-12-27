/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.5"

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
#define yyparse         Compilerparse
#define yylex           Compilerlex
#define yyerror         Compilererror
#define yylval          Compilerlval
#define yychar          Compilerchar
#define yydebug         Compilerdebug
#define yynerrs         Compilernerrs
#define yylloc          Compilerlloc

/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 1 "hphp.y"

#ifdef XHPAST2_PARSER
#include "hphp/parser/xhpast2/parser.h"
#else
#include "hphp/compiler/parser/parser.h"
#endif
#include "hphp/util/util.h"
#include "hphp/util/logger.h"

// macros for bison
#define YYSTYPE HPHP::HPHP_PARSER_NS::Token
#define YYSTYPE_IS_TRIVIAL 1
#define YYLTYPE HPHP::Location
#define YYLTYPE_IS_TRIVIAL 1
#define YYERROR_VERBOSE
#define YYINITDEPTH 500
#define YYLEX_PARAM _p

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
    if (YYID (N)) {                                                     \
      (Current).first(YYRHSLOC (Rhs, 1));                               \
      (Current).last (YYRHSLOC (Rhs, N));                               \
    } else {                                                            \
      (Current).line0 = (Current).line1 = YYRHSLOC (Rhs, 0).line1;      \
      (Current).char0 = (Current).char1 = YYRHSLOC (Rhs, 0).char1;      \
    }                                                                   \
  while (YYID (0));                                                     \
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
  while (YYID (0))

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
  while (YYID (0))

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
  while (YYID (0))

# define YYSTACK_RELOCATE_RESET(Stack_alloc, Stack)                     \
  do                                                                    \
    {                                                                   \
      YYSIZE_T yynewbytes;                                              \
      YYCOPY_RESET (&yyptr->Stack_alloc, Stack, yysize);                \
      Stack = &yyptr->Stack_alloc;                                      \
      yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
      yyptr += yynewbytes / sizeof (*yyptr);                            \
    }                                                                   \
  while (YYID (0))

#define YYSTACK_CLEANUP                         \
  YYTOKEN_RESET (yyvs, yystacksize);            \
  if (yyvs != yyvsa) {                          \
    YYSTACK_FREE (yyvs);                        \
  }                                             \
  if (yyls != yylsa) {                          \
    YYSTACK_FREE (yyls);                        \
  }                                             \


// macros for rules
#define BEXP(e...) _p->onBinaryOpExp(e);
#define UEXP(e...) _p->onUnaryOpExp(e);

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
  t.setText(boost::lexical_cast<std::string>(num));
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
// converting constant declartion to "define(name, value);"

static void on_constant(Parser *_p, Token &out, Token &name, Token &value) {
  Token sname;   _p->onScalar(sname, T_CONSTANT_ENCAPSED_STRING, name);

  Token fname;   fname.setText("define");
  Token params1; _p->onCallParam(params1, NULL, sname, 0);
  Token params2; _p->onCallParam(params2, &params1, value, 0);
  Token call;    _p->onCall(call, 0, fname, params2, 0);

  _p->onExpStatement(out, call);
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
   * The basic builtin types "bool", "int", "double", and "string" all map to
   * T_STRING in the parser, and the parser always uses type code 5 for
   * T_STRING. However, XHP uses different type codes for these basic builtin
   * types, so we need to fix up the type code here to make XHP happy.
   */
  if (type.num() == 5 && type.text().size() >= 3 && type.text().size() <= 7) {
    switch (type.text()[0]) {
      case 'b':
        if ((type.text().size() == 4 &&
             strcasecmp(type.text().c_str(), "bool") == 0) ||
            (type.text().size() == 7 &&
             strcasecmp(type.text().c_str(), "boolean") == 0)) {
          type.reset();
          type.setNum(2);
        }
        break;
      case 'd':
        if (type.text().size() == 6 &&
            strcasecmp(type.text().c_str(), "double") == 0) {
          type.reset();
          type.setNum(8);
        }
        break;
      case 'f':
        if (type.text().size() == 5 &&
            strcasecmp(type.text().c_str(), "float") == 0) {
          type.reset();
          type.setNum(8);
        }
        break;
      case 'i':
        if ((type.text().size() == 3 &&
             strcasecmp(type.text().c_str(), "int") == 0) ||
            (type.text().size() == 7 &&
             strcasecmp(type.text().c_str(), "integer") == 0)) {
          type.reset();
          type.setNum(3);
        }
        break;
      case 'm':
        if ((type.text().size() == 5 &&
             strcasecmp(type.text().c_str(), "mixed") == 0)) {
          type.reset();
          type.setNum(6);
        }
        break;
      case 'r':
        if (type.text().size() == 4 &&
            strcasecmp(type.text().c_str(), "real") == 0) {
          type.reset();
          type.setNum(8);
        }
        break;
      case 's':
        if (type.text().size() == 6 &&
            strcasecmp(type.text().c_str(), "string") == 0) {
          type.reset();
          type.setNum(1);
        }
        break;
      default:
        break;
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
  HPHP::Util::split(':', attributes.text().c_str(), classes, true);
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
    Token params1; _p->onCallParam(params1, NULL, param1, 0);

    for (unsigned int i = 0; i < classes.size(); i++) {
      Token parent;  parent.set(T_STRING, classes[i]);
      Token cls;     _p->onName(cls, parent, Parser::StringName);
      Token fname;   fname.setText("__xhpAttributeDeclaration");
      Token param;   _p->onCall(param, 0, fname, dummy, &cls);

      Token params; _p->onCallParam(params, &params1, param, 0);
      params1 = params;
    }

    Token params2; _p->onCallParam(params2, &params1, arrAttributes, 0);

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
      "Syntax only allowed with -v Eval.EnableHipHopSyntax=true", _p);
  }
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


/* Line 268 of yacc.c  */
#line 637 "hphp.tab.cpp"

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
     T_SR_EQUAL = 268,
     T_SL_EQUAL = 269,
     T_XOR_EQUAL = 270,
     T_OR_EQUAL = 271,
     T_AND_EQUAL = 272,
     T_MOD_EQUAL = 273,
     T_CONCAT_EQUAL = 274,
     T_DIV_EQUAL = 275,
     T_MUL_EQUAL = 276,
     T_MINUS_EQUAL = 277,
     T_PLUS_EQUAL = 278,
     T_BOOLEAN_OR = 279,
     T_BOOLEAN_AND = 280,
     T_IS_NOT_IDENTICAL = 281,
     T_IS_IDENTICAL = 282,
     T_IS_NOT_EQUAL = 283,
     T_IS_EQUAL = 284,
     T_IS_GREATER_OR_EQUAL = 285,
     T_IS_SMALLER_OR_EQUAL = 286,
     T_SR = 287,
     T_SL = 288,
     T_INSTANCEOF = 289,
     T_UNSET_CAST = 290,
     T_BOOL_CAST = 291,
     T_OBJECT_CAST = 292,
     T_ARRAY_CAST = 293,
     T_STRING_CAST = 294,
     T_DOUBLE_CAST = 295,
     T_INT_CAST = 296,
     T_DEC = 297,
     T_INC = 298,
     T_CLONE = 299,
     T_NEW = 300,
     T_EXIT = 301,
     T_IF = 302,
     T_ELSEIF = 303,
     T_ELSE = 304,
     T_ENDIF = 305,
     T_LNUMBER = 306,
     T_DNUMBER = 307,
     T_STRING = 308,
     T_STRING_VARNAME = 309,
     T_VARIABLE = 310,
     T_NUM_STRING = 311,
     T_INLINE_HTML = 312,
     T_CHARACTER = 313,
     T_BAD_CHARACTER = 314,
     T_ENCAPSED_AND_WHITESPACE = 315,
     T_CONSTANT_ENCAPSED_STRING = 316,
     T_ECHO = 317,
     T_DO = 318,
     T_WHILE = 319,
     T_ENDWHILE = 320,
     T_FOR = 321,
     T_ENDFOR = 322,
     T_FOREACH = 323,
     T_ENDFOREACH = 324,
     T_DECLARE = 325,
     T_ENDDECLARE = 326,
     T_AS = 327,
     T_SWITCH = 328,
     T_ENDSWITCH = 329,
     T_CASE = 330,
     T_DEFAULT = 331,
     T_BREAK = 332,
     T_GOTO = 333,
     T_CONTINUE = 334,
     T_FUNCTION = 335,
     T_CONST = 336,
     T_RETURN = 337,
     T_TRY = 338,
     T_CATCH = 339,
     T_THROW = 340,
     T_USE = 341,
     T_GLOBAL = 342,
     T_PUBLIC = 343,
     T_PROTECTED = 344,
     T_PRIVATE = 345,
     T_FINAL = 346,
     T_ABSTRACT = 347,
     T_STATIC = 348,
     T_VAR = 349,
     T_UNSET = 350,
     T_ISSET = 351,
     T_EMPTY = 352,
     T_HALT_COMPILER = 353,
     T_CLASS = 354,
     T_INTERFACE = 355,
     T_EXTENDS = 356,
     T_IMPLEMENTS = 357,
     T_OBJECT_OPERATOR = 358,
     T_DOUBLE_ARROW = 359,
     T_LIST = 360,
     T_ARRAY = 361,
     T_CLASS_C = 362,
     T_METHOD_C = 363,
     T_FUNC_C = 364,
     T_LINE = 365,
     T_FILE = 366,
     T_COMMENT = 367,
     T_DOC_COMMENT = 368,
     T_OPEN_TAG = 369,
     T_OPEN_TAG_WITH_ECHO = 370,
     T_CLOSE_TAG = 371,
     T_WHITESPACE = 372,
     T_START_HEREDOC = 373,
     T_END_HEREDOC = 374,
     T_DOLLAR_OPEN_CURLY_BRACES = 375,
     T_CURLY_OPEN = 376,
     T_DOUBLE_COLON = 377,
     T_NAMESPACE = 378,
     T_NS_C = 379,
     T_DIR = 380,
     T_NS_SEPARATOR = 381,
     T_YIELD = 382,
     T_XHP_LABEL = 383,
     T_XHP_TEXT = 384,
     T_XHP_ATTRIBUTE = 385,
     T_XHP_CATEGORY = 386,
     T_XHP_CATEGORY_LABEL = 387,
     T_XHP_CHILDREN = 388,
     T_XHP_ENUM = 389,
     T_XHP_REQUIRED = 390,
     T_TRAIT = 391,
     T_INSTEADOF = 392,
     T_TRAIT_C = 393,
     T_VARARG = 394,
     T_HH_ERROR = 395,
     T_FINALLY = 396,
     T_XHP_TAG_LT = 397,
     T_XHP_TAG_GT = 398,
     T_TYPELIST_LT = 399,
     T_TYPELIST_GT = 400,
     T_UNRESOLVED_LT = 401,
     T_COLLECTION = 402,
     T_SHAPE = 403,
     T_TYPE = 404,
     T_UNRESOLVED_TYPE = 405,
     T_NEWTYPE = 406,
     T_UNRESOLVED_NEWTYPE = 407,
     T_COMPILER_HALT_OFFSET = 408,
     T_AWAIT = 409,
     T_ASYNC = 410,
     T_TUPLE = 411,
     T_FROM = 412,
     T_WHERE = 413,
     T_JOIN = 414,
     T_IN = 415,
     T_ON = 416,
     T_EQUALS = 417,
     T_INTO = 418,
     T_LET = 419,
     T_ORDERBY = 420,
     T_ASCENDING = 421,
     T_DESCENDING = 422,
     T_SELECT = 423,
     T_GROUP = 424,
     T_BY = 425,
     T_LAMBDA_OP = 426,
     T_LAMBDA_CP = 427,
     T_UNRESOLVED_OP = 428
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


/* Line 343 of yacc.c  */
#line 865 "hphp.tab.cpp"

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
# if defined YYENABLE_NLS && YYENABLE_NLS
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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

# define YYCOPY_NEEDED 1

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

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
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
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   14553

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  203
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  246
/* YYNRULES -- Number of rules.  */
#define YYNRULES  817
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1527

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   428

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    49,   201,     2,   198,    48,    32,   202,
     193,   194,    46,    43,     9,    44,    45,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    27,   195,
      37,    14,    38,    26,    52,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    62,     2,   200,    31,     2,   199,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   196,    30,   197,    51,     2,     2,     2,
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
      17,    18,    19,    20,    21,    22,    23,    24,    25,    28,
      29,    33,    34,    35,    36,    39,    40,    41,    42,    50,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
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
     184,   185,   186,   187,   188,   189,   190,   191,   192
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    26,    30,    31,    38,    39,    45,    49,    52,
      54,    56,    58,    60,    62,    64,    66,    68,    70,    72,
      74,    76,    78,    80,    82,    84,    86,    88,    90,    92,
      96,    98,   100,   103,   107,   112,   114,   118,   120,   124,
     127,   129,   132,   135,   141,   146,   149,   150,   152,   154,
     156,   158,   162,   168,   177,   178,   183,   184,   191,   192,
     203,   204,   209,   212,   216,   219,   223,   226,   230,   234,
     238,   242,   246,   252,   254,   256,   257,   267,   273,   274,
     288,   289,   295,   299,   303,   306,   309,   312,   315,   318,
     321,   325,   328,   331,   335,   338,   339,   344,   354,   355,
     356,   361,   364,   365,   367,   368,   370,   371,   381,   382,
     393,   394,   406,   407,   416,   417,   427,   428,   436,   437,
     446,   447,   455,   456,   465,   467,   469,   471,   473,   475,
     478,   481,   484,   485,   488,   489,   492,   493,   495,   499,
     501,   505,   508,   509,   511,   514,   519,   521,   526,   528,
     533,   535,   540,   542,   547,   551,   557,   561,   566,   571,
     577,   583,   588,   589,   591,   593,   598,   599,   605,   606,
     609,   610,   614,   615,   619,   622,   624,   625,   630,   636,
     644,   651,   658,   666,   676,   685,   689,   692,   694,   695,
     699,   704,   711,   717,   723,   730,   739,   747,   750,   751,
     753,   756,   760,   765,   769,   771,   773,   776,   781,   785,
     791,   793,   797,   800,   801,   802,   807,   808,   814,   817,
     818,   829,   830,   842,   846,   850,   854,   859,   864,   868,
     874,   877,   880,   881,   888,   894,   899,   903,   905,   907,
     911,   916,   918,   920,   922,   924,   929,   931,   935,   938,
     939,   942,   943,   945,   949,   951,   953,   955,   957,   961,
     966,   971,   976,   978,   980,   983,   986,   989,   993,   997,
     999,  1001,  1003,  1005,  1009,  1011,  1015,  1017,  1019,  1021,
    1022,  1024,  1027,  1029,  1031,  1033,  1035,  1037,  1039,  1041,
    1043,  1044,  1046,  1048,  1050,  1054,  1060,  1062,  1066,  1072,
    1077,  1081,  1085,  1088,  1090,  1092,  1096,  1100,  1102,  1104,
    1105,  1108,  1113,  1117,  1124,  1127,  1131,  1138,  1140,  1142,
    1144,  1151,  1155,  1160,  1167,  1171,  1175,  1179,  1183,  1187,
    1191,  1195,  1199,  1203,  1207,  1211,  1214,  1217,  1220,  1223,
    1227,  1231,  1235,  1239,  1243,  1247,  1251,  1255,  1259,  1263,
    1267,  1271,  1275,  1279,  1283,  1287,  1290,  1293,  1296,  1299,
    1303,  1307,  1311,  1315,  1319,  1323,  1327,  1331,  1335,  1339,
    1345,  1350,  1352,  1355,  1358,  1361,  1364,  1367,  1370,  1373,
    1376,  1379,  1381,  1383,  1385,  1389,  1392,  1394,  1396,  1398,
    1404,  1405,  1406,  1418,  1419,  1432,  1433,  1437,  1438,  1445,
    1448,  1453,  1455,  1461,  1465,  1471,  1475,  1478,  1479,  1482,
    1483,  1488,  1493,  1497,  1502,  1507,  1512,  1517,  1519,  1521,
    1525,  1528,  1532,  1537,  1540,  1544,  1546,  1549,  1551,  1554,
    1556,  1558,  1560,  1562,  1564,  1566,  1571,  1576,  1579,  1588,
    1599,  1602,  1604,  1608,  1610,  1613,  1615,  1617,  1619,  1621,
    1624,  1629,  1633,  1637,  1642,  1644,  1647,  1652,  1655,  1662,
    1663,  1665,  1670,  1671,  1674,  1675,  1677,  1679,  1683,  1685,
    1689,  1691,  1693,  1697,  1701,  1703,  1705,  1707,  1709,  1711,
    1713,  1715,  1717,  1719,  1721,  1723,  1725,  1727,  1729,  1731,
    1733,  1735,  1737,  1739,  1741,  1743,  1745,  1747,  1749,  1751,
    1753,  1755,  1757,  1759,  1761,  1763,  1765,  1767,  1769,  1771,
    1773,  1775,  1777,  1779,  1781,  1783,  1785,  1787,  1789,  1791,
    1793,  1795,  1797,  1799,  1801,  1803,  1805,  1807,  1809,  1811,
    1813,  1815,  1817,  1819,  1821,  1823,  1825,  1827,  1829,  1831,
    1833,  1835,  1837,  1839,  1841,  1843,  1845,  1847,  1849,  1851,
    1853,  1855,  1857,  1859,  1861,  1866,  1868,  1870,  1872,  1874,
    1876,  1878,  1880,  1882,  1885,  1887,  1888,  1889,  1891,  1893,
    1897,  1898,  1900,  1902,  1904,  1906,  1908,  1910,  1912,  1914,
    1916,  1918,  1920,  1922,  1926,  1929,  1931,  1933,  1936,  1939,
    1944,  1949,  1953,  1958,  1960,  1962,  1966,  1970,  1972,  1974,
    1976,  1978,  1982,  1986,  1990,  1993,  1994,  1996,  1997,  1999,
    2000,  2006,  2010,  2014,  2016,  2018,  2020,  2022,  2026,  2029,
    2031,  2033,  2035,  2037,  2039,  2042,  2045,  2050,  2055,  2059,
    2064,  2067,  2068,  2074,  2078,  2082,  2084,  2088,  2090,  2093,
    2094,  2100,  2104,  2107,  2108,  2112,  2113,  2118,  2121,  2122,
    2126,  2130,  2132,  2133,  2135,  2138,  2141,  2146,  2150,  2154,
    2157,  2162,  2165,  2170,  2172,  2174,  2176,  2178,  2180,  2183,
    2188,  2192,  2197,  2201,  2203,  2205,  2207,  2209,  2212,  2217,
    2222,  2226,  2228,  2230,  2234,  2242,  2249,  2258,  2268,  2277,
    2288,  2296,  2303,  2312,  2314,  2317,  2322,  2327,  2329,  2331,
    2336,  2338,  2339,  2341,  2344,  2346,  2348,  2351,  2356,  2360,
    2364,  2365,  2367,  2370,  2375,  2379,  2382,  2386,  2393,  2394,
    2396,  2401,  2404,  2405,  2411,  2415,  2419,  2421,  2428,  2433,
    2438,  2441,  2444,  2445,  2451,  2455,  2459,  2461,  2464,  2465,
    2471,  2475,  2479,  2481,  2484,  2487,  2489,  2492,  2494,  2499,
    2503,  2507,  2514,  2518,  2520,  2522,  2524,  2529,  2534,  2539,
    2544,  2547,  2550,  2555,  2558,  2561,  2563,  2567,  2571,  2572,
    2575,  2581,  2588,  2590,  2593,  2595,  2600,  2604,  2605,  2607,
    2611,  2615,  2617,  2619,  2620,  2621,  2624,  2628,  2630,  2636,
    2640,  2644,  2648,  2650,  2653,  2654,  2659,  2662,  2665,  2667,
    2669,  2671,  2676,  2683,  2685,  2694,  2700,  2702
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     204,     0,    -1,    -1,   205,   206,    -1,   206,   207,    -1,
      -1,   221,    -1,   237,    -1,   241,    -1,   246,    -1,   435,
      -1,   117,   193,   194,   195,    -1,   142,   213,   195,    -1,
      -1,   142,   213,   196,   208,   206,   197,    -1,    -1,   142,
     196,   209,   206,   197,    -1,   105,   211,   195,    -1,   218,
     195,    -1,    72,    -1,   149,    -1,   150,    -1,   152,    -1,
     154,    -1,   153,    -1,   175,    -1,   177,    -1,   178,    -1,
     180,    -1,   179,    -1,   181,    -1,   182,    -1,   183,    -1,
     184,    -1,   185,    -1,   186,    -1,   187,    -1,   188,    -1,
     189,    -1,   211,     9,   212,    -1,   212,    -1,   213,    -1,
     145,   213,    -1,   213,    91,   210,    -1,   145,   213,    91,
     210,    -1,   210,    -1,   213,   145,   210,    -1,   213,    -1,
     142,   145,   213,    -1,   145,   213,    -1,   214,    -1,   214,
     438,    -1,   214,   438,    -1,   218,     9,   436,    14,   382,
      -1,   100,   436,    14,   382,    -1,   219,   220,    -1,    -1,
     221,    -1,   237,    -1,   241,    -1,   246,    -1,   196,   219,
     197,    -1,    66,   314,   221,   268,   270,    -1,    66,   314,
      27,   219,   269,   271,    69,   195,    -1,    -1,    83,   314,
     222,   262,    -1,    -1,    82,   223,   221,    83,   314,   195,
      -1,    -1,    85,   193,   316,   195,   316,   195,   316,   194,
     224,   260,    -1,    -1,    92,   314,   225,   265,    -1,    96,
     195,    -1,    96,   323,   195,    -1,    98,   195,    -1,    98,
     323,   195,    -1,   101,   195,    -1,   101,   323,   195,    -1,
     146,    96,   195,    -1,   106,   278,   195,    -1,   112,   280,
     195,    -1,    81,   315,   195,    -1,   114,   193,   432,   194,
     195,    -1,   195,    -1,    76,    -1,    -1,    87,   193,   323,
      91,   259,   258,   194,   226,   261,    -1,    89,   193,   264,
     194,   263,    -1,    -1,   102,   229,   103,   193,   375,    74,
     194,   196,   219,   197,   231,   227,   234,    -1,    -1,   102,
     229,   160,   228,   232,    -1,   104,   323,   195,    -1,    97,
     210,   195,    -1,   323,   195,    -1,   317,   195,    -1,   318,
     195,    -1,   319,   195,    -1,   320,   195,    -1,   321,   195,
      -1,   101,   320,   195,    -1,   322,   195,    -1,   345,   195,
      -1,   101,   344,   195,    -1,   210,    27,    -1,    -1,   196,
     230,   219,   197,    -1,   231,   103,   193,   375,    74,   194,
     196,   219,   197,    -1,    -1,    -1,   196,   233,   219,   197,
      -1,   160,   232,    -1,    -1,    32,    -1,    -1,    99,    -1,
      -1,   236,   235,   437,   238,   193,   274,   194,   441,   303,
      -1,    -1,   307,   236,   235,   437,   239,   193,   274,   194,
     441,   303,    -1,    -1,   402,   306,   236,   235,   437,   240,
     193,   274,   194,   441,   303,    -1,    -1,   252,   249,   242,
     253,   254,   196,   281,   197,    -1,    -1,   402,   252,   249,
     243,   253,   254,   196,   281,   197,    -1,    -1,   119,   250,
     244,   255,   196,   281,   197,    -1,    -1,   402,   119,   250,
     245,   255,   196,   281,   197,    -1,    -1,   155,   251,   247,
     254,   196,   281,   197,    -1,    -1,   402,   155,   251,   248,
     254,   196,   281,   197,    -1,   437,    -1,   147,    -1,   437,
      -1,   437,    -1,   118,    -1,   111,   118,    -1,   110,   118,
      -1,   120,   375,    -1,    -1,   121,   256,    -1,    -1,   120,
     256,    -1,    -1,   375,    -1,   256,     9,   375,    -1,   375,
      -1,   257,     9,   375,    -1,   123,   259,    -1,    -1,   409,
      -1,    32,   409,    -1,   124,   193,   421,   194,    -1,   221,
      -1,    27,   219,    86,   195,    -1,   221,    -1,    27,   219,
      88,   195,    -1,   221,    -1,    27,   219,    84,   195,    -1,
     221,    -1,    27,   219,    90,   195,    -1,   210,    14,   382,
      -1,   264,     9,   210,    14,   382,    -1,   196,   266,   197,
      -1,   196,   195,   266,   197,    -1,    27,   266,    93,   195,
      -1,    27,   195,   266,    93,   195,    -1,   266,    94,   323,
     267,   219,    -1,   266,    95,   267,   219,    -1,    -1,    27,
      -1,   195,    -1,   268,    67,   314,   221,    -1,    -1,   269,
      67,   314,    27,   219,    -1,    -1,    68,   221,    -1,    -1,
      68,    27,   219,    -1,    -1,   273,     9,   158,    -1,   273,
     387,    -1,   158,    -1,    -1,   403,   309,   448,    74,    -1,
     403,   309,   448,    32,    74,    -1,   403,   309,   448,    32,
      74,    14,   382,    -1,   403,   309,   448,    74,    14,   382,
      -1,   273,     9,   403,   309,   448,    74,    -1,   273,     9,
     403,   309,   448,    32,    74,    -1,   273,     9,   403,   309,
     448,    32,    74,    14,   382,    -1,   273,     9,   403,   309,
     448,    74,    14,   382,    -1,   275,     9,   158,    -1,   275,
     387,    -1,   158,    -1,    -1,   403,   448,    74,    -1,   403,
     448,    32,    74,    -1,   403,   448,    32,    74,    14,   382,
      -1,   403,   448,    74,    14,   382,    -1,   275,     9,   403,
     448,    74,    -1,   275,     9,   403,   448,    32,    74,    -1,
     275,     9,   403,   448,    32,    74,    14,   382,    -1,   275,
       9,   403,   448,    74,    14,   382,    -1,   277,   387,    -1,
      -1,   323,    -1,    32,   409,    -1,   277,     9,   323,    -1,
     277,     9,    32,   409,    -1,   278,     9,   279,    -1,   279,
      -1,    74,    -1,   198,   409,    -1,   198,   196,   323,   197,
      -1,   280,     9,    74,    -1,   280,     9,    74,    14,   382,
      -1,    74,    -1,    74,    14,   382,    -1,   281,   282,    -1,
      -1,    -1,   305,   283,   311,   195,    -1,    -1,   307,   447,
     284,   311,   195,    -1,   312,   195,    -1,    -1,   306,   236,
     235,   437,   193,   285,   272,   194,   441,   304,    -1,    -1,
     402,   306,   236,   235,   437,   193,   286,   272,   194,   441,
     304,    -1,   149,   291,   195,    -1,   150,   297,   195,    -1,
     152,   299,   195,    -1,     4,   120,   375,   195,    -1,     4,
     121,   375,   195,    -1,   105,   257,   195,    -1,   105,   257,
     196,   287,   197,    -1,   287,   288,    -1,   287,   289,    -1,
      -1,   217,   141,   210,   156,   257,   195,    -1,   290,    91,
     306,   210,   195,    -1,   290,    91,   307,   195,    -1,   217,
     141,   210,    -1,   210,    -1,   292,    -1,   291,     9,   292,
      -1,   293,   372,   295,   296,    -1,   147,    -1,   125,    -1,
     375,    -1,   113,    -1,   153,   196,   294,   197,    -1,   381,
      -1,   294,     9,   381,    -1,    14,   382,    -1,    -1,    52,
     154,    -1,    -1,   298,    -1,   297,     9,   298,    -1,   151,
      -1,   300,    -1,   210,    -1,   116,    -1,   193,   301,   194,
      -1,   193,   301,   194,    46,    -1,   193,   301,   194,    26,
      -1,   193,   301,   194,    43,    -1,   300,    -1,   302,    -1,
     302,    46,    -1,   302,    26,    -1,   302,    43,    -1,   301,
       9,   301,    -1,   301,    30,   301,    -1,   210,    -1,   147,
      -1,   151,    -1,   195,    -1,   196,   219,   197,    -1,   195,
      -1,   196,   219,   197,    -1,   307,    -1,   113,    -1,   307,
      -1,    -1,   308,    -1,   307,   308,    -1,   107,    -1,   108,
      -1,   109,    -1,   112,    -1,   111,    -1,   110,    -1,   174,
      -1,   310,    -1,    -1,   107,    -1,   108,    -1,   109,    -1,
     311,     9,    74,    -1,   311,     9,    74,    14,   382,    -1,
      74,    -1,    74,    14,   382,    -1,   312,     9,   436,    14,
     382,    -1,   100,   436,    14,   382,    -1,   193,   313,   194,
      -1,    64,   377,   380,    -1,    63,   323,    -1,   364,    -1,
     340,    -1,   193,   323,   194,    -1,   315,     9,   323,    -1,
     323,    -1,   315,    -1,    -1,   146,   323,    -1,   146,   323,
     123,   323,    -1,   409,    14,   317,    -1,   124,   193,   421,
     194,    14,   317,    -1,   173,   323,    -1,   409,    14,   320,
      -1,   124,   193,   421,   194,    14,   320,    -1,   324,    -1,
     409,    -1,   313,    -1,   124,   193,   421,   194,    14,   323,
      -1,   409,    14,   323,    -1,   409,    14,    32,   409,    -1,
     409,    14,    32,    64,   377,   380,    -1,   409,    25,   323,
      -1,   409,    24,   323,    -1,   409,    23,   323,    -1,   409,
      22,   323,    -1,   409,    21,   323,    -1,   409,    20,   323,
      -1,   409,    19,   323,    -1,   409,    18,   323,    -1,   409,
      17,   323,    -1,   409,    16,   323,    -1,   409,    15,   323,
      -1,   409,    61,    -1,    61,   409,    -1,   409,    60,    -1,
      60,   409,    -1,   323,    28,   323,    -1,   323,    29,   323,
      -1,   323,    10,   323,    -1,   323,    12,   323,    -1,   323,
      11,   323,    -1,   323,    30,   323,    -1,   323,    32,   323,
      -1,   323,    31,   323,    -1,   323,    45,   323,    -1,   323,
      43,   323,    -1,   323,    44,   323,    -1,   323,    46,   323,
      -1,   323,    47,   323,    -1,   323,    48,   323,    -1,   323,
      42,   323,    -1,   323,    41,   323,    -1,    43,   323,    -1,
      44,   323,    -1,    49,   323,    -1,    51,   323,    -1,   323,
      34,   323,    -1,   323,    33,   323,    -1,   323,    36,   323,
      -1,   323,    35,   323,    -1,   323,    37,   323,    -1,   323,
      40,   323,    -1,   323,    38,   323,    -1,   323,    39,   323,
      -1,   323,    50,   377,    -1,   193,   324,   194,    -1,   323,
      26,   323,    27,   323,    -1,   323,    26,    27,   323,    -1,
     431,    -1,    59,   323,    -1,    58,   323,    -1,    57,   323,
      -1,    56,   323,    -1,    55,   323,    -1,    54,   323,    -1,
      53,   323,    -1,    65,   378,    -1,    52,   323,    -1,   384,
      -1,   339,    -1,   338,    -1,   199,   379,   199,    -1,    13,
     323,    -1,   326,    -1,   329,    -1,   342,    -1,   105,   193,
     363,   387,   194,    -1,    -1,    -1,   236,   235,   193,   327,
     274,   194,   441,   325,   196,   219,   197,    -1,    -1,   307,
     236,   235,   193,   328,   274,   194,   441,   325,   196,   219,
     197,    -1,    -1,    74,   330,   332,    -1,    -1,   190,   331,
     274,   191,   441,   332,    -1,     8,   323,    -1,     8,   196,
     219,   197,    -1,    80,    -1,   334,     9,   333,   123,   323,
      -1,   333,   123,   323,    -1,   335,     9,   333,   123,   382,
      -1,   333,   123,   382,    -1,   334,   386,    -1,    -1,   335,
     386,    -1,    -1,   167,   193,   336,   194,    -1,   125,   193,
     422,   194,    -1,    62,   422,   200,    -1,   375,   196,   424,
     197,    -1,   375,   196,   426,   197,    -1,   342,    62,   417,
     200,    -1,   343,    62,   417,   200,    -1,   339,    -1,   433,
      -1,   193,   324,   194,    -1,   346,   347,    -1,   409,    14,
     344,    -1,   176,    74,   179,   323,    -1,   348,   359,    -1,
     348,   359,   362,    -1,   359,    -1,   359,   362,    -1,   349,
      -1,   348,   349,    -1,   350,    -1,   351,    -1,   352,    -1,
     353,    -1,   354,    -1,   355,    -1,   176,    74,   179,   323,
      -1,   183,    74,    14,   323,    -1,   177,   323,    -1,   178,
      74,   179,   323,   180,   323,   181,   323,    -1,   178,    74,
     179,   323,   180,   323,   181,   323,   182,    74,    -1,   184,
     356,    -1,   357,    -1,   356,     9,   357,    -1,   323,    -1,
     323,   358,    -1,   185,    -1,   186,    -1,   360,    -1,   361,
      -1,   187,   323,    -1,   188,   323,   189,   323,    -1,   182,
      74,   347,    -1,   363,     9,    74,    -1,   363,     9,    32,
      74,    -1,    74,    -1,    32,    74,    -1,   161,   147,   365,
     162,    -1,   367,    47,    -1,   367,   162,   368,   161,    47,
     366,    -1,    -1,   147,    -1,   367,   369,    14,   370,    -1,
      -1,   368,   371,    -1,    -1,   147,    -1,   148,    -1,   196,
     323,   197,    -1,   148,    -1,   196,   323,   197,    -1,   364,
      -1,   373,    -1,   372,    27,   373,    -1,   372,    44,   373,
      -1,   210,    -1,    65,    -1,    99,    -1,   100,    -1,   101,
      -1,   146,    -1,   173,    -1,   102,    -1,   103,    -1,   160,
      -1,   104,    -1,    66,    -1,    67,    -1,    69,    -1,    68,
      -1,    83,    -1,    84,    -1,    82,    -1,    85,    -1,    86,
      -1,    87,    -1,    88,    -1,    89,    -1,    90,    -1,    50,
      -1,    91,    -1,    92,    -1,    93,    -1,    94,    -1,    95,
      -1,    96,    -1,    98,    -1,    97,    -1,    81,    -1,    13,
      -1,   118,    -1,   119,    -1,   120,    -1,   121,    -1,    64,
      -1,    63,    -1,   113,    -1,     5,    -1,     7,    -1,     6,
      -1,     4,    -1,     3,    -1,   142,    -1,   105,    -1,   106,
      -1,   115,    -1,   116,    -1,   117,    -1,   112,    -1,   111,
      -1,   110,    -1,   109,    -1,   108,    -1,   107,    -1,   174,
      -1,   114,    -1,   124,    -1,   125,    -1,    10,    -1,    12,
      -1,    11,    -1,   126,    -1,   128,    -1,   127,    -1,   129,
      -1,   130,    -1,   144,    -1,   143,    -1,   172,    -1,   155,
      -1,   157,    -1,   156,    -1,   168,    -1,   170,    -1,   167,
      -1,   216,   193,   276,   194,    -1,   217,    -1,   147,    -1,
     375,    -1,   112,    -1,   415,    -1,   375,    -1,   112,    -1,
     419,    -1,   193,   194,    -1,   314,    -1,    -1,    -1,    79,
      -1,   428,    -1,   193,   276,   194,    -1,    -1,    70,    -1,
      71,    -1,    80,    -1,   129,    -1,   130,    -1,   144,    -1,
     126,    -1,   157,    -1,   127,    -1,   128,    -1,   143,    -1,
     172,    -1,   137,    79,   138,    -1,   137,   138,    -1,   381,
      -1,   215,    -1,    43,   382,    -1,    44,   382,    -1,   125,
     193,   385,   194,    -1,   175,   193,   385,   194,    -1,    62,
     385,   200,    -1,   167,   193,   337,   194,    -1,   383,    -1,
     341,    -1,   217,   141,   210,    -1,   147,   141,   210,    -1,
     215,    -1,    73,    -1,   433,    -1,   381,    -1,   201,   428,
     201,    -1,   202,   428,   202,    -1,   137,   428,   138,    -1,
     388,   386,    -1,    -1,     9,    -1,    -1,     9,    -1,    -1,
     388,     9,   382,   123,   382,    -1,   388,     9,   382,    -1,
     382,   123,   382,    -1,   382,    -1,    70,    -1,    71,    -1,
      80,    -1,   137,    79,   138,    -1,   137,   138,    -1,    70,
      -1,    71,    -1,   210,    -1,   389,    -1,   210,    -1,    43,
     390,    -1,    44,   390,    -1,   125,   193,   392,   194,    -1,
     175,   193,   392,   194,    -1,    62,   392,   200,    -1,   167,
     193,   395,   194,    -1,   393,   386,    -1,    -1,   393,     9,
     391,   123,   391,    -1,   393,     9,   391,    -1,   391,   123,
     391,    -1,   391,    -1,   394,     9,   391,    -1,   391,    -1,
     396,   386,    -1,    -1,   396,     9,   333,   123,   391,    -1,
     333,   123,   391,    -1,   394,   386,    -1,    -1,   193,   397,
     194,    -1,    -1,   399,     9,   210,   398,    -1,   210,   398,
      -1,    -1,   401,   399,   386,    -1,    42,   400,    41,    -1,
     402,    -1,    -1,   405,    -1,   122,   414,    -1,   122,   210,
      -1,   122,   196,   323,   197,    -1,    62,   417,   200,    -1,
     196,   323,   197,    -1,   410,   406,    -1,   193,   313,   194,
     406,    -1,   420,   406,    -1,   193,   313,   194,   406,    -1,
     414,    -1,   374,    -1,   412,    -1,   413,    -1,   407,    -1,
     409,   404,    -1,   193,   313,   194,   404,    -1,   376,   141,
     414,    -1,   411,   193,   276,   194,    -1,   193,   409,   194,
      -1,   374,    -1,   412,    -1,   413,    -1,   407,    -1,   409,
     405,    -1,   193,   313,   194,   405,    -1,   411,   193,   276,
     194,    -1,   193,   409,   194,    -1,   414,    -1,   407,    -1,
     193,   409,   194,    -1,   409,   122,   210,   438,   193,   276,
     194,    -1,   409,   122,   414,   193,   276,   194,    -1,   409,
     122,   196,   323,   197,   193,   276,   194,    -1,   193,   313,
     194,   122,   210,   438,   193,   276,   194,    -1,   193,   313,
     194,   122,   414,   193,   276,   194,    -1,   193,   313,   194,
     122,   196,   323,   197,   193,   276,   194,    -1,   376,   141,
     210,   438,   193,   276,   194,    -1,   376,   141,   414,   193,
     276,   194,    -1,   376,   141,   196,   323,   197,   193,   276,
     194,    -1,   415,    -1,   418,   415,    -1,   415,    62,   417,
     200,    -1,   415,   196,   323,   197,    -1,   416,    -1,    74,
      -1,   198,   196,   323,   197,    -1,   323,    -1,    -1,   198,
      -1,   418,   198,    -1,   414,    -1,   408,    -1,   419,   404,
      -1,   193,   313,   194,   404,    -1,   376,   141,   414,    -1,
     193,   409,   194,    -1,    -1,   408,    -1,   419,   405,    -1,
     193,   313,   194,   405,    -1,   193,   409,   194,    -1,   421,
       9,    -1,   421,     9,   409,    -1,   421,     9,   124,   193,
     421,   194,    -1,    -1,   409,    -1,   124,   193,   421,   194,
      -1,   423,   386,    -1,    -1,   423,     9,   323,   123,   323,
      -1,   423,     9,   323,    -1,   323,   123,   323,    -1,   323,
      -1,   423,     9,   323,   123,    32,   409,    -1,   423,     9,
      32,   409,    -1,   323,   123,    32,   409,    -1,    32,   409,
      -1,   425,   386,    -1,    -1,   425,     9,   323,   123,   323,
      -1,   425,     9,   323,    -1,   323,   123,   323,    -1,   323,
      -1,   427,   386,    -1,    -1,   427,     9,   382,   123,   382,
      -1,   427,     9,   382,    -1,   382,   123,   382,    -1,   382,
      -1,   428,   429,    -1,   428,    79,    -1,   429,    -1,    79,
     429,    -1,    74,    -1,    74,    62,   430,   200,    -1,    74,
     122,   210,    -1,   139,   323,   197,    -1,   139,    73,    62,
     323,   200,   197,    -1,   140,   409,   197,    -1,   210,    -1,
      75,    -1,    74,    -1,   115,   193,   432,   194,    -1,   116,
     193,   409,   194,    -1,   116,   193,   324,   194,    -1,   116,
     193,   313,   194,    -1,     7,   323,    -1,     6,   323,    -1,
       5,   193,   323,   194,    -1,     4,   323,    -1,     3,   323,
      -1,   409,    -1,   432,     9,   409,    -1,   376,   141,   210,
      -1,    -1,    91,   447,    -1,   168,   437,    14,   447,   195,
      -1,   170,   437,   434,    14,   447,   195,    -1,   210,    -1,
     447,   210,    -1,   210,    -1,   210,   163,   442,   164,    -1,
     163,   439,   164,    -1,    -1,   447,    -1,   439,     9,   447,
      -1,   439,     9,   158,    -1,   439,    -1,   158,    -1,    -1,
      -1,    27,   447,    -1,   442,     9,   210,    -1,   210,    -1,
     442,     9,   210,    91,   447,    -1,   210,    91,   447,    -1,
      80,   123,   447,    -1,   444,     9,   443,    -1,   443,    -1,
     444,   386,    -1,    -1,   167,   193,   445,   194,    -1,    26,
     447,    -1,    52,   447,    -1,   217,    -1,   125,    -1,   446,
      -1,   125,   163,   447,   164,    -1,   125,   163,   447,     9,
     447,   164,    -1,   147,    -1,   193,    99,   193,   440,   194,
      27,   447,   194,    -1,   193,   439,     9,   447,   194,    -1,
     447,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   723,   723,   723,   732,   734,   737,   738,   739,   740,
     741,   742,   745,   747,   747,   749,   749,   751,   752,   757,
     758,   759,   760,   761,   762,   763,   764,   765,   766,   767,
     768,   769,   770,   771,   772,   773,   774,   775,   776,   780,
     782,   785,   786,   787,   788,   793,   794,   798,   799,   801,
     804,   810,   817,   824,   828,   834,   836,   839,   840,   841,
     842,   845,   846,   850,   855,   855,   861,   861,   868,   867,
     873,   873,   878,   879,   880,   881,   882,   883,   884,   885,
     886,   887,   888,   889,   890,   893,   891,   898,   906,   900,
     910,   908,   912,   913,   917,   918,   919,   920,   921,   922,
     923,   924,   925,   926,   927,   935,   935,   940,   946,   950,
     950,   958,   959,   963,   964,   968,   973,   972,   985,   983,
     997,   995,  1011,  1010,  1029,  1027,  1046,  1045,  1054,  1052,
    1064,  1063,  1075,  1073,  1086,  1087,  1091,  1094,  1097,  1098,
    1099,  1102,  1104,  1107,  1108,  1111,  1112,  1115,  1116,  1120,
    1121,  1126,  1127,  1130,  1131,  1132,  1136,  1137,  1141,  1142,
    1146,  1147,  1151,  1152,  1157,  1158,  1163,  1164,  1165,  1166,
    1169,  1172,  1174,  1177,  1178,  1182,  1184,  1187,  1190,  1193,
    1194,  1197,  1198,  1202,  1204,  1206,  1207,  1211,  1215,  1219,
    1224,  1229,  1234,  1239,  1245,  1254,  1256,  1258,  1259,  1263,
    1266,  1269,  1273,  1277,  1281,  1285,  1290,  1298,  1300,  1303,
    1304,  1305,  1307,  1312,  1313,  1316,  1317,  1318,  1322,  1323,
    1325,  1326,  1330,  1332,  1335,  1335,  1339,  1338,  1342,  1346,
    1344,  1359,  1356,  1369,  1371,  1373,  1375,  1377,  1379,  1381,
    1385,  1386,  1387,  1390,  1396,  1399,  1405,  1408,  1413,  1415,
    1420,  1425,  1429,  1430,  1436,  1437,  1442,  1443,  1448,  1449,
    1453,  1454,  1458,  1460,  1466,  1471,  1472,  1474,  1478,  1479,
    1480,  1481,  1485,  1486,  1487,  1488,  1489,  1490,  1492,  1497,
    1500,  1501,  1505,  1506,  1510,  1511,  1514,  1515,  1518,  1519,
    1522,  1523,  1527,  1528,  1529,  1530,  1531,  1532,  1533,  1537,
    1538,  1541,  1542,  1543,  1546,  1548,  1550,  1551,  1554,  1556,
    1560,  1561,  1563,  1564,  1565,  1568,  1572,  1573,  1577,  1578,
    1582,  1583,  1587,  1591,  1596,  1600,  1604,  1609,  1610,  1611,
    1614,  1616,  1617,  1618,  1621,  1622,  1623,  1624,  1625,  1626,
    1627,  1628,  1629,  1630,  1631,  1632,  1633,  1634,  1635,  1636,
    1637,  1638,  1639,  1640,  1641,  1642,  1643,  1644,  1645,  1646,
    1647,  1648,  1649,  1650,  1651,  1652,  1653,  1654,  1655,  1656,
    1657,  1658,  1659,  1660,  1661,  1663,  1664,  1666,  1668,  1669,
    1670,  1671,  1672,  1673,  1674,  1675,  1676,  1677,  1678,  1679,
    1680,  1681,  1682,  1683,  1684,  1685,  1686,  1687,  1688,  1692,
    1696,  1701,  1700,  1715,  1713,  1730,  1730,  1745,  1745,  1763,
    1764,  1769,  1774,  1778,  1784,  1788,  1794,  1796,  1800,  1802,
    1806,  1810,  1811,  1815,  1822,  1829,  1831,  1836,  1837,  1838,
    1842,  1846,  1850,  1854,  1856,  1858,  1860,  1865,  1866,  1871,
    1872,  1873,  1874,  1875,  1876,  1880,  1884,  1888,  1892,  1897,
    1902,  1906,  1907,  1911,  1912,  1916,  1917,  1921,  1922,  1926,
    1930,  1934,  1938,  1939,  1940,  1941,  1945,  1951,  1960,  1973,
    1974,  1977,  1980,  1983,  1984,  1987,  1991,  1994,  1997,  2004,
    2005,  2009,  2010,  2012,  2016,  2017,  2018,  2019,  2020,  2021,
    2022,  2023,  2024,  2025,  2026,  2027,  2028,  2029,  2030,  2031,
    2032,  2033,  2034,  2035,  2036,  2037,  2038,  2039,  2040,  2041,
    2042,  2043,  2044,  2045,  2046,  2047,  2048,  2049,  2050,  2051,
    2052,  2053,  2054,  2055,  2056,  2057,  2058,  2059,  2060,  2061,
    2062,  2063,  2064,  2065,  2066,  2067,  2068,  2069,  2070,  2071,
    2072,  2073,  2074,  2075,  2076,  2077,  2078,  2079,  2080,  2081,
    2082,  2083,  2084,  2085,  2086,  2087,  2088,  2089,  2090,  2091,
    2092,  2093,  2094,  2095,  2099,  2104,  2105,  2108,  2109,  2110,
    2114,  2115,  2116,  2120,  2121,  2122,  2126,  2127,  2128,  2131,
    2133,  2137,  2138,  2139,  2141,  2142,  2143,  2144,  2145,  2146,
    2147,  2148,  2149,  2150,  2153,  2158,  2159,  2160,  2161,  2162,
    2164,  2166,  2167,  2169,  2170,  2174,  2177,  2183,  2184,  2185,
    2186,  2187,  2188,  2189,  2194,  2196,  2200,  2201,  2204,  2205,
    2209,  2212,  2214,  2216,  2220,  2221,  2222,  2224,  2227,  2231,
    2232,  2233,  2236,  2237,  2238,  2239,  2240,  2242,  2244,  2245,
    2250,  2252,  2255,  2258,  2260,  2262,  2265,  2267,  2271,  2273,
    2276,  2279,  2285,  2287,  2290,  2291,  2296,  2299,  2303,  2303,
    2308,  2311,  2312,  2316,  2317,  2322,  2323,  2327,  2328,  2332,
    2333,  2338,  2340,  2345,  2346,  2347,  2348,  2349,  2350,  2351,
    2353,  2356,  2358,  2362,  2363,  2364,  2365,  2366,  2368,  2370,
    2372,  2376,  2377,  2378,  2382,  2385,  2388,  2391,  2395,  2399,
    2406,  2410,  2414,  2421,  2422,  2427,  2429,  2430,  2433,  2434,
    2437,  2438,  2442,  2443,  2447,  2448,  2449,  2450,  2452,  2455,
    2458,  2459,  2460,  2462,  2464,  2468,  2469,  2470,  2472,  2473,
    2474,  2478,  2480,  2483,  2485,  2486,  2487,  2488,  2491,  2493,
    2494,  2498,  2500,  2503,  2505,  2506,  2507,  2511,  2513,  2516,
    2519,  2521,  2523,  2527,  2528,  2530,  2531,  2537,  2538,  2540,
    2542,  2544,  2546,  2549,  2550,  2551,  2555,  2556,  2557,  2558,
    2559,  2560,  2561,  2562,  2563,  2567,  2568,  2572,  2580,  2582,
    2586,  2589,  2595,  2596,  2602,  2603,  2610,  2613,  2617,  2620,
    2625,  2626,  2627,  2628,  2632,  2633,  2637,  2639,  2640,  2642,
    2646,  2652,  2654,  2658,  2661,  2664,  2672,  2675,  2678,  2679,
    2682,  2683,  2686,  2690,  2694,  2700,  2708,  2709
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_REQUIRE_ONCE", "T_REQUIRE", "T_EVAL",
  "T_INCLUDE_ONCE", "T_INCLUDE", "T_LAMBDA_ARROW", "','", "T_LOGICAL_OR",
  "T_LOGICAL_XOR", "T_LOGICAL_AND", "T_PRINT", "'='", "T_SR_EQUAL",
  "T_SL_EQUAL", "T_XOR_EQUAL", "T_OR_EQUAL", "T_AND_EQUAL", "T_MOD_EQUAL",
  "T_CONCAT_EQUAL", "T_DIV_EQUAL", "T_MUL_EQUAL", "T_MINUS_EQUAL",
  "T_PLUS_EQUAL", "'?'", "':'", "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'",
  "'^'", "'&'", "T_IS_NOT_IDENTICAL", "T_IS_IDENTICAL", "T_IS_NOT_EQUAL",
  "T_IS_EQUAL", "'<'", "'>'", "T_IS_GREATER_OR_EQUAL",
  "T_IS_SMALLER_OR_EQUAL", "T_SR", "T_SL", "'+'", "'-'", "'.'", "'*'",
  "'/'", "'%'", "'!'", "T_INSTANCEOF", "'~'", "'@'", "T_UNSET_CAST",
  "T_BOOL_CAST", "T_OBJECT_CAST", "T_ARRAY_CAST", "T_STRING_CAST",
  "T_DOUBLE_CAST", "T_INT_CAST", "T_DEC", "T_INC", "'['", "T_CLONE",
  "T_NEW", "T_EXIT", "T_IF", "T_ELSEIF", "T_ELSE", "T_ENDIF", "T_LNUMBER",
  "T_DNUMBER", "T_STRING", "T_STRING_VARNAME", "T_VARIABLE",
  "T_NUM_STRING", "T_INLINE_HTML", "T_CHARACTER", "T_BAD_CHARACTER",
  "T_ENCAPSED_AND_WHITESPACE", "T_CONSTANT_ENCAPSED_STRING", "T_ECHO",
  "T_DO", "T_WHILE", "T_ENDWHILE", "T_FOR", "T_ENDFOR", "T_FOREACH",
  "T_ENDFOREACH", "T_DECLARE", "T_ENDDECLARE", "T_AS", "T_SWITCH",
  "T_ENDSWITCH", "T_CASE", "T_DEFAULT", "T_BREAK", "T_GOTO", "T_CONTINUE",
  "T_FUNCTION", "T_CONST", "T_RETURN", "T_TRY", "T_CATCH", "T_THROW",
  "T_USE", "T_GLOBAL", "T_PUBLIC", "T_PROTECTED", "T_PRIVATE", "T_FINAL",
  "T_ABSTRACT", "T_STATIC", "T_VAR", "T_UNSET", "T_ISSET", "T_EMPTY",
  "T_HALT_COMPILER", "T_CLASS", "T_INTERFACE", "T_EXTENDS", "T_IMPLEMENTS",
  "T_OBJECT_OPERATOR", "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_CLASS_C",
  "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT",
  "T_DOC_COMMENT", "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO", "T_CLOSE_TAG",
  "T_WHITESPACE", "T_START_HEREDOC", "T_END_HEREDOC",
  "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN", "T_DOUBLE_COLON",
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_YIELD",
  "T_XHP_LABEL", "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_XHP_ENUM", "T_XHP_REQUIRED",
  "T_TRAIT", "T_INSTEADOF", "T_TRAIT_C", "T_VARARG", "T_HH_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "T_AWAIT", "T_ASYNC", "T_TUPLE", "T_FROM",
  "T_WHERE", "T_JOIN", "T_IN", "T_ON", "T_EQUALS", "T_INTO", "T_LET",
  "T_ORDERBY", "T_ASCENDING", "T_DESCENDING", "T_SELECT", "T_GROUP",
  "T_BY", "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP", "'('", "')'",
  "';'", "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept",
  "start", "$@1", "top_statement_list", "top_statement", "$@2", "$@3",
  "ident", "use_declarations", "use_declaration", "namespace_name",
  "namespace_string_base", "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "$@9", "$@10", "try_statement_list", "$@11",
  "additional_catches", "finally_statement_list", "$@12",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@13", "$@14", "$@15",
  "class_declaration_statement", "$@16", "$@17", "$@18", "$@19",
  "trait_declaration_statement", "$@20", "$@21", "class_decl_name",
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
  "class_statement_list", "class_statement", "$@22", "$@23", "$@24",
  "$@25", "trait_rules", "trait_precedence_rule", "trait_alias_rule",
  "trait_alias_rule_method", "xhp_attribute_stmt", "xhp_attribute_decl",
  "xhp_attribute_decl_type", "xhp_attribute_enum", "xhp_attribute_default",
  "xhp_attribute_is_required", "xhp_category_stmt", "xhp_category_decl",
  "xhp_children_stmt", "xhp_children_paren_expr", "xhp_children_decl_expr",
  "xhp_children_decl_tag", "function_body", "method_body",
  "variable_modifiers", "method_modifiers", "non_empty_member_modifiers",
  "member_modifier", "parameter_modifiers", "parameter_modifier",
  "class_variable_declaration", "class_constant_declaration",
  "expr_with_parens", "parenthesis_expr", "expr_list", "for_expr",
  "yield_expr", "yield_assign_expr", "yield_list_assign_expr",
  "await_expr", "await_assign_expr", "await_list_assign_expr", "expr",
  "expr_no_variable", "lambda_use_vars", "closure_expression", "$@26",
  "$@27", "lambda_expression", "$@28", "$@29", "lambda_body",
  "shape_keyname", "non_empty_shape_pair_list",
  "non_empty_static_shape_pair_list", "shape_pair_list",
  "static_shape_pair_list", "shape_literal", "array_literal",
  "collection_literal", "static_collection_literal", "dim_expr",
  "dim_expr_base", "query_expr", "query_assign_expr", "query_head",
  "query_body", "query_body_clauses", "query_body_clause", "from_clause",
  "let_clause", "where_clause", "join_clause", "join_into_clause",
  "orderby_clause", "orderings", "ordering", "ordering_direction",
  "select_or_group_clause", "select_clause", "group_clause",
  "query_continuation", "lexical_var_list", "xhp_tag", "xhp_tag_body",
  "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
  "static_class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "common_scalar", "static_scalar",
  "static_class_constant", "scalar", "static_array_pair_list",
  "possible_comma", "hh_possible_comma",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_scalar_ae",
  "static_array_pair_list_ae", "non_empty_static_array_pair_list_ae",
  "non_empty_static_scalar_list_ae", "static_shape_pair_list_ae",
  "non_empty_static_shape_pair_list_ae", "static_scalar_list_ae",
  "attribute_static_scalar_list", "non_empty_user_attribute_list",
  "user_attribute_list", "$@30", "non_empty_user_attributes",
  "optional_user_attributes", "property_access",
  "property_access_without_variables", "array_access",
  "dimmable_variable_access", "dimmable_variable_no_calls_access",
  "variable", "dimmable_variable", "callable_variable",
  "object_method_call", "class_method_call", "variable_without_objects",
  "reference_variable", "compound_variable", "dim_offset",
  "simple_indirect_reference", "variable_no_calls",
  "dimmable_variable_no_calls", "assignment_list", "array_pair_list",
  "non_empty_array_pair_list", "collection_init",
  "non_empty_collection_init", "static_collection_init",
  "non_empty_static_collection_init", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions", "variable_list",
  "class_constant", "hh_opt_constraint", "hh_type_alias_statement",
  "hh_name_with_type", "hh_name_with_typevar", "hh_typeargs_opt",
  "hh_type_list", "hh_func_type_list", "hh_opt_return_type",
  "hh_typevar_list", "hh_shape_member_type",
  "hh_non_empty_shape_member_list", "hh_shape_member_list",
  "hh_shape_type", "hh_type", "hh_type_opt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,    44,
     264,   265,   266,   267,    61,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,    63,    58,   279,   280,
     124,    94,    38,   281,   282,   283,   284,    60,    62,   285,
     286,   287,   288,    43,    45,    46,    42,    47,    37,    33,
     289,   126,    64,   290,   291,   292,   293,   294,   295,   296,
     297,   298,    91,   299,   300,   301,   302,   303,   304,   305,
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
     426,   427,   428,    40,    41,    59,   123,   125,    36,    96,
      93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   203,   205,   204,   206,   206,   207,   207,   207,   207,
     207,   207,   207,   208,   207,   209,   207,   207,   207,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   211,
     211,   212,   212,   212,   212,   213,   213,   214,   214,   214,
     215,   216,   217,   218,   218,   219,   219,   220,   220,   220,
     220,   221,   221,   221,   222,   221,   223,   221,   224,   221,
     225,   221,   221,   221,   221,   221,   221,   221,   221,   221,
     221,   221,   221,   221,   221,   226,   221,   221,   227,   221,
     228,   221,   221,   221,   221,   221,   221,   221,   221,   221,
     221,   221,   221,   221,   221,   230,   229,   231,   231,   233,
     232,   234,   234,   235,   235,   236,   238,   237,   239,   237,
     240,   237,   242,   241,   243,   241,   244,   241,   245,   241,
     247,   246,   248,   246,   249,   249,   250,   251,   252,   252,
     252,   253,   253,   254,   254,   255,   255,   256,   256,   257,
     257,   258,   258,   259,   259,   259,   260,   260,   261,   261,
     262,   262,   263,   263,   264,   264,   265,   265,   265,   265,
     266,   266,   266,   267,   267,   268,   268,   269,   269,   270,
     270,   271,   271,   272,   272,   272,   272,   273,   273,   273,
     273,   273,   273,   273,   273,   274,   274,   274,   274,   275,
     275,   275,   275,   275,   275,   275,   275,   276,   276,   277,
     277,   277,   277,   278,   278,   279,   279,   279,   280,   280,
     280,   280,   281,   281,   283,   282,   284,   282,   282,   285,
     282,   286,   282,   282,   282,   282,   282,   282,   282,   282,
     287,   287,   287,   288,   289,   289,   290,   290,   291,   291,
     292,   292,   293,   293,   293,   293,   294,   294,   295,   295,
     296,   296,   297,   297,   298,   299,   299,   299,   300,   300,
     300,   300,   301,   301,   301,   301,   301,   301,   301,   302,
     302,   302,   303,   303,   304,   304,   305,   305,   306,   306,
     307,   307,   308,   308,   308,   308,   308,   308,   308,   309,
     309,   310,   310,   310,   311,   311,   311,   311,   312,   312,
     313,   313,   313,   313,   313,   314,   315,   315,   316,   316,
     317,   317,   318,   319,   320,   321,   322,   323,   323,   323,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   325,
     325,   327,   326,   328,   326,   330,   329,   331,   329,   332,
     332,   333,   334,   334,   335,   335,   336,   336,   337,   337,
     338,   339,   339,   340,   341,   342,   342,   343,   343,   343,
     344,   345,   346,   347,   347,   347,   347,   348,   348,   349,
     349,   349,   349,   349,   349,   350,   351,   352,   353,   354,
     355,   356,   356,   357,   357,   358,   358,   359,   359,   360,
     361,   362,   363,   363,   363,   363,   364,   365,   365,   366,
     366,   367,   367,   368,   368,   369,   370,   370,   371,   371,
     371,   372,   372,   372,   373,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   374,   375,   375,   376,   376,   376,
     377,   377,   377,   378,   378,   378,   379,   379,   379,   380,
     380,   381,   381,   381,   381,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   383,   383,   384,   384,   384,
     384,   384,   384,   384,   385,   385,   386,   386,   387,   387,
     388,   388,   388,   388,   389,   389,   389,   389,   389,   390,
     390,   390,   391,   391,   391,   391,   391,   391,   391,   391,
     392,   392,   393,   393,   393,   393,   394,   394,   395,   395,
     396,   396,   397,   397,   398,   398,   399,   399,   401,   400,
     402,   403,   403,   404,   404,   405,   405,   406,   406,   407,
     407,   408,   408,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   410,   410,   410,   410,   410,   410,   410,
     410,   411,   411,   411,   412,   412,   412,   412,   412,   412,
     413,   413,   413,   414,   414,   415,   415,   415,   416,   416,
     417,   417,   418,   418,   419,   419,   419,   419,   419,   419,
     420,   420,   420,   420,   420,   421,   421,   421,   421,   421,
     421,   422,   422,   423,   423,   423,   423,   423,   423,   423,
     423,   424,   424,   425,   425,   425,   425,   426,   426,   427,
     427,   427,   427,   428,   428,   428,   428,   429,   429,   429,
     429,   429,   429,   430,   430,   430,   431,   431,   431,   431,
     431,   431,   431,   431,   431,   432,   432,   433,   434,   434,
     435,   435,   436,   436,   437,   437,   438,   438,   439,   439,
     440,   440,   440,   440,   441,   441,   442,   442,   442,   442,
     443,   444,   444,   445,   445,   446,   447,   447,   447,   447,
     447,   447,   447,   447,   447,   447,   448,   448
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     2,     3,     4,     1,     3,     1,     3,     2,
       1,     2,     2,     5,     4,     2,     0,     1,     1,     1,
       1,     3,     5,     8,     0,     4,     0,     6,     0,    10,
       0,     4,     2,     3,     2,     3,     2,     3,     3,     3,
       3,     3,     5,     1,     1,     0,     9,     5,     0,    13,
       0,     5,     3,     3,     2,     2,     2,     2,     2,     2,
       3,     2,     2,     3,     2,     0,     4,     9,     0,     0,
       4,     2,     0,     1,     0,     1,     0,     9,     0,    10,
       0,    11,     0,     8,     0,     9,     0,     7,     0,     8,
       0,     7,     0,     8,     1,     1,     1,     1,     1,     2,
       2,     2,     0,     2,     0,     2,     0,     1,     3,     1,
       3,     2,     0,     1,     2,     4,     1,     4,     1,     4,
       1,     4,     1,     4,     3,     5,     3,     4,     4,     5,
       5,     4,     0,     1,     1,     4,     0,     5,     0,     2,
       0,     3,     0,     3,     2,     1,     0,     4,     5,     7,
       6,     6,     7,     9,     8,     3,     2,     1,     0,     3,
       4,     6,     5,     5,     6,     8,     7,     2,     0,     1,
       2,     3,     4,     3,     1,     1,     2,     4,     3,     5,
       1,     3,     2,     0,     0,     4,     0,     5,     2,     0,
      10,     0,    11,     3,     3,     3,     4,     4,     3,     5,
       2,     2,     0,     6,     5,     4,     3,     1,     1,     3,
       4,     1,     1,     1,     1,     4,     1,     3,     2,     0,
       2,     0,     1,     3,     1,     1,     1,     1,     3,     4,
       4,     4,     1,     1,     2,     2,     2,     3,     3,     1,
       1,     1,     1,     3,     1,     3,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     3,     5,     1,     3,     5,     4,
       3,     3,     2,     1,     1,     3,     3,     1,     1,     0,
       2,     4,     3,     6,     2,     3,     6,     1,     1,     1,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     3,     2,     1,     1,     1,     5,
       0,     0,    11,     0,    12,     0,     3,     0,     6,     2,
       4,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     4,     4,     4,     4,     1,     1,     3,
       2,     3,     4,     2,     3,     1,     2,     1,     2,     1,
       1,     1,     1,     1,     1,     4,     4,     2,     8,    10,
       2,     1,     3,     1,     2,     1,     1,     1,     1,     2,
       4,     3,     3,     4,     1,     2,     4,     2,     6,     0,
       1,     4,     0,     2,     0,     1,     1,     3,     1,     3,
       1,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     0,     0,     1,     1,     3,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     2,     1,     1,     2,     2,     4,
       4,     3,     4,     1,     1,     3,     3,     1,     1,     1,
       1,     3,     3,     3,     2,     0,     1,     0,     1,     0,
       5,     3,     3,     1,     1,     1,     1,     3,     2,     1,
       1,     1,     1,     1,     2,     2,     4,     4,     3,     4,
       2,     0,     5,     3,     3,     1,     3,     1,     2,     0,
       5,     3,     2,     0,     3,     0,     4,     2,     0,     3,
       3,     1,     0,     1,     2,     2,     4,     3,     3,     2,
       4,     2,     4,     1,     1,     1,     1,     1,     2,     4,
       3,     4,     3,     1,     1,     1,     1,     2,     4,     4,
       3,     1,     1,     3,     7,     6,     8,     9,     8,    10,
       7,     6,     8,     1,     2,     4,     4,     1,     1,     4,
       1,     0,     1,     2,     1,     1,     2,     4,     3,     3,
       0,     1,     2,     4,     3,     2,     3,     6,     0,     1,
       4,     2,     0,     5,     3,     3,     1,     6,     4,     4,
       2,     2,     0,     5,     3,     3,     1,     2,     0,     5,
       3,     3,     1,     2,     2,     1,     2,     1,     4,     3,
       3,     6,     3,     1,     1,     1,     4,     4,     4,     4,
       2,     2,     4,     2,     2,     1,     3,     3,     0,     2,
       5,     6,     1,     2,     1,     4,     3,     0,     1,     3,
       3,     1,     1,     0,     0,     2,     3,     1,     5,     3,
       3,     3,     1,     2,     0,     4,     2,     2,     1,     1,
       1,     4,     6,     1,     8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   658,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   732,     0,   720,   575,
       0,   581,   582,    19,   608,   708,    84,   583,     0,    66,
       0,     0,     0,     0,     0,     0,     0,     0,   115,     0,
       0,     0,     0,     0,     0,   292,   293,   294,   297,   296,
     295,     0,     0,     0,     0,   138,     0,     0,     0,   587,
     589,   590,   584,   585,     0,     0,   591,   586,     0,     0,
     566,    20,    21,    22,    24,    23,     0,   588,     0,     0,
       0,     0,   592,     0,   298,    25,    26,    27,    29,    28,
      30,    31,    32,    33,    34,    35,    36,    37,    38,   407,
       0,    83,    56,   712,   576,     0,     0,     4,    45,    47,
      50,   607,     0,   565,     0,     6,   114,     7,     8,     9,
       0,     0,   290,   329,     0,     0,     0,     0,     0,     0,
       0,   327,   396,   397,   393,   392,   314,   398,     0,     0,
     313,   674,   567,     0,   610,   391,   289,   677,   328,     0,
       0,   675,   676,   673,   703,   707,     0,   381,   609,    10,
     297,   296,   295,     0,     0,    45,   114,     0,   774,   328,
     773,     0,   771,   770,   395,     0,     0,   365,   366,   367,
     368,   390,   388,   387,   386,   385,   384,   383,   382,   708,
     568,     0,   787,   567,     0,   348,   346,     0,   736,     0,
     617,   312,   571,     0,   787,   570,     0,   580,   715,   714,
     572,     0,     0,   574,   389,     0,     0,     0,     0,   317,
       0,    64,   319,     0,     0,    70,    72,     0,     0,    74,
       0,     0,     0,   809,   813,     0,     0,    45,   808,     0,
     810,     0,     0,    76,     0,     0,     0,     0,   105,     0,
       0,     0,     0,    40,    41,   215,     0,     0,   214,   140,
     139,   220,     0,     0,     0,     0,     0,   784,   126,   136,
     728,   732,   757,     0,   594,     0,     0,     0,   755,     0,
      15,     0,    49,     0,   320,   130,   137,   472,   417,     0,
     778,   324,   662,   329,     0,   327,   328,     0,     0,   577,
       0,   578,     0,     0,     0,   104,     0,     0,    52,   208,
       0,    18,   113,     0,   135,   122,   134,   295,   114,   291,
      95,    96,    97,    98,    99,   101,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   720,    94,   711,   711,   102,   742,     0,     0,     0,
       0,     0,   288,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   347,   345,     0,   678,   663,
     711,     0,   669,   208,   711,     0,   713,   704,   728,     0,
     114,     0,     0,   660,   655,   617,     0,     0,     0,     0,
     740,     0,   422,   616,   731,     0,     0,    52,     0,   208,
     311,     0,   716,   663,   671,   573,     0,    56,   176,     0,
     406,     0,    81,     0,     0,   318,     0,     0,     0,     0,
       0,    73,    93,    75,   806,   807,     0,   804,     0,     0,
     788,     0,   783,     0,   100,    77,   103,     0,     0,     0,
       0,     0,     0,     0,   430,     0,   437,   439,   440,   441,
     442,   443,   444,   435,   457,   458,    56,     0,    90,    92,
      42,     0,    17,     0,     0,   216,     0,    79,     0,     0,
      80,   775,     0,     0,   329,   327,   328,     0,     0,   146,
       0,   729,     0,     0,     0,     0,   593,   756,   608,     0,
       0,   754,   613,   753,    48,     5,    12,    13,    78,     0,
     144,     0,     0,   411,     0,   617,     0,     0,     0,     0,
     197,     0,   619,   661,   817,   310,   378,   682,    61,    55,
      57,    58,    59,    60,     0,   394,   611,   612,    46,     0,
       0,     0,   619,   209,     0,   401,   116,   142,     0,   351,
     353,   352,     0,     0,   349,   350,   354,   356,   355,   370,
     369,   372,   371,   373,   375,   376,   374,   364,   363,   358,
     359,   357,   360,   361,   362,   377,   710,     0,     0,   746,
       0,   617,     0,   777,   680,   703,   128,   132,   124,   114,
       0,     0,   322,   325,   331,   431,   344,   343,   342,   341,
     340,   339,   338,   337,   336,   335,   334,     0,   665,   664,
       0,     0,     0,     0,     0,     0,     0,   772,   653,   657,
     616,   659,     0,     0,   787,     0,   735,     0,   734,     0,
     719,   718,     0,     0,   665,   664,   315,   178,   180,    56,
     409,   316,     0,    56,   160,    65,   319,     0,     0,     0,
       0,   172,   172,    71,     0,     0,   802,   617,     0,   793,
       0,     0,     0,   615,     0,     0,   566,     0,    25,    50,
     596,   565,   604,     0,   595,    54,   603,     0,     0,   447,
       0,     0,   453,   450,   451,   459,     0,   438,   433,     0,
     436,     0,     0,     0,     0,    39,    43,     0,   213,   221,
     218,     0,     0,   766,   769,   768,   767,    11,   797,     0,
       0,     0,   728,   725,     0,   421,   765,   764,   763,     0,
     759,     0,   760,   762,     0,     5,   321,     0,     0,   466,
     467,   475,   474,     0,     0,   616,   416,   420,     0,   779,
       0,   794,   662,   196,   816,     0,     0,   679,   663,   670,
     709,     0,   786,   210,   564,   618,   207,     0,   662,     0,
       0,   144,   403,   118,   380,     0,   425,   426,     0,   423,
     616,   741,     0,     0,   208,   146,   144,   142,     0,   720,
     332,     0,     0,   208,   667,   668,   681,   705,   706,     0,
       0,     0,   641,   624,   625,   626,     0,     0,     0,    25,
     633,   632,   647,   617,     0,   655,   739,   738,     0,   717,
     663,   672,   579,     0,   182,     0,     0,    62,     0,     0,
       0,     0,     0,     0,   152,   153,   164,     0,    56,   162,
      87,   172,     0,   172,     0,     0,   811,     0,   616,   803,
     805,   792,   791,     0,   789,   597,   598,   623,     0,   617,
     615,     0,     0,   419,   615,     0,   748,   432,     0,     0,
       0,   455,   456,   454,     0,     0,   434,     0,   106,     0,
     109,    91,    44,   217,     0,   776,    82,     0,     0,   785,
     145,   147,   223,     0,     0,   726,     0,   758,     0,    16,
       0,   143,   223,     0,     0,   413,     0,   780,     0,     0,
       0,   195,   817,     0,   199,     0,   665,   664,   789,     0,
     211,    53,     0,   662,   141,     0,   662,     0,   379,   745,
     744,     0,   208,     0,     0,     0,   144,   120,   580,   666,
     208,     0,     0,   629,   630,   631,   634,   635,   645,     0,
     617,   641,     0,   628,   649,   641,   616,   652,   654,   656,
       0,   733,   666,     0,     0,     0,     0,   179,   410,    67,
       0,   319,   154,   728,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   166,     0,   800,   801,     0,     0,   815,
       0,   601,   616,   614,     0,   606,     0,   617,     0,     0,
     605,   752,     0,   617,   445,     0,   446,   452,   460,   461,
       0,    56,   219,   799,   796,     0,   289,   730,   728,   323,
     326,   330,     0,    14,   289,   478,     0,     0,   480,   473,
     476,     0,   471,     0,   781,   795,   408,     0,   200,     0,
       0,     0,   208,   212,   794,     0,   223,     0,   662,     0,
     208,     0,   701,   223,   223,     0,     0,   333,   208,     0,
     695,     0,   638,   616,   640,     0,   627,     0,     0,   617,
       0,   646,   737,     0,    56,     0,   175,   161,     0,     0,
     151,    85,   165,     0,     0,   168,     0,   173,   174,    56,
     167,   812,   790,     0,   622,   621,   599,     0,   616,   418,
     602,   600,     0,   424,   616,   747,     0,     0,     0,     0,
     148,     0,     0,     0,   287,     0,     0,     0,   127,   222,
     224,     0,   286,     0,   289,     0,   761,   131,   469,     0,
       0,   412,     0,   203,     0,   202,   666,   208,     0,   400,
     794,   289,   794,     0,   743,     0,   700,   289,   289,   223,
     662,     0,   694,   644,   643,   636,     0,   639,   616,   648,
     637,    56,   181,    63,    68,   155,     0,   163,   169,    56,
     171,     0,     0,   415,     0,   751,   750,     0,    56,   110,
     798,     0,     0,     0,     0,   149,   254,   252,   566,    24,
       0,   248,     0,   253,   264,     0,   262,   267,     0,   266,
       0,   265,     0,   114,   226,     0,   228,     0,   727,   470,
     468,   479,   477,   204,     0,   201,   208,     0,   698,     0,
       0,     0,   123,   400,   794,   702,   129,   133,   289,     0,
     696,     0,   651,     0,   177,     0,    56,   158,    86,   170,
     814,   620,     0,     0,     0,     0,     0,     0,     0,     0,
     238,   242,     0,     0,   233,   530,   529,   526,   528,   527,
     547,   549,   548,   518,   508,   524,   523,   485,   495,   496,
     498,   497,   517,   501,   499,   500,   502,   503,   504,   505,
     506,   507,   509,   510,   511,   512,   513,   514,   516,   515,
     486,   487,   488,   491,   492,   494,   532,   533,   542,   541,
     540,   539,   538,   537,   525,   544,   534,   535,   536,   519,
     520,   521,   522,   545,   546,   550,   552,   551,   553,   554,
     531,   556,   555,   489,   558,   560,   559,   493,   563,   561,
     562,   557,   490,   543,   484,   259,   481,     0,   234,   280,
     281,   279,   272,     0,   273,   235,   306,     0,     0,     0,
       0,   114,     0,   206,     0,   697,     0,    56,   282,    56,
     117,     0,     0,   125,   794,   642,     0,    56,   156,    69,
       0,   414,   749,   448,   108,   236,   237,   309,   150,     0,
       0,   256,   249,     0,     0,     0,   261,   263,     0,     0,
     268,   275,   276,   274,     0,     0,   225,     0,     0,     0,
       0,   205,   699,     0,   464,   619,     0,     0,    56,   119,
       0,   650,     0,     0,     0,    88,   239,    45,     0,   240,
     241,     0,     0,   255,   258,   482,   483,     0,   250,   277,
     278,   270,   271,   269,   307,   304,   229,   227,   308,     0,
     465,   618,     0,   402,   283,     0,   121,     0,   159,   449,
       0,   112,     0,   289,   257,   260,     0,   662,   231,     0,
     462,   399,   404,   157,     0,     0,    89,   246,     0,   288,
     305,   185,     0,   619,   300,   662,   463,     0,   111,     0,
       0,   245,   794,   662,   184,   301,   302,   303,   817,   299,
       0,     0,     0,   244,     0,   183,   300,     0,   794,     0,
     243,   284,    56,   230,   817,     0,   187,     0,    56,     0,
       0,   188,     0,   232,     0,   285,     0,   191,     0,   190,
     107,   192,     0,   189,     0,   194,   193
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   117,   735,   515,   175,   262,   263,
     119,   120,   121,   122,   123,   124,   307,   539,   540,   434,
     230,  1235,   440,  1166,  1451,   703,   259,   476,  1415,   881,
    1011,  1466,   323,   176,   541,   769,   927,  1056,   542,   557,
     787,   499,   785,   543,   520,   786,   325,   278,   295,   130,
     771,   738,   721,   890,  1184,   975,   834,  1369,  1238,   655,
     840,   439,   663,   842,  1089,   648,   824,   827,   965,  1472,
    1473,   531,   532,   551,   552,   267,   268,   272,  1016,  1119,
    1202,  1349,  1457,  1475,  1379,  1419,  1420,  1421,  1190,  1191,
    1192,  1380,  1386,  1428,  1195,  1196,  1200,  1342,  1343,  1344,
    1360,  1503,  1120,  1121,   177,   132,  1488,  1489,  1347,  1123,
     133,   223,   435,   436,   134,   135,   136,   137,   138,   139,
     140,   141,  1220,   142,   768,   926,   143,   227,   302,   430,
     524,   525,   997,   526,   998,   144,   145,   146,   682,   147,
     148,   256,   149,   257,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   693,   694,   873,   473,   474,   475,   700,
    1405,   150,   521,  1210,   522,   903,   743,  1032,  1029,  1335,
    1336,   151,   152,   153,   217,   224,   310,   420,   154,   857,
     686,   155,   858,   414,   753,   859,   811,   946,   948,   949,
     950,   813,  1068,  1069,   814,   629,   405,   185,   186,   156,
     534,   388,   389,   759,   157,   218,   179,   159,   160,   161,
     162,   163,   164,   165,   587,   166,   220,   221,   502,   209,
     210,   590,   591,  1002,  1003,   287,   288,   729,   167,   492,
     168,   529,   169,   249,   279,   318,   449,   853,   910,   719,
     666,   667,   668,   250,   251,   755
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1217
static const yytype_int16 yypact[] =
{
   -1217,   129, -1217, -1217,  4090, 11690, 11690,   -59, 11690, 11690,
   11690, -1217, 11690, 11690, 11690, 11690, 11690, 11690, 11690, 11690,
   11690, 11690, 11690, 11690, 13661, 13661,  9090, 11690, 13725,   -57,
     -37, -1217, -1217, -1217, -1217,   150, -1217, -1217, 11690, -1217,
     -37,   -20,   105,   124,   -37,  9290,  8932,  9490, -1217, 13089,
    8690,   112, 11690, 13928,    26, -1217, -1217, -1217,   243,   263,
      54,   193,   205,   207,   223, -1217,  8932,   245,   248, -1217,
   -1217, -1217, -1217, -1217,   332, 13779, -1217, -1217,  8932,  9690,
   -1217, -1217, -1217, -1217, -1217, -1217,  8932, -1217,   198,   292,
    8932,  8932, -1217, 11690, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
   11690, -1217, -1217,   302,   320,   347,   347, -1217,   475,   299,
     396, -1217,   328, -1217,    32, -1217,   504, -1217, -1217, -1217,
   13943,   454, -1217, -1217,   363,   372,   385,   406,   414,   423,
   12582, -1217, -1217, -1217, -1217,   477, -1217,   480,   490,   427,
   -1217,    98,   433,   482, -1217, -1217,   768,     8,   510,   115,
     440,   118,   119,   444,   143, -1217,    33, -1217,   569, -1217,
   -1217, -1217,   506,   458,   507, -1217,   504,   454, 14463,  1331,
   14463, 11690, 14463, 14463,  4076,   612,  8932, -1217, -1217,   613,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217, 13374,   502, -1217,   525,   547,   547, 13661, 14123,   472,
     666, -1217,   506, 13374,   502,   535,   538,   488,   127, -1217,
     560,   115,  9890, -1217, -1217, 11690,  7290,   675,    50, 14463,
    8290, -1217, 11690, 11690,  8932, -1217, -1217, 12623,   497, -1217,
   12664, 13089, 13089,   522, -1217,   500,  2137,   676, -1217,   681,
   -1217,  8932,   622, -1217,   503, 12705,   508,   462, -1217,     2,
   12750,  8932,    64, -1217,    62, -1217, 13474,    65, -1217, -1217,
   -1217,   685,    67, 13661, 13661, 11690,   511,   539, -1217, -1217,
   13524,  9090,   300,   251, -1217, 11890, 13661,   344, -1217,  8932,
   -1217,   309,   299,   512, 14164, -1217, -1217, -1217,   621,   692,
     617, 14463,   133,   515, 14463,   517,    94,  4290, 11690,   275,
     524,   357,   275,   268,   264, -1217,  8932, 13089,   519, 10090,
   13089, -1217, -1217,  2356, -1217, -1217, -1217, -1217,   504, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, 11690, 11690, 11690, 10290,
   11690, 11690, 11690, 11690, 11690, 11690, 11690, 11690, 11690, 11690,
   11690, 11690, 11690, 11690, 11690, 11690, 11690, 11690, 11690, 11690,
   11690, 13725, -1217, 11690, 11690, -1217, 11690,   885,  8932,  8932,
   13943,   614,   266,  8490, 11690, 11690, 11690, 11690, 11690, 11690,
   11690, 11690, 11690, 11690, 11690, -1217, -1217,  2260, -1217,   134,
   11690, 11690, -1217, 10090, 11690, 11690,   302,   135, 13524,   533,
     504, 10490, 12791, -1217,   534,   722, 13374,   542,   -16,   885,
     547, 10690, -1217, 10890, -1217,   545,    -1, -1217,    51, 10090,
   -1217,  3397, -1217,   136, -1217, -1217, 12832, -1217, -1217, 11090,
   -1217, 11690, -1217,   650,  7490,   733,   548, 14356,   730,    44,
      21, -1217, -1217, -1217, -1217, -1217, 13089,   673,   563,   737,
   -1217, 13237, -1217,   578, -1217, -1217, -1217,   684, 11690,   690,
     691, 11690, 11690, 11690, -1217,   462, -1217, -1217, -1217, -1217,
   -1217, -1217, -1217,   577, -1217, -1217, -1217,   574, -1217, -1217,
     237, 13928, -1217,  8932, 11690,   547,    26, -1217, 13237,   695,
   -1217,   547,    57,    79,   581,   582,   495,   576,  8932,   652,
     586,   547,    80,   588, 13172,  8932, -1217, -1217,   718,  2696,
     -54, -1217, -1217, -1217,   299, -1217, -1217, -1217, -1217, 11690,
     662,   623,   233, -1217,   663,   778,   594, 13089, 13089,   775,
   -1217,   599,   782, -1217, 13089,    10,   731,   103, -1217, -1217,
   -1217, -1217, -1217, -1217,  2785, -1217, -1217, -1217, -1217,    48,
   13661,   598,   785, 14463,   781, -1217, -1217,   679,  8532, 14503,
    3878,  4076, 11690, 14422,  4475,  4674,  1652,  2742,  2803,  3090,
    3090,  3090,  3090,  1456,  1456,  1456,  1456,   672,   672,   611,
     611,   611,   613,   613,   613, -1217, 14463,   600,   601, 14219,
     608,   797, 11690,   142,   615,   135, -1217, -1217, -1217,   504,
   13424, 11690, -1217, -1217,  4076, -1217,  4076,  4076,  4076,  4076,
    4076,  4076,  4076,  4076,  4076,  4076,  4076, 11690,   142,   616,
     607,  3072,   618,   619,  3370,    82,   620, -1217, 13326, -1217,
    8932, -1217,   515,    10,   502, 13661, 14463, 13661, 14260,    99,
     137, -1217,   624, 11690, -1217, -1217, -1217,  7090,   395, -1217,
   14463, 14463,   -37, -1217, -1217, -1217, 11690, 13108, 13237,  8932,
    7690,   626,   628, -1217,    86,   688, -1217,   805,   640,  3023,
   13089, 13237, 13237, 13237,   643,    41,   674,   645,   646,   327,
   -1217,   699, -1217,   648, -1217, -1217, -1217, 11690,   637, 14463,
     638,   828, 12918,   836, -1217, 14463, 12873, -1217,   577,   772,
   -1217,  4490, 13864,   651,  8932, -1217, -1217,  3492, -1217, -1217,
     835, 13661,   656, -1217, -1217, -1217, -1217, -1217,   763,    90,
   13864,   660, 13524, 13607,   849, -1217, -1217, -1217, -1217,   664,
   -1217, 11690, -1217, -1217,  3690, -1217, 14463, 13864,   669, -1217,
   -1217, -1217, -1217,   852, 11690,   621, -1217, -1217,   678, -1217,
   13089,   840,    29, -1217, -1217,   272, 12903, -1217,   140, -1217,
   -1217, 13089, -1217,   547, -1217, 11290, -1217, 13237,    25,   689,
   13864,   662, -1217, -1217,  4276, 11690, -1217, -1217, 11690, -1217,
   11690, -1217, 12074,   696, 10090,   652,   662,   679,  8932, 13725,
     547, 12115,   697, 10090, -1217, -1217,   148, -1217, -1217,   854,
   13708, 13708, 13326, -1217, -1217, -1217,   698,   250,   700,   705,
   -1217, -1217, -1217,   862,   680,   534,   547,   547, 11490, -1217,
     152, -1217, -1217, 12156,   408,   -37,  8290, -1217,  4690,   686,
    4890,   693, 13661,   706,   760,   547, -1217,   871, -1217, -1217,
   -1217, -1217,    55, -1217,     3, 13089, -1217, 13089,   673, -1217,
   -1217, -1217,   883,   709,   711, -1217, -1217,   786,   708,   903,
   13237,   776,  8932,   621, 13237,  8932, 13237, 14463, 11690, 11690,
   11690, -1217, -1217, -1217, 11690, 11690, -1217,   462, -1217,   841,
   -1217, -1217, -1217, -1217, 13237,   547, -1217, 13089,  8932, -1217,
     910, -1217, -1217,    83,   728,   547,  8890, -1217,  2028, -1217,
    3890,   910, -1217,   271,   -10, 14463,   799, -1217,   729, 13089,
     675, -1217, 13089,   851,   914, 11690,   142,   736, -1217, 13661,
   14463, -1217,   732,    25, -1217,   735,    25,   740,  4276, 14463,
   14315,   741, 10090,   744,   739,   743,   662, -1217,   488,   750,
   10090,   755, 11690, -1217, -1217, -1217, -1217, -1217,   821,   753,
     941, 13326,   816, -1217,   621, 13326, 13326, -1217, -1217, -1217,
   13661, 14463, -1217,   -37,   928,   891,  8290, -1217, -1217, -1217,
     766, 11690,   547, 13524, 13108,   769, 13237,  5090,   513,   770,
   11690,    23,   227, -1217,   800, -1217, -1217,  3489,   940, -1217,
   13237, -1217, 13237, -1217,   780, -1217,   846,   961,   784,   787,
   -1217,   853,   783,   963, 14463, 13001, 14463, -1217, 14463, -1217,
     789, -1217, -1217, -1217,   884, 13864,    59, -1217, 13524, -1217,
   -1217,  4076,   788, -1217,   438, -1217,    56, 11690, -1217, -1217,
   -1217, 11690, -1217, 11690, -1217, -1217, -1217,   286,   965, 13237,
   12197,   791, 10090,   547,   840,   792, -1217,   794,    25, 11690,
   10090,   795, -1217, -1217, -1217,   798,   802, -1217, 10090,   804,
   -1217, 13326, -1217, 13326, -1217,   808, -1217,   868,   814,   987,
     815, -1217,   547,   972, -1217,   817, -1217, -1217,   826,    84,
   -1217, -1217, -1217,   827,   829, -1217, 12329, -1217, -1217, -1217,
   -1217, -1217, -1217, 13089, -1217,   887, -1217, 13237,   621, -1217,
   -1217, -1217, 13237, -1217, 13237, -1217, 11690,   825,  5290, 13089,
   -1217,   465, 13089, 13864, -1217, 13883,   872,  3145, -1217, -1217,
   -1217,   614, 13024,    68,   266,    85, -1217, -1217,   879, 12244,
   12285, 14463,   955,  1016, 13237, -1217,   847, 10090,   839,   936,
     840,   720,   840,   848, 14463,   855, -1217,   906,  1369, -1217,
      25,   856, -1217, -1217,   920, -1217, 13326, -1217,   621, -1217,
   -1217, -1217,  7090, -1217, -1217, -1217,  7890, -1217, -1217, -1217,
    7090,   857, 13237, -1217,   923, -1217,   924, 12390, -1217, -1217,
   -1217, 13864, 13864,  1034,    40, -1217, -1217, -1217,    69,   858,
      70, -1217, 12402, -1217, -1217,    71, -1217, -1217, 13799, -1217,
     866, -1217,   979,   504, -1217, 13089, -1217,   614, -1217, -1217,
   -1217, -1217, -1217,  1061, 13237, -1217, 10090,   882, -1217,   889,
     881,   387, -1217,   936,   840, -1217, -1217, -1217,  1481,   890,
   -1217, 13326, -1217,   962,  7090,  8090, -1217, -1217, -1217,  7090,
   -1217, -1217, 13237, 13237, 11690,  5490,   892,   893, 13237, 13864,
   -1217, -1217,  1287, 13883, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217, -1217, -1217, -1217, -1217,   451, -1217,   872, -1217, -1217,
   -1217, -1217, -1217,    74,   514, -1217,  1064,    72,  8932,   979,
    1072,   504, 13237, -1217,   895, -1217,   365, -1217, -1217, -1217,
   -1217,   896,   387, -1217,   840, -1217, 13326, -1217, -1217, -1217,
    5690, -1217, -1217, 12960, -1217, -1217, -1217, -1217, -1217,  3095,
      35, -1217, -1217, 13237, 12402, 12402,  1041, -1217, 13799, 13799,
     553, -1217, -1217, -1217, 13237,  1020, -1217,   902,    75, 13237,
    8932, -1217, -1217,  1022, -1217,  1088,  5890,  6090, -1217, -1217,
     387, -1217,  6290,   904,  1024,   997, -1217,  1011,   964, -1217,
   -1217,  1015,  1287, -1217, -1217, -1217, -1217,   956, -1217,  1079,
   -1217, -1217, -1217, -1217, -1217,  1097, -1217, -1217, -1217,   919,
   -1217,   377,   921, -1217, -1217,  6490, -1217,   922, -1217, -1217,
     925,   954,  8932,   266, -1217, -1217, 13237,    88, -1217,  1042,
   -1217, -1217, -1217, -1217, 13864,   651, -1217,   966,  8932,   260,
   -1217, -1217,   926,  1114,    15,    88, -1217,  1050, -1217, 13864,
     930, -1217,   840,   132, -1217, -1217, -1217, -1217, 13089, -1217,
     932,   933,    76, -1217,   398, -1217,    15,   417,   840,   934,
   -1217, -1217, -1217, -1217, 13089,  1055,  1117,   398, -1217,  6690,
     425,  1118, 13237, -1217,  6890, -1217,  1059,  1122, 13237, -1217,
   -1217,  1123, 13237, -1217, 13237, -1217, -1217
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1217, -1217, -1217,  -477, -1217, -1217, -1217,    -4, -1217,   657,
     -17,  1003,  1907, -1217,  1509, -1217,  -401, -1217,    11, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,  -326,
   -1217, -1217,  -171,    13,     0, -1217, -1217, -1217,     6, -1217,
   -1217, -1217, -1217,     7, -1217, -1217,   771,   774,   777,   984,
     356,  -644,   360,   410,  -331, -1217,   175, -1217, -1217, -1217,
   -1217, -1217, -1217,  -560,    73, -1217, -1217, -1217, -1217,  -325,
   -1217,  -738, -1217,  -364, -1217, -1217,   665, -1217,  -862, -1217,
   -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,  -100,
   -1217, -1217, -1217, -1217, -1217,  -182, -1217,    37,  -791, -1217,
   -1216,  -351, -1217,  -153,    20,  -130,  -333, -1217,  -189, -1217,
     -50,   -22,  1126,  -613,  -352, -1217, -1217,   -36, -1217, -1217,
    2577,   -14,   -58, -1217, -1217, -1217, -1217, -1217, -1217,   259,
    -711, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
   -1217,   801, -1217, -1217,   293, -1217,   707, -1217, -1217, -1217,
   -1217, -1217, -1217, -1217,   297, -1217,   714, -1217, -1217,   478,
   -1217,   270, -1217, -1217, -1217, -1217, -1217, -1217, -1217, -1217,
    -764, -1217,   912,   368,  -336, -1217, -1217,   246,   948,  2207,
   -1217, -1217,  -725,  -378,  -543, -1217, -1217,   380,  -615,  -810,
   -1217, -1217, -1217, -1217, -1217,   376, -1217, -1217, -1217,  -246,
    -720,  -183,  -181,  -131, -1217, -1217,    27, -1217, -1217, -1217,
   -1217,    -8,  -112, -1217,   113, -1217, -1217, -1217,  -382,   915,
   -1217, -1217, -1217, -1217, -1217,   499,   291, -1217, -1217,   913,
   -1217, -1217, -1217,  -308,   -84,  -191,  -284, -1217, -1009, -1217,
     351, -1217, -1217, -1217,  -177,  -893
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -788
static const yytype_int16 yytable[] =
{
     118,   329,   296,   371,   127,   399,   299,   300,   226,   766,
     128,   129,   554,   812,   254,   125,   625,   126,   231,  1037,
     219,   602,   235,   417,   131,   585,   647,   631,   392,   622,
     922,   158,   912,   549,   906,  1139,   264,   422,   734,   423,
    1024,   320,   238,   831,  1422,   247,   326,   329,   661,  1249,
    1087,   205,   206,   659,   397,   642,   533,   761,   291,   431,
     303,   292,   277,  1111,   444,   445,   711,    11,   387,   450,
    -686,    11,   390,   481,   486,   701,   489,  1205,  -251,  1253,
    1337,  1395,   277,  1388,  1395,  1249,   277,   277,   711,   723,
     424,   723,   723,   723,   723,   845,   305,   980,   981,   888,
     265,    11,   844,  1128,  1389,   477,   387,   199,   401,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     861,   387,  1485,  1486,  1487,   199,   277,   925,   271,     3,
      11,  1221,   756,  1223,   181,   994,   222,   306,  1030,   999,
     450,  1065,   935,   733,   328,  1070,  1409,   746,   979,   980,
     981,   407,   996,   483,   385,   386,   225,   558,  -405,  1112,
    -683,   390,   478,   415,  1113,  -690,    55,    56,    57,   170,
     171,   327,  1114,   232,    11,    11,   372,   390,   537,   284,
    -684,  -685,   404,   530,  1141,  1045,  1031,   911,  1047,  -721,
     400,  1147,  1148,   640,  1446,  -568,  -687,   394,  -722,  -724,
     983,  -692,  -688,   297,  -686,   394,   391,   316,  1115,  1116,
    -689,  1117,   762,   781,  -723,  1362,   387,   662,  1088,  -198,
    -618,   421,   118,  -618,   266,   494,   118,   321,   408,   626,
     438,   396,  1423,    94,   410,  1250,  1251,   428,   660,   556,
     416,   433,   329,  1067,   480,   432,  1471,   452,   828,   113,
     846,   712,   830,   158,   889,   595,  1118,   158,   900,   482,
     487,   495,   490,  1206,  -251,  1254,  1338,  1396,  1390,   664,
    1437,  1500,   514,   713,   724,   595,   799,  1017,  1165,  1208,
     740,   978,  -186,   982,  -569,   296,   326,  1228,   537,   849,
    1495,   530,  1055,   485,  -683,   391,  -693,   595,   233,  -690,
     491,   491,   496,   118,   913,   317,   595,   501,   258,   595,
    1143,   391,   548,   510,  -684,  -685,   247,   234,  1132,   277,
     126,   980,   981,  -721,  -198,   282,  -618,   131,   704,   952,
    -687,   395,  -722,  -724,   158,  -787,  -688,   603,   282,   395,
     893,  1071,   282,   511,  -689,   297,   914,   511,  -723,   282,
     748,   749,   757,   219,   758,  1410,   632,   754,  1078,   594,
    1133,   269,   504,   593,   277,   277,   277,    55,    56,    57,
     170,   171,   327,    55,    56,    57,   170,   171,   327,   619,
     741,   270,   316,   618,   599,   852,   273,  1174,   953,   506,
     285,   286,   204,   204,   282,   742,   216,  1403,   274,   309,
     275,   594,   783,   285,   286,   634,   282,   285,   286,  1459,
     641,   283,  1229,   645,   285,   286,   276,   644,   282,  1025,
     933,   282,   505,   511,  1090,   501,   312,   792,   788,   941,
     118,   282,  1026,   408,    94,   957,   511,   977,   280,  1404,
      94,   281,  1111,   783,   316,   654,  1153,  1233,  1154,  1505,
     757,  1460,   758,   938,   316,  1481,   819,  1516,   820,   285,
     286,   158,   825,   826,   264,  1383,   547,  1027,  -787,   546,
     284,   285,   286,  1494,   773,   963,   964,   588,  1384,   706,
      11,   993,   512,   285,   286,   298,   285,   286,   417,  1507,
     317,  1506,   450,   854,   718,  1385,   285,   286,   308,  1517,
     728,   730,   315,   620,   516,   517,   533,   623,   821,   401,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   319,   533,  -787,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   322,  -787,  1112,  -427,
    1391,  1232,   363,  1113,  1019,    55,    56,    57,   170,   171,
     327,  1114,   364,    48,   277,   385,   386,  1392,   330,   317,
    1393,    55,    56,    57,   170,   171,   327,   331,  1051,   204,
     385,   386,  1064,   908,   507,   204,  1059,   763,   513,  1431,
     332,   204,  1358,  1359,   918,  1181,  1182,  1115,  1116,  -787,
    1117,  1079,  -787,  1501,  1502,  1497,  1432,  1429,  1430,  1433,
     507,   333,   513,   507,   513,   513,  1084,   980,   981,   334,
    1108,  1510,    94,   311,   313,   314,  1365,   387,   335,  1099,
    1425,  1426,   365,   367,   810,  1105,   815,   790,    94,   366,
     829,  -428,   387,   393,   204,  1127,  1125,  -691,   457,   458,
     459,   204,   204,   118,   595,   460,   461,  -568,   204,   462,
     463,   398,   289,   403,   204,   837,   118,   358,   359,   360,
     126,   361,   816,   361,   817,   317,   409,   131,   984,   387,
     985,   839,   412,  1162,   158,   413,  -567,   533,  1138,   418,
     533,   419,   421,   429,   835,   446,  1145,   158,  1170,   716,
    -782,  1159,   442,   447,  1151,   451,   453,   118,   454,   488,
     882,   523,   498,   456,   937,   497,   527,   518,   528,   535,
    1013,   536,   -51,    48,   126,   355,   356,   357,   358,   359,
     360,   131,   361,   545,  1111,  1041,   555,   628,   158,   216,
     118,   630,  1035,   652,   127,   754,   633,  1474,   885,   639,
     128,   129,   431,   656,   658,   125,   670,   126,   917,   501,
     895,  1411,   916,   665,   131,  1474,   669,   687,   688,   699,
    1234,   158,    11,  1496,   690,   691,   204,   702,  1239,   710,
    1124,   717,   720,  1217,   204,   714,   715,  1245,  1124,   722,
     731,   219,   725,   737,   277,   739,   744,   745,   747,   750,
     751,   752,   764,  -429,   765,   767,   945,   945,   810,   770,
     776,   777,   533,   966,  1183,   779,   780,   794,   784,   793,
     918,   847,   796,   772,   848,   862,   868,   869,   822,   797,
    1112,   841,   118,   843,   118,  1113,   118,    55,    56,    57,
     170,   171,   327,  1114,   850,  1370,   860,   967,   863,   864,
     865,   126,   870,   126,   866,   874,   877,   880,   131,   884,
     131,   886,  1354,   158,   887,   158,   892,   158,   995,   972,
    1020,  1000,  1442,   896,   897,   902,   904,   909,   942,  1115,
    1116,   956,  1117,   907,   958,    55,    56,    57,    58,    59,
     327,   969,   923,   974,  1014,   976,    65,   368,   971,   932,
     940,   951,   987,   954,    94,  1124,   118,  1350,   955,   973,
     127,  1124,  1124,   988,   533,   989,   128,   129,   991,   990,
    1111,   125,   992,   126,   506,  1010,  1171,  1222,   204,  1015,
     131,  1018,  1033,   369,  1034,  1038,  1044,   158,  1039,  1042,
    1484,  1046,  1180,  1048,  1050,  1053,   203,   203,  1052,  1054,
     215,  1073,    94,  1058,  1061,  1204,  1043,   810,    11,  1060,
    1063,   810,   810,  1062,  1066,  1074,  1406,    33,  1407,   199,
    1075,  1077,   118,  1081,  1091,  1085,  1412,  1093,   204,  1097,
    1098,  1207,  1104,   118,  1096,  1109,  1102,  1076,  1100,  1134,
    1103,  1101,  1124,  1107,  1137,  1126,  1140,  1072,  1142,  1146,
     126,  1156,   329,   158,  1149,  1150,  1158,   131,  1152,  1161,
     501,   835,  1155,   204,   158,   204,  1112,  1445,  1157,  1160,
    1172,  1113,  1163,    55,    56,    57,   170,   171,   327,  1114,
    1164,  1178,  1167,  1194,  1168,   204,  1209,   202,   202,  1213,
    1214,   214,  1348,  1218,    81,    82,  1122,    83,    84,    85,
    1216,  1219,  1224,  1231,  1122,   501,  1242,  1243,  1248,  1225,
    1230,  1240,   214,  1346,  1252,  1115,  1116,   810,  1117,   810,
      95,  1345,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,  1352,  1355,  1357,  1394,   204,
      94,   592,  1356,   113,  1364,  1366,  1399,  1375,  1376,  1402,
     204,   204,  1408,  1427,  1435,  1436,  1440,  1441,  1449,  1448,
    1450,  1509,  -247,  1226,   118,  1452,  1453,  1514,   247,  1389,
    1455,  1456,  1458,  1199,  1465,  1461,  1476,  1463,  1464,   203,
    1482,   126,  1479,  1483,  1491,  1493,  1498,  1499,   131,  1511,
    1508,  1512,  1518,  1521,  1203,   158,  1522,  1524,   705,  1478,
     370,   598,   596,   936,   372,   934,   597,   901,  1492,  1080,
    1490,   708,   810,  1382,  1201,  1387,  1513,   216,   118,  1169,
    1398,  1122,   118,  1504,   228,  1361,   118,  1122,  1122,  1036,
    1009,  1007,   697,  1028,   605,   126,   876,  1237,   203,   698,
    1400,   947,   131,   126,  1057,   203,   203,   493,  1334,   158,
     131,   959,   203,   158,  1341,     0,   503,   158,   203,   986,
     204,   247,     0,     0,   202,     0,     0,     0,     0,     0,
     202,   533,     0,     0,     0,     0,   202,     0,     0,     0,
    1351,     0,     0,     0,     0,     0,     0,   810,     0,   533,
     118,   118,     0,     0,     0,   118,     0,   533,     0,     0,
       0,   118,     0,     0,   214,   214,  1368,   126,  1122,   214,
       0,     0,   126,     0,   131,     0,     0,     0,   126,   131,
       0,   158,   158,     0,  1397,   131,   158,     0,     0,   202,
       0,     0,   158,   215,     0,     0,   202,   202,     0,     0,
       0,     0,     0,   202,     0,     0,     0,   204,     0,   202,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1468,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     203,   754,     0,     0,     0,     0,  1439,     0,     0,     0,
     214,     0,     0,   214,     0,     0,     0,   754,   204,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   329,
       0,   204,   204,     0,   277,   401,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,    31,    32,     0,
       0,     0,   810,   683,   214,     0,   118,    37,     0,     0,
       0,     0,     0,  1111,     0,  1417,     0,     0,     0,     0,
    1334,  1334,     0,   126,  1341,  1341,   204,     0,     0,     0,
     131,   385,   386,     0,     0,     0,   277,   158,     0,   684,
     683,   202,   118,   118,     0,     0,     0,     0,   118,   202,
       0,    11,     0,    69,    70,    71,    72,    73,     0,   126,
     126,     0,     0,     0,   675,   126,   131,   131,     0,     0,
      76,    77,   131,   158,   158,     0,   684,     0,     0,   158,
       0,   118,     0,     0,    87,     0,     0,     0,  1467,   214,
       0,     0,     0,   387,   679,     0,     0,     0,   126,    92,
       0,     0,   203,     0,  1480,   131,     0,     0,     0,  1112,
       0,     0,   158,  1469,  1113,     0,    55,    56,    57,   170,
     171,   327,  1114,     0,     0,  1111,     0,     0,     0,     0,
       0,   679,     0,  -788,  -788,  -788,  -788,   353,   354,   355,
     356,   357,   358,   359,   360,   118,   361,     0,     0,     0,
     118,     0,   203,     0,     0,     0,     0,     0,  1115,  1116,
       0,  1117,   126,    11,     0,     0,     0,   126,     0,   131,
     214,   214,     0,     0,   131,     0,   158,   214,     0,     0,
       0,   158,     0,    94,     0,     0,     0,   203,     0,   203,
       0,     0,     0,   202,     0,     0,     0,     0,   248,     0,
       0,     0,     0,     0,     0,     0,  1227,     0,     0,   203,
     683,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1112,     0,   683,   683,   683,  1113,     0,    55,    56,
      57,   170,   171,   327,  1114,     0,     0,     0,     0,     0,
       0,     0,     0,   202,     0,     0,   684,     0,     0,     0,
       0,     0,     0,     0,   879,     0,     0,     0,     0,   684,
     684,   684,     0,   203,     0,     0,     0,     0,     0,     0,
    1115,  1116,   891,  1117,   203,   203,     0,     0,   202,     0,
     202,     0,     0,     0,     0,     0,     0,     0,     0,   891,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
     202,   679,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   214,   214,   679,   679,   679,     0,  1363,   683,
       0,     0,   924,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   215,   361,     0,     0,   214,     0,     0,     0,     0,
       0,     0,     0,     0,   202,   684,     0,     0,     0,     0,
       0,     0,     0,   214,     0,   202,   202,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,   203,     0,     0,     0,     0,     0,
     248,   248,     0,   214,     0,   248,     0,     0,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
     679,     0,   683,   214,     0,     0,   683,     0,   683,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,   683,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   684,     0,
       0,     0,   684,     0,   684,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   248,     0,     0,   248,
       0,   203,   684,     0,     0,   202,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   679,     0,     0,     0,   679,     0,   679,
       0,     0,   203,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   203,   203,   679,   683,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   683,     0,   683,     0,     0,     0,     0,     0,
       0,     0,   214,     0,     0,   214,     0,     0,     0,     0,
       0,     0,   202,     0,   684,     0,     0,  1110,     0,     0,
     203,     0,     0,     0,     0,     0,     0,     0,   684,     0,
     684,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   683,     0,     0,     0,   248,     0,     0,     0,     0,
     681,     0,     0,   202,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   202,   202,     0,   679,
       0,     0,     0,     0,     0,     0,     0,   684,     0,     0,
     214,     0,     0,   679,     0,   679,     0,   681,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   683,
       0,     0,     0,     0,   683,     0,   683,     0,   214,     0,
       0,   202,     0,     0,     0,  1185,     0,  1193,     0,     0,
       0,     0,     0,     0,     0,     0,   248,   248,   336,   337,
     338,     0,   679,   248,     0,   684,   683,     0,     0,     0,
     684,     0,   684,     0,   339,     0,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,     0,   361,     0,
       0,     0,   684,     0,   683,     0,     0,     0,     0,     0,
       0,     0,     0,  1246,  1247,     0,   214,     0,     0,     0,
     679,     0,     0,     0,     0,   679,     0,   679,     0,     0,
       0,     0,   214,     0,     0,   214,   214,     0,   214,     0,
     684,     0,     0,     0,     0,   214,   683,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   679,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   683,   683,     0,     0,     0,     0,
     683,  1378,   684,   241,     0,  1193,     0,   681,     0,     0,
       0,     0,     0,     0,     0,   679,     0,     0,   248,   248,
     681,   681,   681,     0,   214,   214,     0,     0,     0,   242,
     684,   684,     0,     0,     0,     0,   684,     0,     0,     0,
    1381,     0,     0,     0,     0,     0,     0,     0,   214,    33,
       0,     0,     0,     0,     0,     0,     0,   679,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1022,     0,
       0,     0,     0,     0,     0,     0,   448,     0,     0,     0,
       0,     0,     0,     0,     0,   679,   679,     0,     0,     0,
       0,   679,   214,     0,     0,     0,   214,     0,     0,   248,
       0,     0,   243,     0,   683,     0,     0,     0,     0,     0,
     248,     0,     0,     0,     0,     0,   681,     0,     0,   174,
       0,     0,    78,     0,   244,     0,    81,    82,     0,    83,
      84,    85,     0,     0,     0,   683,     0,     0,     0,     0,
     684,     0,     0,     0,   245,     0,   683,     0,     0,     0,
       0,   683,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
     246,   684,    33,     0,   199,     0,     0,     0,     0,     0,
       0,     0,   684,     0,     0,     0,     0,   684,     0,     0,
       0,     0,     0,     0,   248,   679,   248,     0,   680,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   683,   681,
    1454,     0,     0,   681,     0,   681,  1477,     0,     0,     0,
       0,     0,   214,     0,     0,     0,   679,     0,     0,     0,
       0,  1185,     0,   681,     0,   680,   248,   679,     0,     0,
       0,     0,   679,     0,   684,     0,     0,     0,     0,    81,
      82,     0,    83,    84,    85,     0,     0,     0,   248,     0,
       0,   248,     0,     0,   683,     0,     0,     0,    33,     0,
     683,     0,     0,     0,   683,    95,   683,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
       0,     0,     0,     0,     0,     0,   617,     0,   113,   679,
     684,     0,     0,     0,     0,     0,   684,   214,     0,     0,
     684,     0,   684,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   214,     0,     0,   681,     0,     0,     0,     0,
       0,   214,     0,     0,     0,     0,   248,     0,     0,   681,
       0,   681,     0,     0,     0,    81,    82,   214,    83,    84,
      85,     0,     0,     0,     0,   679,     0,     0,     0,     0,
       0,   679,     0,     0,     0,   679,     0,   679,     0,     0,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,     0,   681,   555,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   680,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   680,   680,
     680,     0,   178,   180,     0,   182,   183,   184,     0,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,     0,   248,   208,   211,     0,   681,     0,     0,     0,
       0,   681,     0,   681,     0,   229,     0,     0,   248,     0,
       0,   248,   237,     0,   240,     0,     0,   255,     0,   260,
       0,   248,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   681,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   294,     0,   685,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     301,     0,     0,     0,   680,     0,     0,     0,     0,     0,
       0,   681,     0,     0,     0,     0,     0,   304,     0,     0,
       0,     0,     0,     0,     0,   709,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   336,   337,   338,     0,
       0,     0,     0,     0,   248,     0,     0,     0,     0,     0,
       0,     0,   339,   681,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,     0,   361,     0,     0,     0,
       0,   681,   681,     0,     0,     0,     0,   681,   402,     0,
       0,     0,     0,     0,     0,     0,     0,   680,     0,     0,
       0,   680,     0,   680,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   680,   361,     0,     0,   336,   337,   338,     0,   426,
       0,     0,   426,     0,     0,     0,     0,     0,     0,   229,
     437,   339,     0,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   304,   361,     0,     0,     0,     0,   208,     0,
       0,   681,   509,     0,     0,   836,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   855,   856,
       0,     0,     0,   680,     0,   544,     0,     0,  1418,     0,
       0,     0,   681,   732,     0,     0,   553,   680,     0,   680,
       0,     0,     0,   681,     0,     0,     0,     0,   681,     0,
       0,     0,     0,   559,   560,   561,   563,   564,   565,   566,
     567,   568,   569,   570,   571,   572,   573,   574,   575,   576,
     577,   578,   579,   580,   581,   582,   583,   584,     0,     0,
     586,   586,     0,   589,     0,     0,   680,     0,     0,     0,
     604,   606,   607,   608,   609,   610,   611,   612,   613,   614,
     615,   616,     0,     0,     0,   681,     0,   586,   621,     0,
     553,   586,   624,     0,   921,     0,     0,     0,   604,     0,
       0,     0,   760,     0,     0,     0,     0,     0,   636,     0,
     638,     0,     0,     0,     0,     0,   553,   248,     0,     0,
       0,     0,     0,     0,   680,     0,   650,     0,   651,   680,
       0,   680,     0,   248,     0,     0,     0,     0,     0,     0,
       0,   681,     0,     0,     0,     0,     0,   681,     0,     0,
       0,   681,     0,   681,     0,   689,     0,     0,   692,   695,
     696,   680,     0,     0,     0,     0,     0,     0,     0,   241,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   707,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1001,     0,   242,     0,     0,     0,   680,
       0,     0,   336,   337,   338,     0,     0,     0,     0,     0,
       0,  1012,     0,     0,     0,    33,   736,     0,   339,     0,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   680,   361,  -788,  -788,  -788,  -788,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   774,
     361,     0,     0,     0,     0,     0,     0,     0,   243,   680,
     680,     0,     0,     0,     0,   680,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   174,     0,    33,    78,   782,
     244,     0,    81,    82,     0,    83,    84,    85,   294,     0,
       0,   851,     0,  1082,     0,     0,     0,     0,     0,     0,
     245,     0,     0,     0,   791,     0,     0,  1094,    95,  1095,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,     0,   246,    33,     0,     0,
     823,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   229,     0,     0,     0,   174,     0,     0,
      78,     0,     0,     0,    81,    82,  1135,    83,    84,    85,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   680,
       0,  1197,     0,     0,   867,     0,     0,     0,     0,   795,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,     0,     0,     0,
     680,     0,  1416,     0,    81,    82,     0,    83,    84,    85,
       0,   680,     0,     0,  1173,     0,   680,     0,   898,  1175,
       0,  1176,     0,     0,     0,     0,     0,     0,     0,     0,
      95,   905,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,     0,  1198,     0,
       0,  1215,   920,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   928,     0,     0,   929,     0,   930,     0,     0,
       0,   553,     0,   680,     0,     0,     0,     0,     0,     0,
     553,     0,     0,     0,     0,     0,     0,     0,     0,  1241,
     336,   337,   338,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   961,   339,     0,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   680,
     361,  1353,     0,     0,     0,   680,     0,     0,     0,   680,
       0,   680,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1004,  1005,  1006,     0,  1371,
    1372,   692,  1008,     0,     0,  1377,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    33,
       0,   199,     0,  1021,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1040,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   336,   337,   338,     0,     0,     0,     0,   553,
       0,     0,     0,     0,     0,   241,     0,   553,   339,  1021,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   242,   361,     0,     0,     0,    81,    82,   229,    83,
      84,    85,     0,     0,     0,     0,     0,  1086,     0,  1401,
       0,    33,     0,     0,     0,     0,     0,   798,     0,     0,
       0,     0,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
    1424,     0,     0,   643,     0,   113,     0,     0,     0,     0,
       0,  1434,     0,     0,  1129,     0,  1438,     0,  1130,     0,
    1131,     0,     0,     0,   243,     0,     0,     0,     0,   553,
       0,     0,     0,     0,     0,     0,  1144,   553,     0,     0,
       0,   174,     0,     0,    78,   553,   244,     0,    81,    82,
       0,    83,    84,    85,     0,     0,     0,  1092,     0,     0,
       0,     0,     0,     0,     0,     0,   245,     0,     0,     0,
       0,     0,     0,  1470,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,   246,  1177,     0,     0,     0,     0,     0,   883,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   553,     0,     0,     0,     0,  1519,
       0,     0,     0,     0,     0,  1523,     0,     0,     0,  1525,
       0,  1526,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
      49,    50,    51,   553,    52,    53,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,    64,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,  1373,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,    75,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,    90,     0,
      91,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,   899,   113,   114,
     338,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   339,     0,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,     0,   361,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
      49,    50,    51,     0,    52,    53,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,    64,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,    75,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,    90,     0,
      91,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1023,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,   339,    10,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,     0,   361,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
      49,    50,    51,     0,    52,    53,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,    64,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,    75,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,    90,     0,
      91,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,     0,   361,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,   538,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,   878,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,     0,   361,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,   968,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,   970,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
    1083,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1179,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1374,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,  1413,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1443,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1444,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,  1447,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1462,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1515,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1520,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   427,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
     170,   171,    60,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   653,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
     170,   171,    60,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   838,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
     170,   171,    60,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1236,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
     170,   171,    60,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1367,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
     170,   171,    60,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
     170,   171,    60,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   600,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,    33,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,   601,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,   252,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,    81,    82,   110,    83,    84,    85,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,     0,   772,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,   252,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   253,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,    33,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,   601,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,    81,    82,   110,    83,    84,    85,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   207,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   236,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   239,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   293,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,   425,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   550,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   562,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   600,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   635,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   637,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,   649,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   919,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   960,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,   508,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,   336,   337,   338,     0,   113,   114,
       0,   115,   116,     0,     0,     0,     0,     0,     0,     0,
     339,     0,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,     0,   361,   336,   337,   338,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   339,     0,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,   336,   337,   338,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   339,     0,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,     0,   361,   336,   337,   338,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   339,     0,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,     0,   361,     0,     0,
       0,     0,     0,     0,   336,   337,   338,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     339,   931,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,     0,   361,   336,   337,   338,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   339,   939,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,     0,     0,     0,   336,
     337,   338,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   962,     0,   339,  1087,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1136,     0,     0,     0,     0,     0,
     336,   337,   338,     0,     0,  1255,  1256,  1257,  1258,  1259,
       0,     0,  1260,  1261,  1262,  1263,   339,     0,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,     0,
     361,  1211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1264,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1265,  1266,  1267,  1268,  1269,
    1270,  1271,     0,     0,    33,     0,     0,     0,     0,     0,
       0,     0,  1212,  1272,  1273,  1274,  1275,  1276,  1277,  1278,
    1279,  1280,  1281,  1282,  1283,  1284,  1285,  1286,  1287,  1288,
    1289,  1290,  1291,  1292,  1293,  1294,  1295,  1296,  1297,  1298,
    1299,  1300,  1301,  1302,  1303,  1304,  1305,  1306,  1307,  1308,
    1309,  1310,  1311,  1312,  1088,     0,  1313,  1314,  1315,  1316,
    1317,  1318,  1319,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1320,  1321,  1322,     0,  1323,     0,
       0,    81,    82,     0,    83,    84,    85,  1324,  1325,  1326,
       0,     0,  1327,     0,     0,     0,     0,     0,     0,  1328,
    1329,  1244,  1330,     0,  1331,  1332,  1333,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   336,   337,   338,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   339,     0,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,     0,   361,   336,   337,   338,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   339,
       0,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   336,   337,   338,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     339,     0,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,     0,   361,   336,   337,   338,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   339,     0,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,     0,     0,     0,     0,
     336,   337,   338,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   339,   362,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,     0,
     361,   336,   337,   338,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   339,   441,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   336,   337,   338,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   339,   443,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,     0,   361,   336,   337,   338,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   339,
     455,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,     0,     0,     0,     0,   336,   337,
     338,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   339,   479,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,     0,   361,     0,
     336,   337,   338,     0,     0,    33,     0,   199,     0,     0,
       0,     0,     0,     0,     0,   627,   339,     0,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,     0,
     361,   336,   337,   338,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   646,   339,     0,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     241,   361,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,   875,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   242,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,     0,    33,     0,     0,   915,
       0,   113,     0,   871,   872,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   241,     0,     0,     0,     0,
       0,     0,     0,  -288,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   170,   171,   327,     0,     0,     0,
     832,   242,  1414,     0,     0,     0,     0,     0,     0,   243,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    33,     0,     0,     0,     0,   174,     0,     0,    78,
       0,   244,     0,    81,    82,     0,    83,    84,    85,     0,
      33,  1106,   199,     0,     0,     0,     0,     0,     0,     0,
       0,   245,     0,     0,     0,     0,     0,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   243,     0,     0,   246,     0,     0,
     200,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   174,   833,     0,    78,     0,   244,     0,    81,    82,
       0,    83,    84,    85,    33,     0,   726,   727,     0,     0,
     174,     0,     0,    78,     0,    80,   245,    81,    82,     0,
      83,    84,    85,     0,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
     671,   672,   246,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,   673,
       0,   201,     0,     0,     0,     0,   113,    31,    32,    33,
       0,     0,     0,     0,     0,     0,     0,    37,     0,     0,
       0,    81,    82,     0,    83,    84,    85,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   674,    69,    70,    71,    72,    73,     0,   800,
     801,     0,     0,     0,   675,     0,     0,     0,     0,   174,
      76,    77,    78,     0,   676,     0,    81,    82,   802,    83,
      84,    85,     0,     0,    87,     0,   803,   804,    33,     0,
       0,     0,     0,     0,   677,     0,   805,     0,     0,    92,
       0,     0,   678,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    27,    28,     0,
       0,     0,     0,     0,     0,     0,    33,     0,   199,     0,
       0,   806,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   807,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    81,    82,     0,    83,    84,
      85,     0,     0,     0,     0,     0,   200,     0,   789,     0,
       0,     0,     0,   808,     0,     0,    33,     0,   199,     0,
       0,   809,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   174,     0,     0,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    88,   200,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    33,     0,   199,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,   174,   406,     0,    78,
       0,    80,   113,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    33,     0,   199,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,   174,   201,     0,    78,
       0,    80,   113,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   500,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,   174,   201,     0,    78,
     484,    80,   113,    81,    82,     0,    83,    84,    85,    33,
       0,   199,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,     0,   201,     0,   200,
       0,     0,   113,     0,     0,     0,     0,     0,     0,     0,
       0,   894,     0,    33,     0,   199,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   174,
       0,     0,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,   943,   944,
      33,     0,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,    33,     0,   199,
     201,     0,     0,   174,     0,   113,    78,     0,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    95,   212,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,    33,     0,     0,   201,     0,     0,    81,    82,   113,
      83,    84,    85,     0,     0,     0,     0,   174,     0,     0,
      78,    33,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,     0,   213,     0,
       0,     0,     0,   113,   289,     0,     0,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    33,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1339,     0,    81,    82,
    1340,    83,    84,    85,    95,    33,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,     0,     0,    95,   290,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,  1198,     0,     0,     0,  1186,     0,     0,     0,
      33,     0,     0,     0,     0,     0,   174,     0,  1187,    78,
       0,    80,     0,    81,    82,    33,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   174,     0,     0,    78,     0,
    1188,     0,    81,    82,     0,    83,  1189,    85,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,     0,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   261,     0,     0,     0,    81,    82,     0,
      83,    84,    85,     0,     0,     0,     0,     0,     0,     0,
     324,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   336,   337,   338,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   339,
       0,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   336,   337,   338,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     339,     0,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,     0,   361,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   336,
     337,   338,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   339,   411,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     336,   337,   338,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   339,   519,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,     0,
     361,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   336,   337,   338,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   339,   778,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,   336,   337,   338,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   339,   818,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,     0,   361,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   336,   337,   338,     0,     0,     0,  1049,     0,
       0,     0,     0,     0,     0,     0,     0,   657,   339,   775,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,     0,   361,   336,   337,   338,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   339,
       0,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   337,   338,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   339,
       0,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-1217))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-788))

static const yytype_int16 yycheck[] =
{
       4,   131,    86,   156,     4,   176,    90,    91,    30,   552,
       4,     4,   320,   628,    50,     4,   398,     4,    40,   912,
      28,   373,    44,   214,     4,   361,   427,   405,   159,   393,
     768,     4,   752,   317,   745,  1044,    53,   220,   515,   220,
     902,     9,    46,   656,     9,    49,   130,   177,    27,     9,
      27,    24,    25,     9,   166,   419,   302,     9,    75,     9,
     110,    78,    66,     4,   241,   242,     9,    42,   122,   246,
      62,    42,    62,     9,     9,   476,     9,     9,     9,     9,
       9,     9,    86,     9,     9,     9,    90,    91,     9,     9,
     221,     9,     9,     9,     9,     9,   110,    94,    95,     9,
      74,    42,   662,    47,    30,   103,   122,    74,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      79,   122,   107,   108,   109,    74,   130,   771,    74,     0,
      42,  1140,   122,  1142,   193,   860,   193,   110,   148,   864,
     317,   951,   786,   197,   131,   955,  1362,   525,    93,    94,
      95,   201,   863,    91,    60,    61,   193,   328,     8,   100,
      62,    62,   160,   213,   105,    62,   107,   108,   109,   110,
     111,   112,   113,   193,    42,    42,   156,    62,   194,   138,
      62,    62,   186,   158,  1046,   923,   196,   158,   926,    62,
     177,  1053,  1054,   194,  1410,   141,    62,    62,    62,    62,
     197,   193,    62,   147,   196,    62,   196,   145,   149,   150,
      62,   152,   164,   591,    62,  1224,   122,   196,   195,   194,
     191,   122,   226,   194,   198,   275,   230,   195,   201,   400,
     234,   198,   197,   174,   207,   195,   196,   226,   194,   323,
     213,   230,   372,   954,   261,   195,   158,   251,   649,   198,
     164,   194,   653,   226,   164,   367,   197,   230,   735,   195,
     195,   275,   195,   195,   195,   195,   195,   195,   194,   446,
     195,   195,   289,   194,   194,   387,   194,   194,   194,   194,
      47,   841,   194,   843,   141,   369,   370,  1149,   194,   667,
     158,   158,   936,   266,   196,   196,   193,   409,   193,   196,
     273,   274,   275,   307,    32,   163,   418,   280,   196,   421,
    1048,   196,   316,   286,   196,   196,   320,   193,    32,   323,
     307,    94,    95,   196,   191,    74,   194,   307,    91,    79,
     196,   196,   196,   196,   307,   193,   196,   373,    74,   196,
     722,   956,    74,    79,   196,   147,    74,    79,   196,    74,
     527,   528,   535,   361,   535,  1364,   406,   534,   971,   367,
      74,   118,    62,   367,   368,   369,   370,   107,   108,   109,
     110,   111,   112,   107,   108,   109,   110,   111,   112,   387,
     147,   118,   145,   387,   371,   669,   193,  1098,   138,   138,
     139,   140,    24,    25,    74,   162,    28,    32,   193,    79,
     193,   409,   593,   139,   140,   409,    74,   139,   140,    32,
     418,    79,  1150,   421,   139,   140,   193,   421,    74,   148,
     784,    74,   122,    79,   197,   398,    79,   618,   599,   793,
     434,    74,   161,   406,   174,   813,    79,   838,   193,    74,
     174,   193,     4,   634,   145,   434,  1061,  1158,  1063,    32,
     633,    74,   633,   789,   145,   195,   639,    32,   639,   139,
     140,   434,    67,    68,   481,    14,   202,   196,   141,   201,
     138,   139,   140,  1482,   558,    67,    68,   364,    27,   483,
      42,   859,   138,   139,   140,   193,   139,   140,   679,  1498,
     163,    74,   669,   670,   498,    44,   139,   140,   196,    74,
     504,   505,    27,   390,   195,   196,   752,   394,   639,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   193,   768,   196,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    32,   141,   100,    62,
      26,  1156,    62,   105,   896,   107,   108,   109,   110,   111,
     112,   113,    62,    99,   558,    60,    61,    43,   195,   163,
      46,   107,   108,   109,   110,   111,   112,   195,   932,   201,
      60,    61,   950,   750,   283,   207,   940,   550,   287,    26,
     195,   213,   195,   196,   761,   120,   121,   149,   150,   193,
     152,   973,   196,   195,   196,  1488,    43,  1388,  1389,    46,
     309,   195,   311,   312,   313,   314,    93,    94,    95,   195,
    1011,  1504,   174,   114,   115,   116,  1231,   122,   195,   997,
    1384,  1385,   195,   141,   628,  1003,   630,   600,   174,   196,
     652,    62,   122,   193,   266,   197,  1018,   193,   176,   177,
     178,   273,   274,   647,   756,   183,   184,   141,   280,   187,
     188,   193,   145,    41,   286,   659,   660,    46,    47,    48,
     647,    50,   635,    50,   637,   163,   141,   647,   845,   122,
     847,   660,   200,  1074,   647,     9,   141,   923,  1042,   141,
     926,   193,   122,     8,   657,   163,  1050,   660,  1089,   194,
      14,  1069,   195,   193,  1058,    14,    74,   701,   195,    14,
     704,    80,   163,   195,   788,   194,    14,   195,    91,   194,
     887,   194,   193,    99,   701,    43,    44,    45,    46,    47,
      48,   701,    50,   199,     4,   916,   193,   193,   701,   361,
     734,     9,   909,    83,   734,   912,   194,  1457,   711,   194,
     734,   734,     9,   195,    14,   734,     9,   734,   756,   722,
     723,  1366,   756,    80,   734,  1475,   193,   179,    74,   182,
    1161,   734,    42,  1483,    74,    74,   398,   193,  1169,    74,
    1016,   195,   120,  1137,   406,   194,   194,  1178,  1024,   193,
      62,   789,   194,   121,   788,   162,   123,     9,   194,    14,
     191,     9,   194,    62,     9,    14,   800,   801,   802,   120,
     200,   200,  1048,   825,  1112,   197,     9,   200,   193,   193,
     987,   123,   194,   193,     9,   141,   179,   179,   194,   200,
     100,   195,   826,   195,   828,   105,   830,   107,   108,   109,
     110,   111,   112,   113,   194,  1236,   193,   826,   193,   193,
     141,   828,    14,   830,   196,     9,    74,   196,   828,    14,
     830,   195,  1216,   826,    91,   828,   196,   830,   862,   832,
     896,   865,  1405,    14,   200,   196,    14,    27,    14,   149,
     150,     9,   152,   195,   194,   107,   108,   109,   110,   111,
     112,   195,   193,   123,   888,    14,   118,   119,   195,   193,
     193,   193,     9,   193,   174,  1141,   900,  1205,   193,   193,
     900,  1147,  1148,   194,  1150,   194,   900,   900,   200,   123,
       4,   900,     9,   900,   138,    74,  1093,   197,   550,     9,
     900,   193,   123,   155,   195,    74,   194,   900,    14,   193,
    1473,   196,  1109,   193,   193,   196,    24,    25,   194,   196,
      28,   963,   174,   193,   123,  1122,   919,   951,    42,   194,
       9,   955,   956,   200,   138,    27,  1357,    72,  1359,    74,
      69,   195,   966,   194,   164,   195,  1367,    27,   600,   123,
       9,  1124,     9,   977,   194,    91,   123,   966,   194,    14,
     197,   194,  1228,   194,   193,   197,   194,   960,   194,   194,
     977,   123,  1122,   966,   196,   193,     9,   977,   194,    27,
     973,   974,   194,   635,   977,   637,   100,  1408,   194,   194,
     123,   105,   195,   107,   108,   109,   110,   111,   112,   113,
     194,   196,   195,   151,   195,   657,   147,    24,    25,    74,
      14,    28,  1203,   194,   149,   150,  1016,   152,   153,   154,
     193,   105,   194,   123,  1024,  1018,   123,   123,    14,   194,
     194,   194,    49,    74,   196,   149,   150,  1061,   152,  1063,
     175,   195,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    14,   194,   196,    14,   711,
     174,   196,   193,   198,   194,   123,    14,   195,   195,   194,
     722,   723,   196,    52,    74,   193,    74,     9,    74,   195,
     103,  1502,    91,   197,  1108,   141,    91,  1508,  1112,    30,
     154,    14,   193,  1117,   160,   194,    74,   195,   193,   207,
     194,  1108,   156,     9,    74,   195,   194,   194,  1108,    74,
     196,    14,    14,    74,  1121,  1108,    14,    14,   481,  1465,
     156,   370,   368,   787,  1124,   785,   369,   737,  1479,   974,
    1475,   486,  1156,  1253,  1117,  1337,  1507,   789,  1162,  1086,
    1349,  1141,  1166,  1496,    38,  1223,  1170,  1147,  1148,   910,
     877,   874,   465,   903,   373,  1162,   698,  1166,   266,   465,
    1351,   801,  1162,  1170,   938,   273,   274,   274,  1192,  1162,
    1170,   815,   280,  1166,  1198,    -1,   281,  1170,   286,   848,
     832,  1205,    -1,    -1,   201,    -1,    -1,    -1,    -1,    -1,
     207,  1457,    -1,    -1,    -1,    -1,   213,    -1,    -1,    -1,
    1207,    -1,    -1,    -1,    -1,    -1,    -1,  1231,    -1,  1475,
    1234,  1235,    -1,    -1,    -1,  1239,    -1,  1483,    -1,    -1,
      -1,  1245,    -1,    -1,   241,   242,  1235,  1234,  1228,   246,
      -1,    -1,  1239,    -1,  1234,    -1,    -1,    -1,  1245,  1239,
      -1,  1234,  1235,    -1,  1348,  1245,  1239,    -1,    -1,   266,
      -1,    -1,  1245,   361,    -1,    -1,   273,   274,    -1,    -1,
      -1,    -1,    -1,   280,    -1,    -1,    -1,   919,    -1,   286,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1453,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     398,  1488,    -1,    -1,    -1,    -1,  1400,    -1,    -1,    -1,
     317,    -1,    -1,   320,    -1,    -1,    -1,  1504,   960,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1469,
      -1,   973,   974,    -1,  1348,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    70,    71,    -1,
      -1,    -1,  1366,   451,   361,    -1,  1370,    80,    -1,    -1,
      -1,    -1,    -1,     4,    -1,  1379,    -1,    -1,    -1,    -1,
    1384,  1385,    -1,  1370,  1388,  1389,  1018,    -1,    -1,    -1,
    1370,    60,    61,    -1,    -1,    -1,  1400,  1370,    -1,   451,
     488,   398,  1406,  1407,    -1,    -1,    -1,    -1,  1412,   406,
      -1,    42,    -1,   126,   127,   128,   129,   130,    -1,  1406,
    1407,    -1,    -1,    -1,   137,  1412,  1406,  1407,    -1,    -1,
     143,   144,  1412,  1406,  1407,    -1,   488,    -1,    -1,  1412,
      -1,  1445,    -1,    -1,   157,    -1,    -1,    -1,  1452,   446,
      -1,    -1,    -1,   122,   451,    -1,    -1,    -1,  1445,   172,
      -1,    -1,   550,    -1,  1468,  1445,    -1,    -1,    -1,   100,
      -1,    -1,  1445,  1453,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,    -1,    -1,     4,    -1,    -1,    -1,    -1,
      -1,   488,    -1,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,  1509,    50,    -1,    -1,    -1,
    1514,    -1,   600,    -1,    -1,    -1,    -1,    -1,   149,   150,
      -1,   152,  1509,    42,    -1,    -1,    -1,  1514,    -1,  1509,
     527,   528,    -1,    -1,  1514,    -1,  1509,   534,    -1,    -1,
      -1,  1514,    -1,   174,    -1,    -1,    -1,   635,    -1,   637,
      -1,    -1,    -1,   550,    -1,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,   657,
     658,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,   671,   672,   673,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   600,    -1,    -1,   658,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   702,    -1,    -1,    -1,    -1,   671,
     672,   673,    -1,   711,    -1,    -1,    -1,    -1,    -1,    -1,
     149,   150,   720,   152,   722,   723,    -1,    -1,   635,    -1,
     637,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   737,
      -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,
     657,   658,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   669,   670,   671,   672,   673,    -1,   197,   767,
      -1,    -1,   770,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,   789,    50,    -1,    -1,   702,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   711,   767,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   720,    -1,   722,   723,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     737,    -1,    -1,    -1,   832,    -1,    -1,    -1,    -1,    -1,
     241,   242,    -1,   750,    -1,   246,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   761,    -1,    -1,    -1,    -1,    -1,
     767,    -1,   860,   770,    -1,    -1,   864,    -1,   866,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   789,    -1,    -1,    -1,   884,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   860,    -1,
      -1,    -1,   864,    -1,   866,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   317,    -1,    -1,   320,
      -1,   919,   884,    -1,    -1,   832,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   845,    -1,
     847,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   860,    -1,    -1,    -1,   864,    -1,   866,
      -1,    -1,   960,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   973,   974,   884,   976,    -1,
     887,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   990,    -1,   992,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   909,    -1,    -1,   912,    -1,    -1,    -1,    -1,
      -1,    -1,   919,    -1,   976,    -1,    -1,  1015,    -1,    -1,
    1018,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   990,    -1,
     992,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1039,    -1,    -1,    -1,   446,    -1,    -1,    -1,    -1,
     451,    -1,    -1,   960,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   973,   974,    -1,   976,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1039,    -1,    -1,
     987,    -1,    -1,   990,    -1,   992,    -1,   488,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1097,
      -1,    -1,    -1,    -1,  1102,    -1,  1104,    -1,  1015,    -1,
      -1,  1018,    -1,    -1,    -1,  1113,    -1,  1115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   527,   528,    10,    11,
      12,    -1,  1039,   534,    -1,  1097,  1134,    -1,    -1,    -1,
    1102,    -1,  1104,    -1,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    -1,  1134,    -1,  1172,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1181,  1182,    -1,  1093,    -1,    -1,    -1,
    1097,    -1,    -1,    -1,    -1,  1102,    -1,  1104,    -1,    -1,
      -1,    -1,  1109,    -1,    -1,  1112,  1113,    -1,  1115,    -1,
    1172,    -1,    -1,    -1,    -1,  1122,  1214,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1134,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1242,  1243,    -1,    -1,    -1,    -1,
    1248,  1249,  1214,    26,    -1,  1253,    -1,   658,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1172,    -1,    -1,   669,   670,
     671,   672,   673,    -1,  1181,  1182,    -1,    -1,    -1,    52,
    1242,  1243,    -1,    -1,    -1,    -1,  1248,    -1,    -1,    -1,
    1252,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1205,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1214,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1242,  1243,    -1,    -1,    -1,
      -1,  1248,  1249,    -1,    -1,    -1,  1253,    -1,    -1,   750,
      -1,    -1,   125,    -1,  1352,    -1,    -1,    -1,    -1,    -1,
     761,    -1,    -1,    -1,    -1,    -1,   767,    -1,    -1,   142,
      -1,    -1,   145,    -1,   147,    -1,   149,   150,    -1,   152,
     153,   154,    -1,    -1,    -1,  1383,    -1,    -1,    -1,    -1,
    1352,    -1,    -1,    -1,   167,    -1,  1394,    -1,    -1,    -1,
      -1,  1399,   175,    -1,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,    -1,    -1,
     193,  1383,    72,    -1,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1394,    -1,    -1,    -1,    -1,  1399,    -1,    -1,
      -1,    -1,    -1,    -1,   845,  1352,   847,    -1,   451,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1456,   860,
    1422,    -1,    -1,   864,    -1,   866,  1464,    -1,    -1,    -1,
      -1,    -1,  1379,    -1,    -1,    -1,  1383,    -1,    -1,    -1,
      -1,  1479,    -1,   884,    -1,   488,   887,  1394,    -1,    -1,
      -1,    -1,  1399,    -1,  1456,    -1,    -1,    -1,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,    -1,   909,    -1,
      -1,   912,    -1,    -1,  1512,    -1,    -1,    -1,    72,    -1,
    1518,    -1,    -1,    -1,  1522,   175,  1524,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
      -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,   198,  1456,
    1512,    -1,    -1,    -1,    -1,    -1,  1518,  1464,    -1,    -1,
    1522,    -1,  1524,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1479,    -1,    -1,   976,    -1,    -1,    -1,    -1,
      -1,  1488,    -1,    -1,    -1,    -1,   987,    -1,    -1,   990,
      -1,   992,    -1,    -1,    -1,   149,   150,  1504,   152,   153,
     154,    -1,    -1,    -1,    -1,  1512,    -1,    -1,    -1,    -1,
      -1,  1518,    -1,    -1,    -1,  1522,    -1,  1524,    -1,    -1,
      -1,   175,    -1,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,    -1,    -1,  1039,   193,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   658,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   671,   672,
     673,    -1,     5,     6,    -1,     8,     9,    10,    -1,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,  1093,    26,    27,    -1,  1097,    -1,    -1,    -1,
      -1,  1102,    -1,  1104,    -1,    38,    -1,    -1,  1109,    -1,
      -1,  1112,    45,    -1,    47,    -1,    -1,    50,    -1,    52,
      -1,  1122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1134,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    79,    -1,   451,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      93,    -1,    -1,    -1,   767,    -1,    -1,    -1,    -1,    -1,
      -1,  1172,    -1,    -1,    -1,    -1,    -1,   110,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   488,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,  1205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,  1214,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,  1242,  1243,    -1,    -1,    -1,    -1,  1248,   181,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   860,    -1,    -1,
      -1,   864,    -1,   866,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,   884,    50,    -1,    -1,    10,    11,    12,    -1,   222,
      -1,    -1,   225,    -1,    -1,    -1,    -1,    -1,    -1,   232,
     233,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,   275,    50,    -1,    -1,    -1,    -1,   281,    -1,
      -1,  1352,   285,    -1,    -1,   658,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   671,   672,
      -1,    -1,    -1,   976,    -1,   308,    -1,    -1,  1379,    -1,
      -1,    -1,  1383,   197,    -1,    -1,   319,   990,    -1,   992,
      -1,    -1,    -1,  1394,    -1,    -1,    -1,    -1,  1399,    -1,
      -1,    -1,    -1,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,    -1,    -1,
     363,   364,    -1,   366,    -1,    -1,  1039,    -1,    -1,    -1,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,    -1,    -1,    -1,  1456,    -1,   390,   391,    -1,
     393,   394,   395,    -1,   767,    -1,    -1,    -1,   401,    -1,
      -1,    -1,   197,    -1,    -1,    -1,    -1,    -1,   411,    -1,
     413,    -1,    -1,    -1,    -1,    -1,   419,  1488,    -1,    -1,
      -1,    -1,    -1,    -1,  1097,    -1,   429,    -1,   431,  1102,
      -1,  1104,    -1,  1504,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1512,    -1,    -1,    -1,    -1,    -1,  1518,    -1,    -1,
      -1,  1522,    -1,  1524,    -1,   458,    -1,    -1,   461,   462,
     463,  1134,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   484,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   866,    -1,    52,    -1,    -1,    -1,  1172,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,   884,    -1,    -1,    -1,    72,   519,    -1,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,  1214,    50,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,   562,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,  1242,
    1243,    -1,    -1,    -1,    -1,  1248,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   142,    -1,    72,   145,   592,
     147,    -1,   149,   150,    -1,   152,   153,   154,   601,    -1,
      -1,   158,    -1,   976,    -1,    -1,    -1,    -1,    -1,    -1,
     167,    -1,    -1,    -1,   617,    -1,    -1,   990,   175,   992,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,    -1,    -1,   193,    72,    -1,    -1,
     643,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   656,    -1,    -1,    -1,   142,    -1,    -1,
     145,    -1,    -1,    -1,   149,   150,  1039,   152,   153,   154,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1352,
      -1,   116,    -1,    -1,   687,    -1,    -1,    -1,    -1,   197,
     175,    -1,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,    -1,    -1,    -1,    -1,
    1383,    -1,   197,    -1,   149,   150,    -1,   152,   153,   154,
      -1,  1394,    -1,    -1,  1097,    -1,  1399,    -1,   731,  1102,
      -1,  1104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     175,   744,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,    -1,    -1,   193,    -1,
      -1,  1134,   765,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   775,    -1,    -1,   778,    -1,   780,    -1,    -1,
      -1,   784,    -1,  1456,    -1,    -1,    -1,    -1,    -1,    -1,
     793,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1172,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   818,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,  1512,
      50,  1214,    -1,    -1,    -1,  1518,    -1,    -1,    -1,  1522,
      -1,  1524,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   868,   869,   870,    -1,  1242,
    1243,   874,   875,    -1,    -1,  1248,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      -1,    74,    -1,   896,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   915,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,   932,
      -1,    -1,    -1,    -1,    -1,    26,    -1,   940,    26,   942,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    52,    50,    -1,    -1,    -1,   149,   150,   971,   152,
     153,   154,    -1,    -1,    -1,    -1,    -1,   980,    -1,  1352,
      -1,    72,    -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,
      -1,    -1,   175,    -1,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,    -1,    -1,
    1383,    -1,    -1,   196,    -1,   198,    -1,    -1,    -1,    -1,
      -1,  1394,    -1,    -1,  1027,    -1,  1399,    -1,  1031,    -1,
    1033,    -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,  1042,
      -1,    -1,    -1,    -1,    -1,    -1,  1049,  1050,    -1,    -1,
      -1,   142,    -1,    -1,   145,  1058,   147,    -1,   149,   150,
      -1,   152,   153,   154,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,    -1,
      -1,    -1,    -1,  1456,   175,    -1,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
      -1,    -1,   193,  1106,    -1,    -1,    -1,    -1,    -1,   197,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1137,    -1,    -1,    -1,    -1,  1512,
      -1,    -1,    -1,    -1,    -1,  1518,    -1,    -1,    -1,  1522,
      -1,  1524,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
     100,   101,   102,  1216,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,  1244,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,
     170,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      12,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
     100,   101,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,
     170,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    26,    13,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
     100,   101,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,
     170,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      90,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    88,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    86,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,   197,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,   155,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    72,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   149,   150,   193,   152,   153,   154,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,   175,    -1,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    72,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,   146,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   149,   150,   193,   152,   153,   154,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,   175,    -1,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,   195,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    96,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,   194,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,   196,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    -1,    -1,    -1,    -1,   198,   199,
      -1,   201,   202,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,
      -1,    -1,   142,   143,   144,   145,    -1,   147,    -1,   149,
     150,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,    -1,    -1,
      -1,    -1,   172,    -1,   174,   175,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,   193,    10,    11,    12,    -1,   198,   199,
      -1,   201,   202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,   197,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,   197,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   197,    -1,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   197,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    10,    11,    12,    13,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,   197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,
      68,    69,    -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   197,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   195,    -1,   124,   125,   126,   127,
     128,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   142,   143,   144,    -1,   146,    -1,
      -1,   149,   150,    -1,   152,   153,   154,   155,   156,   157,
      -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,    -1,   167,
     168,   181,   170,    -1,   172,   173,   174,   175,    -1,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,   195,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   195,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   195,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
     195,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,   195,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      10,    11,    12,    -1,    -1,    72,    -1,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   194,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   194,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      26,    50,   149,   150,    -1,   152,   153,   154,    -1,    -1,
      -1,    -1,   189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,   175,    -1,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,    -1,    -1,    72,    -1,    -1,   196,
      -1,   198,    -1,   185,   186,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
      32,    52,   182,    -1,    -1,    -1,    -1,    -1,    -1,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    72,    -1,    -1,    -1,    -1,   142,    -1,    -1,   145,
      -1,   147,    -1,   149,   150,    -1,   152,   153,   154,    -1,
      72,   180,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,   174,   175,
      -1,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   125,    -1,    -1,   193,    -1,    -1,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   142,   124,    -1,   145,    -1,   147,    -1,   149,   150,
      -1,   152,   153,   154,    72,    -1,    74,    75,    -1,    -1,
     142,    -1,    -1,   145,    -1,   147,   167,   149,   150,    -1,
     152,   153,   154,    -1,   175,    -1,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
      43,    44,   193,   175,    -1,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,    -1,    62,
      -1,   193,    -1,    -1,    -1,    -1,   198,    70,    71,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,
      -1,   149,   150,    -1,   152,   153,   154,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   175,    -1,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   125,   126,   127,   128,   129,   130,    -1,    43,
      44,    -1,    -1,    -1,   137,    -1,    -1,    -1,    -1,   142,
     143,   144,   145,    -1,   147,    -1,   149,   150,    62,   152,
     153,   154,    -1,    -1,   157,    -1,    70,    71,    72,    -1,
      -1,    -1,    -1,    -1,   167,    -1,    80,    -1,    -1,   172,
      -1,    -1,   175,    -1,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,    74,    -1,
      -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   149,   150,    -1,   152,   153,
     154,    -1,    -1,    -1,    -1,    -1,   112,    -1,    64,    -1,
      -1,    -1,    -1,   167,    -1,    -1,    72,    -1,    74,    -1,
      -1,   175,    -1,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   142,    -1,    -1,   145,
      -1,   147,    -1,   149,   150,    -1,   152,   153,   154,    -1,
      -1,    -1,    -1,    -1,    -1,   161,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,    74,   175,
      -1,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,    -1,    -1,   142,   193,    -1,   145,
      -1,   147,   198,   149,   150,    -1,   152,   153,   154,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,    74,   175,
      -1,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,    -1,    -1,   142,   193,    -1,   145,
      -1,   147,   198,   149,   150,    -1,   152,   153,   154,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   175,
      -1,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,    -1,    -1,   142,   193,    -1,   145,
     196,   147,   198,   149,   150,    -1,   152,   153,   154,    72,
      -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   175,
      -1,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,    -1,    -1,    -1,   193,    -1,   112,
      -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   142,
      -1,    -1,   145,    -1,   147,    -1,   149,   150,    -1,   152,
     153,   154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    70,    71,
      72,    -1,   175,    -1,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    72,    -1,    74,
     193,    -1,    -1,   142,    -1,   198,   145,    -1,   147,    -1,
     149,   150,    -1,   152,   153,   154,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   175,   112,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,    72,    -1,    -1,   193,    -1,    -1,   149,   150,   198,
     152,   153,   154,    -1,    -1,    -1,    -1,   142,    -1,    -1,
     145,    72,   147,    -1,   149,   150,    -1,   152,   153,   154,
      -1,    -1,    -1,   175,    -1,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,    -1,    -1,
     175,    -1,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,    -1,    -1,   193,    -1,
      -1,    -1,    -1,   198,   145,    -1,    -1,    -1,   149,   150,
      -1,   152,   153,   154,    -1,    -1,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,   149,   150,
     151,   152,   153,   154,   175,    72,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
      -1,    -1,    -1,    -1,   175,   196,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
      -1,    -1,   193,    -1,    -1,    -1,   113,    -1,    -1,    -1,
      72,    -1,    -1,    -1,    -1,    -1,   142,    -1,   125,   145,
      -1,   147,    -1,   149,   150,    72,   152,   153,   154,    -1,
      -1,    -1,    -1,    -1,    -1,   142,    -1,    -1,   145,    -1,
     147,    -1,   149,   150,    -1,   152,   153,   154,    -1,   175,
      -1,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,    -1,    -1,    -1,    -1,   175,    -1,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   145,    -1,    -1,    -1,   149,   150,    -1,
     152,   153,   154,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,   149,   150,    -1,   152,   153,   154,    -1,    -1,
      -1,    -1,    -1,   175,    -1,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   175,    -1,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,   123,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,   123,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,   123,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,   123,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,   123,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   204,   205,     0,   206,     3,     4,     5,     6,     7,
      13,    42,    43,    44,    49,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    70,    71,    72,    73,    74,    76,    80,    81,    82,
      83,    85,    87,    89,    92,    96,    97,    98,    99,   100,
     101,   102,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   114,   115,   116,   117,   118,   119,   124,   125,   126,
     127,   128,   129,   130,   137,   142,   143,   144,   145,   146,
     147,   149,   150,   152,   153,   154,   155,   157,   161,   167,
     168,   170,   172,   173,   174,   175,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     193,   195,   196,   198,   199,   201,   202,   207,   210,   213,
     214,   215,   216,   217,   218,   221,   236,   237,   241,   246,
     252,   307,   308,   313,   317,   318,   319,   320,   321,   322,
     323,   324,   326,   329,   338,   339,   340,   342,   343,   345,
     364,   374,   375,   376,   381,   384,   402,   407,   409,   410,
     411,   412,   413,   414,   415,   416,   418,   431,   433,   435,
     110,   111,   112,   124,   142,   210,   236,   307,   323,   409,
     323,   193,   323,   323,   323,   400,   401,   323,   323,   323,
     323,   323,   323,   323,   323,   323,   323,   323,   323,    74,
     112,   193,   214,   375,   376,   409,   409,    32,   323,   422,
     423,   323,   112,   193,   214,   375,   376,   377,   408,   414,
     419,   420,   193,   314,   378,   193,   314,   330,   315,   323,
     223,   314,   193,   193,   193,   314,   195,   323,   210,   195,
     323,    26,    52,   125,   147,   167,   193,   210,   217,   436,
     446,   447,   176,   195,   320,   323,   344,   346,   196,   229,
     323,   145,   211,   212,   213,    74,   198,   278,   279,   118,
     118,    74,   280,   193,   193,   193,   193,   210,   250,   437,
     193,   193,    74,    79,   138,   139,   140,   428,   429,   145,
     196,   213,   213,    96,   323,   251,   437,   147,   193,   437,
     437,   323,   331,   313,   323,   324,   409,   219,   196,    79,
     379,   428,    79,   428,   428,    27,   145,   163,   438,   193,
       9,   195,    32,   235,   147,   249,   437,   112,   236,   308,
     195,   195,   195,   195,   195,   195,    10,    11,    12,    26,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    50,   195,    62,    62,   195,   196,   141,   119,   155,
     252,   306,   307,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    60,    61,   122,   404,   405,
      62,   196,   406,   193,    62,   196,   198,   415,   193,   235,
     236,    14,   323,    41,   210,   399,   193,   313,   409,   141,
     409,   123,   200,     9,   386,   313,   409,   438,   141,   193,
     380,   122,   404,   405,   406,   194,   323,    27,   221,     8,
     332,     9,   195,   221,   222,   315,   316,   323,   210,   264,
     225,   195,   195,   195,   447,   447,   163,   193,    99,   439,
     447,    14,   210,    74,   195,   195,   195,   176,   177,   178,
     183,   184,   187,   188,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   359,   360,   361,   230,   103,   160,   195,
     213,     9,   195,    91,   196,   409,     9,   195,    14,     9,
     195,   409,   432,   432,   313,   324,   409,   194,   163,   244,
     124,   409,   421,   422,    62,   122,   138,   429,    73,   323,
     409,    79,   138,   429,   213,   209,   195,   196,   195,   123,
     247,   365,   367,    80,   333,   334,   336,    14,    91,   434,
     158,   274,   275,   402,   403,   194,   194,   194,   197,   220,
     221,   237,   241,   246,   323,   199,   201,   202,   210,   439,
      32,   276,   277,   323,   436,   193,   437,   242,   235,   323,
     323,   323,    27,   323,   323,   323,   323,   323,   323,   323,
     323,   323,   323,   323,   323,   323,   323,   323,   323,   323,
     323,   323,   323,   323,   323,   377,   323,   417,   417,   323,
     424,   425,   196,   210,   414,   415,   250,   251,   249,   236,
      32,   146,   317,   320,   323,   344,   323,   323,   323,   323,
     323,   323,   323,   323,   323,   323,   323,   196,   210,   414,
     417,   323,   276,   417,   323,   421,   235,   194,   193,   398,
       9,   386,   313,   194,   210,    32,   323,    32,   323,   194,
     194,   414,   276,   196,   210,   414,   194,   219,   268,   196,
     323,   323,    83,    27,   221,   262,   195,    91,    14,     9,
     194,    27,   196,   265,   447,    80,   443,   444,   445,   193,
       9,    43,    44,    62,   125,   137,   147,   167,   175,   214,
     215,   217,   341,   375,   381,   382,   383,   179,    74,   323,
      74,    74,   323,   356,   357,   323,   323,   349,   359,   182,
     362,   219,   193,   228,    91,   212,   210,   323,   279,   382,
      74,     9,   194,   194,   194,   194,   194,   195,   210,   442,
     120,   255,   193,     9,   194,   194,    74,    75,   210,   430,
     210,    62,   197,   197,   206,   208,   323,   121,   254,   162,
      47,   147,   162,   369,   123,     9,   386,   194,   447,   447,
      14,   191,     9,   387,   447,   448,   122,   404,   405,   406,
     197,     9,   164,   409,   194,     9,   387,    14,   327,   238,
     120,   253,   193,   437,   323,    27,   200,   200,   123,   197,
       9,   386,   323,   438,   193,   245,   248,   243,   235,    64,
     409,   323,   438,   193,   200,   197,   194,   200,   197,   194,
      43,    44,    62,    70,    71,    80,   125,   137,   167,   175,
     210,   389,   391,   394,   397,   210,   409,   409,   123,   404,
     405,   406,   194,   323,   269,    67,    68,   270,   219,   314,
     219,   316,    32,   124,   259,   409,   382,   210,    27,   221,
     263,   195,   266,   195,   266,     9,   164,   123,     9,   386,
     194,   158,   439,   440,   447,   382,   382,   382,   385,   388,
     193,    79,   141,   193,   193,   141,   196,   323,   179,   179,
      14,   185,   186,   358,     9,   189,   362,    74,   197,   375,
     196,   232,   210,   197,    14,   409,   195,    91,     9,   164,
     256,   375,   196,   421,   124,   409,    14,   200,   323,   197,
     206,   256,   196,   368,    14,   323,   333,   195,   447,    27,
     441,   158,   403,    32,    74,   196,   210,   414,   447,    32,
     323,   382,   274,   193,   375,   254,   328,   239,   323,   323,
     323,   197,   193,   276,   255,   254,   253,   437,   377,   197,
     193,   276,    14,    70,    71,   210,   390,   390,   391,   392,
     393,   193,    79,   138,   193,   193,     9,   386,   194,   398,
      32,   323,   197,    67,    68,   271,   314,   221,   197,   195,
      84,   195,   409,   193,   123,   258,    14,   219,   266,    93,
      94,    95,   266,   197,   447,   447,   443,     9,   194,   194,
     123,   200,     9,   386,   385,   210,   333,   335,   337,   385,
     210,   382,   426,   427,   323,   323,   323,   357,   323,   347,
      74,   233,   382,   447,   210,     9,   281,   194,   193,   317,
     320,   323,   200,   197,   281,   148,   161,   196,   364,   371,
     148,   196,   370,   123,   195,   447,   332,   448,    74,    14,
     323,   438,   193,   409,   194,   274,   196,   274,   193,   123,
     193,   276,   194,   196,   196,   254,   240,   380,   193,   276,
     194,   123,   200,     9,   386,   392,   138,   333,   395,   396,
     392,   391,   409,   314,    27,    69,   221,   195,   316,   421,
     259,   194,   382,    90,    93,   195,   323,    27,   195,   267,
     197,   164,   158,    27,   382,   382,   194,   123,     9,   386,
     194,   194,   123,   197,     9,   386,   180,   194,   219,    91,
     375,     4,   100,   105,   113,   149,   150,   152,   197,   282,
     305,   306,   307,   312,   402,   421,   197,   197,    47,   323,
     323,   323,    32,    74,    14,   382,   197,   193,   276,   441,
     194,   281,   194,   274,   323,   276,   194,   281,   281,   196,
     193,   276,   194,   391,   391,   194,   123,   194,     9,   386,
     194,    27,   219,   195,   194,   194,   226,   195,   195,   267,
     219,   447,   123,   382,   333,   382,   382,   323,   196,   197,
     447,   120,   121,   436,   257,   375,   113,   125,   147,   153,
     291,   292,   293,   375,   151,   297,   298,   116,   193,   210,
     299,   300,   283,   236,   447,     9,   195,   306,   194,   147,
     366,   197,   197,    74,    14,   382,   193,   276,   194,   105,
     325,   441,   197,   441,   194,   194,   197,   197,   281,   274,
     194,   123,   391,   333,   219,   224,    27,   221,   261,   219,
     194,   382,   123,   123,   181,   219,   375,   375,    14,     9,
     195,   196,   196,     9,   195,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    50,    63,    64,    65,    66,    67,
      68,    69,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   124,   125,   126,   127,   128,   129,   130,
     142,   143,   144,   146,   155,   156,   157,   160,   167,   168,
     170,   172,   173,   174,   210,   372,   373,     9,   195,   147,
     151,   210,   300,   301,   302,   195,    74,   311,   235,   284,
     436,   236,    14,   382,   276,   194,   193,   196,   195,   196,
     303,   325,   441,   197,   194,   391,   123,    27,   221,   260,
     219,   382,   382,   323,   197,   195,   195,   382,   375,   287,
     294,   381,   292,    14,    27,    44,   295,   298,     9,    30,
     194,    26,    43,    46,    14,     9,   195,   437,   311,    14,
     235,   382,   194,    32,    74,   363,   219,   219,   196,   303,
     441,   391,   219,    88,   182,   231,   197,   210,   217,   288,
     289,   290,     9,   197,   382,   373,   373,    52,   296,   301,
     301,    26,    43,    46,   382,    74,   193,   195,   382,   437,
      74,     9,   387,   197,   197,   219,   303,    86,   195,    74,
     103,   227,   141,    91,   381,   154,    14,   285,   193,    32,
      74,   194,   197,   195,   193,   160,   234,   210,   306,   307,
     382,   158,   272,   273,   403,   286,    74,   375,   232,   156,
     210,   195,   194,     9,   387,   107,   108,   109,   309,   310,
     272,    74,   257,   195,   441,   158,   403,   448,   194,   194,
     195,   195,   196,   304,   309,    32,    74,   441,   196,   219,
     448,    74,    14,   304,   219,   197,    32,    74,    14,   382,
     197,    74,    14,   382,    14,   382,   382
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
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
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
# define YYLEX yylex (&yylval, &yylloc)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
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


/*----------.
| yyparse.  |
`----------*/

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
    YYLTYPE yyerror_range[3];

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

#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
  if (yypact_value_is_default (yyn))
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
      if (yytable_value_is_error (yyn))
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

/* Line 1806 of yacc.c  */
#line 723 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 726 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 733 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 734 "hphp.y"
    { }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 737 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num()); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 738 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 739 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 740 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 741 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 742 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 745 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 747 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 748 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 749 "hphp.y"
    { _p->onNamespaceStart("");}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 750 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 751 "hphp.y"
    { _p->nns(); (yyval).reset();}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 752 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 758 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 781 "hphp.y"
    { }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 782 "hphp.y"
    { }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 785 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 786 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 787 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 789 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 800 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 801 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 804 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 811 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 818 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 826 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 829 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 835 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 836 "hphp.y"
    { _p->onStatementListStart((yyval));}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 845 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 849 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 854 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 855 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 857 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 861 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 864 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 868 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 870 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 873 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 875 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 878 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 879 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 880 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 881 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 882 "hphp.y"
    { _p->onReturn((yyval), NULL);}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 883 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 884 "hphp.y"
    { _p->onYieldBreak((yyval));}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 885 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 886 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 887 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 888 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 889 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 890 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 893 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 895 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 899 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 906 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 907 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 910 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 911 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 912 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 913 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 917 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 918 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 919 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 920 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 921 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 922 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 923 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 924 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 925 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 926 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 927 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 935 "hphp.y"
    { _p->onNewLabelScope(false);}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 936 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 945 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 946 "hphp.y"
    { (yyval).reset();}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 950 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 952 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 958 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 959 "hphp.y"
    { (yyval).reset();}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 963 "hphp.y"
    { (yyval) = 1;}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 964 "hphp.y"
    { (yyval).reset();}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 968 "hphp.y"
    { _p->pushFuncLocation(); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 973 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 979 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 985 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 991 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 997 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1011 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1014 "hphp.y"
    { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,(yyvsp[(7) - (8)]));
                                         } else {
                                           stmts = (yyvsp[(7) - (8)]);
                                         }
                                         _p->onClass((yyval),(yyvsp[(1) - (8)]).num(),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),
                                                     stmts,0);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 1029 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1032 "hphp.y"
    { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,(yyvsp[(8) - (9)]));
                                         } else {
                                           stmts = (yyvsp[(8) - (9)]);
                                         }
                                         _p->onClass((yyval),(yyvsp[(2) - (9)]).num(),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),
                                                     stmts,&(yyvsp[(1) - (9)]));
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 1046 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));}
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1049 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1054 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1064 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));}
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1067 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1075 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));}
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1078 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1086 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1087 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 1091 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1094 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1097 "hphp.y"
    { (yyval) = T_CLASS;}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1098 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1099 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1103 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1104 "hphp.y"
    { (yyval).reset();}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1107 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1108 "hphp.y"
    { (yyval).reset();}
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1111 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1112 "hphp.y"
    { (yyval).reset();}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1115 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1117 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1120 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1122 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1126 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1127 "hphp.y"
    { (yyval).reset();}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1131 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1132 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);}
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1138 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1143 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1148 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1153 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1163 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1164 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1165 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1166 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1171 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1173 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));}
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1174 "hphp.y"
    { (yyval).reset();}
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval).reset();}
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval).reset();}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1183 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 1184 "hphp.y"
    { (yyval).reset();}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1189 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 1190 "hphp.y"
    { (yyval).reset();}
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1193 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1194 "hphp.y"
    { (yyval).reset();}
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);}
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1198 "hphp.y"
    { (yyval).reset();}
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1203 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1206 "hphp.y"
    { (yyval).reset();}
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1207 "hphp.y"
    { (yyval).reset();}
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1213 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));}
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1217 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1222 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));}
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1227 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));}
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1232 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));}
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1237 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));}
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1243 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));}
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1249 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));}
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1258 "hphp.y"
    { (yyval).reset();}
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval).reset();}
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);}
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1267 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);}
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1271 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);}
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);}
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1283 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);}
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1288 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);}
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1293 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);}
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1300 "hphp.y"
    { (yyval).reset();}
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1303 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);}
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1304 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1312 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1313 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1317 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1318 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1322 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1324 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1325 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1326 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1331 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 1332 "hphp.y"
    { (yyval).reset();}
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1335 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));}
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1336 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));}
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1340 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);}
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1346 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1352 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1359 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1365 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1370 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));}
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1372 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1374 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1376 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), true); }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1378 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), false); }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1379 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1382 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1385 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1386 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1387 "hphp.y"
    { (yyval).reset(); }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1397 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));}
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1400 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1408 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1413 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1416 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1423 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;}
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1429 "hphp.y"
    { (yyval) = 4;}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1430 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1436 "hphp.y"
    { (yyval) = 6;}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1438 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);}
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1444 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);}
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1448 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1449 "hphp.y"
    { scalar_null(_p, (yyval));}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1453 "hphp.y"
    { scalar_num(_p, (yyval), "1");}
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1454 "hphp.y"
    { scalar_num(_p, (yyval), "0");}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1458 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1461 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1471 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;}
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1472 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1474 "hphp.y"
    { (yyval) = 0;}
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1478 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1479 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);}
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1480 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1481 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1485 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1486 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1487 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1488 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1489 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1491 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1493 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1497 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1500 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;}
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1501 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1505 "hphp.y"
    { (yyval).reset();}
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1506 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1510 "hphp.y"
    { (yyval).reset();}
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1511 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1514 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1515 "hphp.y"
    { (yyval).reset();}
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1518 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1519 "hphp.y"
    { (yyval).reset();}
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));}
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1527 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1528 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1529 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1530 "hphp.y"
    { (yyval) = T_STATIC;}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1531 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1532 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1533 "hphp.y"
    { (yyval) = T_ASYNC;}
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1537 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1538 "hphp.y"
    { (yyval).reset();}
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 1541 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1542 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1543 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1556 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1560 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1562 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1563 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);}
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1564 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1565 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1568 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1573 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1577 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1578 "hphp.y"
    { (yyval).reset();}
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1582 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));}
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1583 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1587 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1592 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1596 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1600 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1605 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1609 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1610 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1611 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1615 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));}
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1616 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1617 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);}
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1620 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));}
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1621 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);}
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1622 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);}
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1623 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);}
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1624 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);}
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1625 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);}
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1626 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);}
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1627 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);}
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1628 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);}
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1629 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);}
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1630 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);}
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1631 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);}
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1632 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);}
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1633 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);}
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1634 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);}
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1635 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);}
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1636 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);}
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1637 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);}
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1638 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);}
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1639 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);}
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1640 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);}
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1641 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');}
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1642 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');}
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1643 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');}
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1644 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');}
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1645 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');}
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1646 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');}
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1647 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');}
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1648 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');}
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1649 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');}
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1650 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);}
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1651 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);}
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1652 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1653 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1654 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);}
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1655 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);}
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1656 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);}
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1657 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);}
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1658 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);}
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1659 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);}
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1660 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');}
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1661 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);}
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1663 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');}
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1664 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);}
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1667 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);}
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1668 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1669 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1670 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));}
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1671 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1672 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);}
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1673 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);}
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1674 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);}
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1675 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);}
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1676 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);}
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1677 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);}
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1678 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);}
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1679 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);}
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1680 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);}
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1681 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1682 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1683 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1684 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));}
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1685 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);}
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1688 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval).reset();}
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1701 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1707 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1715 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1721 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1730 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);}
    break;

  case 406:

/* Line 1806 of yacc.c  */
#line 1738 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 407:

/* Line 1806 of yacc.c  */
#line 1745 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 1753 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 409:

/* Line 1806 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));}
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 1765 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 1769 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); }
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 1777 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 1780 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 1787 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 1790 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval).reset(); }
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval).reset(); }
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 1806 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);}
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 1810 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 1811 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 1816 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 1823 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 1830 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 1832 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 1842 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 1850 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 1855 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); }
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 1857 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 1859 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 1865 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 1867 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 1871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 1874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 1875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 1876 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 1880 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 1884 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 1888 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 1893 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); }
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 1898 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 1902 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 1907 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 1911 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); }
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 1912 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 1916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 1917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 1921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 1922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 1926 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 1930 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 1934 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 1938 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 1939 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 1940 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 1941 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 1948 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));}
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 1951 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),file,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),line,0);
                                         (yyval).setText("");}
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 1962 "hphp.y"
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),file,0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),line,0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());}
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval).reset(); (yyval).setText("");}
    break;

  case 470:

/* Line 1806 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 471:

/* Line 1806 of yacc.c  */
#line 1979 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);}
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 1980 "hphp.y"
    { (yyval).reset();}
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 1983 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);}
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 1984 "hphp.y"
    { (yyval).reset();}
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 1987 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 1991 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 1994 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 1997 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       }
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 2004 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 2005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 481:

/* Line 1806 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 482:

/* Line 1806 of yacc.c  */
#line 2011 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);}
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 2013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);}
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 2016 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 2017 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 2020 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 2021 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 2022 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 491:

/* Line 1806 of yacc.c  */
#line 2023 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 492:

/* Line 1806 of yacc.c  */
#line 2024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 2025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 2026 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 2027 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 2028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 2029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 498:

/* Line 1806 of yacc.c  */
#line 2030 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 499:

/* Line 1806 of yacc.c  */
#line 2031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 500:

/* Line 1806 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 2033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 2034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 2035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 2036 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 2037 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 2038 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 2039 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 2041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 510:

/* Line 1806 of yacc.c  */
#line 2042 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 2043 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 512:

/* Line 1806 of yacc.c  */
#line 2044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 2045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 2046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 2047 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 2048 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 2049 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 2050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 2051 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 2052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 2055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 2056 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 2057 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 2058 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 2059 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 2060 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 2061 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 2062 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 2063 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 2066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 2068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 2069 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 2070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 2071 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 2072 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 2073 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 2074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 545:

/* Line 1806 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 2080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 555:

/* Line 1806 of yacc.c  */
#line 2087 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 557:

/* Line 1806 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 560:

/* Line 1806 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2100 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2105 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2109 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 569:

/* Line 1806 of yacc.c  */
#line 2110 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);}
    break;

  case 570:

/* Line 1806 of yacc.c  */
#line 2114 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 571:

/* Line 1806 of yacc.c  */
#line 2115 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 572:

/* Line 1806 of yacc.c  */
#line 2116 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);}
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval).reset();}
    break;

  case 574:

/* Line 1806 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval).reset();}
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval).reset();}
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2127 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);}
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 580:

/* Line 1806 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval).reset();}
    break;

  case 581:

/* Line 1806 of yacc.c  */
#line 2137 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2138 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2139 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2141 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));}
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2142 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));}
    break;

  case 586:

/* Line 1806 of yacc.c  */
#line 2143 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));}
    break;

  case 587:

/* Line 1806 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));}
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));}
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));}
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2148 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2149 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));}
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 594:

/* Line 1806 of yacc.c  */
#line 2154 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 595:

/* Line 1806 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2160 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 598:

/* Line 1806 of yacc.c  */
#line 2161 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 600:

/* Line 1806 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 601:

/* Line 1806 of yacc.c  */
#line 2166 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); }
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2176 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2178 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 607:

/* Line 1806 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 608:

/* Line 1806 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 610:

/* Line 1806 of yacc.c  */
#line 2186 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 611:

/* Line 1806 of yacc.c  */
#line 2187 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));}
    break;

  case 612:

/* Line 1806 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));}
    break;

  case 613:

/* Line 1806 of yacc.c  */
#line 2190 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));}
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 615:

/* Line 1806 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval).reset();}
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval).reset();}
    break;

  case 617:

/* Line 1806 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval).reset();}
    break;

  case 618:

/* Line 1806 of yacc.c  */
#line 2204 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();}
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval).reset();}
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2211 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 621:

/* Line 1806 of yacc.c  */
#line 2213 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2215 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2216 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 624:

/* Line 1806 of yacc.c  */
#line 2220 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 625:

/* Line 1806 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 626:

/* Line 1806 of yacc.c  */
#line 2222 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 627:

/* Line 1806 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 629:

/* Line 1806 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 631:

/* Line 1806 of yacc.c  */
#line 2233 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 632:

/* Line 1806 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2237 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2238 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 635:

/* Line 1806 of yacc.c  */
#line 2239 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 636:

/* Line 1806 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 637:

/* Line 1806 of yacc.c  */
#line 2243 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 638:

/* Line 1806 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 639:

/* Line 1806 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 640:

/* Line 1806 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 641:

/* Line 1806 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval).reset();}
    break;

  case 642:

/* Line 1806 of yacc.c  */
#line 2257 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 643:

/* Line 1806 of yacc.c  */
#line 2259 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 644:

/* Line 1806 of yacc.c  */
#line 2261 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 645:

/* Line 1806 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 646:

/* Line 1806 of yacc.c  */
#line 2266 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 647:

/* Line 1806 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 648:

/* Line 1806 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 649:

/* Line 1806 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval).reset(); }
    break;

  case 650:

/* Line 1806 of yacc.c  */
#line 2278 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 651:

/* Line 1806 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 652:

/* Line 1806 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 653:

/* Line 1806 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval).reset();}
    break;

  case 654:

/* Line 1806 of yacc.c  */
#line 2290 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 655:

/* Line 1806 of yacc.c  */
#line 2291 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
    break;

  case 656:

/* Line 1806 of yacc.c  */
#line 2298 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 657:

/* Line 1806 of yacc.c  */
#line 2300 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 658:

/* Line 1806 of yacc.c  */
#line 2303 "hphp.y"
    { only_in_hh_syntax(_p);}
    break;

  case 659:

/* Line 1806 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 660:

/* Line 1806 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 661:

/* Line 1806 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 662:

/* Line 1806 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval).reset();}
    break;

  case 663:

/* Line 1806 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 664:

/* Line 1806 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 665:

/* Line 1806 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 666:

/* Line 1806 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 667:

/* Line 1806 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 668:

/* Line 1806 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 669:

/* Line 1806 of yacc.c  */
#line 2332 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 670:

/* Line 1806 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 671:

/* Line 1806 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 672:

/* Line 1806 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 673:

/* Line 1806 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 674:

/* Line 1806 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 675:

/* Line 1806 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 676:

/* Line 1806 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 677:

/* Line 1806 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 678:

/* Line 1806 of yacc.c  */
#line 2350 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 679:

/* Line 1806 of yacc.c  */
#line 2352 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 680:

/* Line 1806 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 681:

/* Line 1806 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 682:

/* Line 1806 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 683:

/* Line 1806 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 684:

/* Line 1806 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 685:

/* Line 1806 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 686:

/* Line 1806 of yacc.c  */
#line 2365 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 687:

/* Line 1806 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 688:

/* Line 1806 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 689:

/* Line 1806 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 690:

/* Line 1806 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 691:

/* Line 1806 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 692:

/* Line 1806 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 693:

/* Line 1806 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 694:

/* Line 1806 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));}
    break;

  case 695:

/* Line 1806 of yacc.c  */
#line 2387 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 696:

/* Line 1806 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 697:

/* Line 1806 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));}
    break;

  case 698:

/* Line 1806 of yacc.c  */
#line 2398 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 699:

/* Line 1806 of yacc.c  */
#line 2402 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));}
    break;

  case 700:

/* Line 1806 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));}
    break;

  case 701:

/* Line 1806 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));}
    break;

  case 702:

/* Line 1806 of yacc.c  */
#line 2417 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));}
    break;

  case 703:

/* Line 1806 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 704:

/* Line 1806 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 705:

/* Line 1806 of yacc.c  */
#line 2428 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 706:

/* Line 1806 of yacc.c  */
#line 2429 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 709:

/* Line 1806 of yacc.c  */
#line 2434 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);}
    break;

  case 710:

/* Line 1806 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 711:

/* Line 1806 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval).reset();}
    break;

  case 712:

/* Line 1806 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = 1;}
    break;

  case 713:

/* Line 1806 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval)++;}
    break;

  case 714:

/* Line 1806 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 715:

/* Line 1806 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 716:

/* Line 1806 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 717:

/* Line 1806 of yacc.c  */
#line 2451 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 718:

/* Line 1806 of yacc.c  */
#line 2454 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 721:

/* Line 1806 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 722:

/* Line 1806 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 723:

/* Line 1806 of yacc.c  */
#line 2463 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 724:

/* Line 1806 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2468 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);}
    break;

  case 726:

/* Line 1806 of yacc.c  */
#line 2469 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 727:

/* Line 1806 of yacc.c  */
#line 2471 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 728:

/* Line 1806 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);}
    break;

  case 729:

/* Line 1806 of yacc.c  */
#line 2473 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));}
    break;

  case 730:

/* Line 1806 of yacc.c  */
#line 2474 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));}
    break;

  case 731:

/* Line 1806 of yacc.c  */
#line 2479 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 732:

/* Line 1806 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval).reset();}
    break;

  case 733:

/* Line 1806 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 734:

/* Line 1806 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 735:

/* Line 1806 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 736:

/* Line 1806 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 737:

/* Line 1806 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);}
    break;

  case 738:

/* Line 1806 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);}
    break;

  case 739:

/* Line 1806 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 740:

/* Line 1806 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 741:

/* Line 1806 of yacc.c  */
#line 2499 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 742:

/* Line 1806 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 743:

/* Line 1806 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 744:

/* Line 1806 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 745:

/* Line 1806 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 746:

/* Line 1806 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 747:

/* Line 1806 of yacc.c  */
#line 2512 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 748:

/* Line 1806 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 749:

/* Line 1806 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 750:

/* Line 1806 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 751:

/* Line 1806 of yacc.c  */
#line 2522 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 752:

/* Line 1806 of yacc.c  */
#line 2523 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 753:

/* Line 1806 of yacc.c  */
#line 2527 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);}
    break;

  case 754:

/* Line 1806 of yacc.c  */
#line 2529 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);}
    break;

  case 755:

/* Line 1806 of yacc.c  */
#line 2530 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);}
    break;

  case 756:

/* Line 1806 of yacc.c  */
#line 2532 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); }
    break;

  case 757:

/* Line 1806 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 758:

/* Line 1806 of yacc.c  */
#line 2539 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 759:

/* Line 1806 of yacc.c  */
#line 2541 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 760:

/* Line 1806 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);}
    break;

  case 761:

/* Line 1806 of yacc.c  */
#line 2545 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));}
    break;

  case 762:

/* Line 1806 of yacc.c  */
#line 2546 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 763:

/* Line 1806 of yacc.c  */
#line 2549 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;}
    break;

  case 764:

/* Line 1806 of yacc.c  */
#line 2550 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;}
    break;

  case 765:

/* Line 1806 of yacc.c  */
#line 2551 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;}
    break;

  case 766:

/* Line 1806 of yacc.c  */
#line 2555 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);}
    break;

  case 767:

/* Line 1806 of yacc.c  */
#line 2556 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);}
    break;

  case 768:

/* Line 1806 of yacc.c  */
#line 2557 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 769:

/* Line 1806 of yacc.c  */
#line 2558 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 770:

/* Line 1806 of yacc.c  */
#line 2559 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);}
    break;

  case 771:

/* Line 1806 of yacc.c  */
#line 2560 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);}
    break;

  case 772:

/* Line 1806 of yacc.c  */
#line 2561 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);}
    break;

  case 773:

/* Line 1806 of yacc.c  */
#line 2562 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);}
    break;

  case 774:

/* Line 1806 of yacc.c  */
#line 2563 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);}
    break;

  case 775:

/* Line 1806 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 776:

/* Line 1806 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 777:

/* Line 1806 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 780:

/* Line 1806 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); }
    break;

  case 781:

/* Line 1806 of yacc.c  */
#line 2590 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); }
    break;

  case 782:

/* Line 1806 of yacc.c  */
#line 2595 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 783:

/* Line 1806 of yacc.c  */
#line 2596 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 784:

/* Line 1806 of yacc.c  */
#line 2602 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 785:

/* Line 1806 of yacc.c  */
#line 2606 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); }
    break;

  case 786:

/* Line 1806 of yacc.c  */
#line 2612 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 787:

/* Line 1806 of yacc.c  */
#line 2613 "hphp.y"
    { (yyval).reset(); }
    break;

  case 788:

/* Line 1806 of yacc.c  */
#line 2617 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 789:

/* Line 1806 of yacc.c  */
#line 2620 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 790:

/* Line 1806 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 791:

/* Line 1806 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 792:

/* Line 1806 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval).reset(); }
    break;

  case 793:

/* Line 1806 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval).reset(); }
    break;

  case 794:

/* Line 1806 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval).reset(); }
    break;

  case 795:

/* Line 1806 of yacc.c  */
#line 2633 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 796:

/* Line 1806 of yacc.c  */
#line 2638 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); }
    break;

  case 797:

/* Line 1806 of yacc.c  */
#line 2639 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); }
    break;

  case 798:

/* Line 1806 of yacc.c  */
#line 2641 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); }
    break;

  case 799:

/* Line 1806 of yacc.c  */
#line 2642 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); }
    break;

  case 800:

/* Line 1806 of yacc.c  */
#line 2648 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); }
    break;

  case 803:

/* Line 1806 of yacc.c  */
#line 2659 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 804:

/* Line 1806 of yacc.c  */
#line 2661 "hphp.y"
    {}
    break;

  case 805:

/* Line 1806 of yacc.c  */
#line 2665 "hphp.y"
    { (yyval).setText("array"); }
    break;

  case 806:

/* Line 1806 of yacc.c  */
#line 2672 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 807:

/* Line 1806 of yacc.c  */
#line 2675 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 808:

/* Line 1806 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 809:

/* Line 1806 of yacc.c  */
#line 2679 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 810:

/* Line 1806 of yacc.c  */
#line 2682 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 811:

/* Line 1806 of yacc.c  */
#line 2684 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); }
    break;

  case 812:

/* Line 1806 of yacc.c  */
#line 2687 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); }
    break;

  case 813:

/* Line 1806 of yacc.c  */
#line 2690 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
    break;

  case 814:

/* Line 1806 of yacc.c  */
#line 2696 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
    break;

  case 815:

/* Line 1806 of yacc.c  */
#line 2700 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); }
    break;

  case 816:

/* Line 1806 of yacc.c  */
#line 2708 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 817:

/* Line 1806 of yacc.c  */
#line 2709 "hphp.y"
    { (yyval).reset(); }
    break;



/* Line 1806 of yacc.c  */
#line 12205 "hphp.tab.cpp"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, _p, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, _p, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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

  yyerror_range[1] = yylsp[1-yylen];
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
      if (!yypact_value_is_default (yyn))
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp, _p);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
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
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, _p);
    }
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



/* Line 2067 of yacc.c  */
#line 2712 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

